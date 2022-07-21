#include <android/api-level.h>
#include <android/hardware_buffer.h>
#include <android/native_window_jni.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <spdlog/sinks/android_sink.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <string>
#include <string_view>

#include "muffin.hpp"

using namespace std::experimental;

extern "C" jint JNI_OnLoad(JavaVM *vm, void *) {
    constexpr auto version = JNI_VERSION_1_6;
    JNIEnv *env{};
    jint result = -1;
    if (vm->GetEnv((void **)&env, version) != JNI_OK) return result;

    auto stream = spdlog::android_logger_st("android", "muffin");
    stream->set_pattern("%v");  // Logcat will report time, thread, and level.
    stream->set_level(spdlog::level::debug);  // just print messages
    spdlog::set_default_logger(stream);
    spdlog::info("Device API Level: {}", android_get_device_api_level());
    return version;
}

void get_field(JNIEnv *env,  //
               const char *type_name, jobject target, const char *field_name,
               jlong &ref) noexcept {
    jclass _type = env->FindClass(type_name);
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    ref = env->GetLongField(target, _field);
}
void get_field(JNIEnv *env,  //
               jclass _type, jobject target, const char *field_name,
               jlong &ref) noexcept {
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    ref = env->GetLongField(target, _field);
}
void set_field(JNIEnv *env,  //
               const char *type_name, jobject target, const char *field_name,
               jlong value) noexcept {
    jclass _type = env->FindClass(type_name);
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    env->SetLongField(target, _field, value);
}
void set_field(JNIEnv *env,  //
               jclass _type, jobject target, const char *field_name,
               jlong value) noexcept {
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    env->SetLongField(target, _field, value);
}

/**
 * @brief Find exception class information (type info)
 * @see java/lang/RuntimeException
 */
void store_runtime_exception(JNIEnv *env, const char *message) noexcept {
    const char *class_name = "java/lang/RuntimeException";
    jclass _type = env->FindClass(class_name);
    if (_type == nullptr) return spdlog::error("No Java class: {}", class_name);
    env->ThrowNew(_type, message);
}

jobject make_runnable(JNIEnv *env, coroutine_handle<> task) noexcept {
    jclass _type = env->FindClass("muffin/NativeRunnable");
    jobject _task = env->AllocObject(_type);
    set_field(env, _type, _task, "handle",
              reinterpret_cast<jlong>(task.address()));
    return _task;
}

uint32_t schedule(JNIEnv *env, jobject executor,
                  coroutine_handle<> task) noexcept {
    jclass _type = env->FindClass("java/util/concurrent/Executor");
    if (_type == nullptr) {
        spdlog::error("No Java class: {}",  //
                      "java/util/concurrent/Executor");
        return EINVAL;
    }
    jmethodID _method = env->GetMethodID(_type, "execute",  //
                                         "(Ljava/lang/Runnable;)V");
    if (_method == nullptr) {
        spdlog::error("No Java method: {} {}",  //
                      "java/util/concurrent/Executor", "execute");
        return ENOTSUP;
    }
    jobject _task = make_runnable(env, task);
    if (_task == nullptr) {
        store_runtime_exception(env, "failed to create runnable");
        return EXIT_FAILURE;
    }
    env->CallVoidMethod(executor, _method, _task);
    return EXIT_SUCCESS;
}

static_assert(sizeof(void *) <= sizeof(jlong),
              "`jlong` must be able to contain `void*` pointer");

extern "C" {

JNIEXPORT
jboolean Java_dev_luncliff_muffin_NativeRunnable_resume(JNIEnv *, jobject,
                                                        void *handle) noexcept {
    spdlog::debug(std::string_view{__PRETTY_FUNCTION__});
    auto task = coroutine_handle<>::from_address(handle);
    task.resume();
    if (task.done()) {
        task.destroy();
        return JNI_TRUE;
    }
    return JNI_FALSE;  // next run will continue the work
}

JNIEXPORT
jlong Java_dev_luncliff_muffin_Renderer1_create1(
    JNIEnv *env, jclass, jobject _executor, EGLDisplay egl_display,
    EGLContext egl_context) noexcept {
    spdlog::debug(std::string_view{__PRETTY_FUNCTION__});
    auto context = std::make_unique<egl_context_t>(egl_display, egl_context);
    if (context->handle() == EGL_NO_CONTEXT) {
        spdlog::error("context is created but not valid");
        return 0;
    }
    if (auto ec = schedule(env, _executor, noop_coroutine()))
        spdlog::error("failed to schedule task: {}", ec);
    return reinterpret_cast<jlong>(context.release());
}

JNIEXPORT
void Java_dev_luncliff_muffin_Renderer1_destroy1(
    JNIEnv *, jclass, egl_context_t *context) noexcept {
    spdlog::debug(std::string_view{__PRETTY_FUNCTION__});
    delete context;
}

JNIEXPORT
jlong Java_dev_luncliff_muffin_Renderer1_create2(JNIEnv *env, jclass,
                                                 egl_context_t *context,
                                                 jobject _surface,
                                                 jobject) noexcept {
    spdlog::debug(std::string_view{__PRETTY_FUNCTION__});
    try {
        auto surface = make_egl_surface(context->get_display(), env, _surface);
        return reinterpret_cast<jlong>(surface.release());
    } catch (const std::exception &ex) {
        store_runtime_exception(env, ex.what());
        return 0;
    }
}

JNIEXPORT
void Java_dev_luncliff_muffin_Renderer1_destroy2(
    JNIEnv *, jclass, egl_surface_t *surface) noexcept {
    spdlog::debug(std::string_view{__PRETTY_FUNCTION__});
    delete surface;
}

JNIEXPORT
jint Java_dev_luncliff_muffin_Renderer1_resume(
    JNIEnv *, jclass, egl_context_t *context, egl_surface_t *surface) noexcept {
    return context->resume(surface->handle());
}

JNIEXPORT
jint Java_dev_luncliff_muffin_Renderer1_suspend(JNIEnv *, jclass,
                                                egl_context_t *context,
                                                jobject) noexcept {
    return context->suspend();
}

JNIEXPORT
jint Java_dev_luncliff_muffin_Renderer1_present(
    JNIEnv *, jclass, egl_context_t *context) noexcept {
    spdlog::debug(std::string_view{__PRETTY_FUNCTION__});
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    return context->swap();
}

JNIEXPORT
jstring Java_dev_luncliff_muffin_Renderer1_toString(JNIEnv *env,
                                                    jobject _this) noexcept {
    jlong value = 0;
    get_field(env, "muffin/Renderer1", _this, "ptr", value);
    constexpr auto cap = 32 - sizeof(jlong);
    char txt[cap]{};
    snprintf(txt, cap, "%p", reinterpret_cast<void *>(value));
    return env->NewStringUTF(txt);
}

}  // extern "C"
