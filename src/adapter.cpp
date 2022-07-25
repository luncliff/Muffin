#include <android/api-level.h>
#include <android/hardware_buffer.h>
#include <android/native_window_jni.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <string>
#include <string_view>

#include "muffin.hpp"

using namespace std::experimental;

void get_field(JNIEnv *env,  //
               jclass _type, jobject target, const char *field_name, jlong &ref) noexcept {
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    ref = env->GetLongField(target, _field);
}

void set_field(JNIEnv *env,  //
               jclass _type, jobject target, const char *field_name, jlong value) noexcept {
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

static_assert(sizeof(void *) <= sizeof(jlong), "`jlong` must be able to contain `void*` pointer");

extern "C" {

JNIEXPORT
jboolean Java_dev_luncliff_muffin_NativeRunnable_resume(JNIEnv *, jobject, void *handle) noexcept {
    auto task = coroutine_handle<>::from_address(handle);
    task.resume();
    if (task.done()) {
        task.destroy();
        return JNI_TRUE;
    }
    return JNI_FALSE;  // next run will continue the work
}

JNIEXPORT
jlong Java_dev_luncliff_muffin_Renderer1_create1(JNIEnv *, jclass, jlong d, jlong c) noexcept {
    auto es_display = reinterpret_cast<EGLDisplay>(d);
    auto es_config = get_default_config(es_display);
    auto es_context = reinterpret_cast<EGLContext>(c);
    auto context = std::make_unique<egl_context_t>(es_display, es_config, es_context);
    if (context->handle() == EGL_NO_CONTEXT) {
        spdlog::error("context is created but not valid");
        return 0;
    }
    return reinterpret_cast<jlong>(context.release());
}

JNIEXPORT
void Java_dev_luncliff_muffin_Renderer1_destroy1(JNIEnv *, jclass, jlong c) noexcept {
    auto context = reinterpret_cast<egl_context_t *>(c);
    delete context;
}

JNIEXPORT
jlong Java_dev_luncliff_muffin_Renderer1_create2(JNIEnv *env, jclass, jlong c, jobject _surface) noexcept {
    try {
        auto context = reinterpret_cast<egl_context_t *>(c);
        auto surface = make_egl_surface(context->get_display(), env, _surface);
        return reinterpret_cast<jlong>(surface.release());
    } catch (const std::exception &ex) {
        store_runtime_exception(env, ex.what());
        return 0;
    }
}

JNIEXPORT
void Java_dev_luncliff_muffin_Renderer1_destroy2(JNIEnv *, jclass, jlong s) noexcept {
    auto surface = reinterpret_cast<egl_surface_t *>(s);
    delete surface;
}

JNIEXPORT
jint Java_dev_luncliff_muffin_Renderer1_resume(JNIEnv *, jclass, jlong c, jlong s) noexcept {
    auto context = reinterpret_cast<egl_context_t *>(c);
    auto surface = reinterpret_cast<egl_surface_t *>(s);
    return context->resume(surface->handle());
}

JNIEXPORT
jint Java_dev_luncliff_muffin_Renderer1_suspend(JNIEnv *, jclass, jlong c) noexcept {
    auto context = reinterpret_cast<egl_context_t *>(c);
    return context->suspend();
}

JNIEXPORT
jint Java_dev_luncliff_muffin_Renderer1_present(JNIEnv *, jclass, jlong c) noexcept {
    auto context = reinterpret_cast<egl_context_t *>(c);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    return context->swap();
}

JNIEXPORT
jstring Java_dev_luncliff_muffin_Renderer1_toString(JNIEnv *env, jobject self) noexcept {
    jclass _type = env->FindClass("dev/luncliff/muffin/Renderer1");
    jlong value = 0;
    get_field(env, _type, self, "ptr", value);
    constexpr auto cap = 32 - sizeof(jlong);
    char txt[cap]{};
    snprintf(txt, cap, "%p", reinterpret_cast<void *>(value));
    return env->NewStringUTF(txt);
}

}  // extern "C"
