#include "muffin.hpp"

#include <android/api-level.h>
#include <android/dlext.h>
#include <android/native_window_jni.h>
#include <dlfcn.h>
#include <spdlog/sinks/android_sink.h>
#include <spdlog/spdlog.h>

extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
    constexpr auto version = JNI_VERSION_1_6;
    JNIEnv* env{};
    jint result = -1;
    if (vm->GetEnv((void**)&env, version) != JNI_OK) return result;

    auto stream = spdlog::android_logger_st("android", "muffin");
    stream->set_pattern("%v");                // Logcat will report time, thread, and level.
    stream->set_level(spdlog::level::debug);  // just print messages
    spdlog::set_default_logger(stream);
    spdlog::info("Device API Level: {}", android_get_device_api_level());
    return version;
}

void get_field(JNIEnv* env,  //
               jclass _type, jobject target, const char* field_name, jlong& ref) noexcept {
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    ref = env->GetLongField(target, _field);
}

void set_field(JNIEnv* env,  //
               jclass _type, jobject target, const char* field_name, jlong value) noexcept {
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    env->SetLongField(target, _field, value);
}

/**
 * @brief Find exception class information (type info)
 * @see java/lang/RuntimeException
 */
void store_runtime_exception(JNIEnv* env, const char* message) noexcept {
    const char* name = "java/lang/RuntimeException";
    jclass t = env->FindClass(name);
    if (t == nullptr) return spdlog::error("{:s}: {:s}", "No Java class", name);
    env->ThrowNew(t, message);
}

static_assert(sizeof(void*) <= sizeof(jlong), "`jlong` must be able to contain `void*` pointer");

native_loader_t::native_loader_t(const char* libname) noexcept(false) : native_loader_t{} { load(libname); }

native_loader_t::~native_loader_t() noexcept {
    if (handle) dlclose(handle);
}

void native_loader_t::load(const char* libname) noexcept(false) {
    android_dlextinfo info{};
    info.flags = ANDROID_DLEXT_FORCE_LOAD;
    handle = android_dlopen_ext(libname, RTLD_NOW, &info);
    if (handle == nullptr) throw std::system_error{errno, std::system_category(), dlerror()};
}

void* native_loader_t::get_proc_address(const char* proc) const noexcept {
    spdlog::trace("{:s}: {:s}", "dlsym", proc);
    return dlsym(handle, proc);
}
