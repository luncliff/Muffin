/**
 * @file    adapter.cpp
 * @author  github.com/luncliff (luncliff@gmail.com)
 */
#include "adapter.h"
#include <chrono>
#include <spdlog/sinks/android_sink.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

uint32_t android_level = 0;

extern "C" jint JNI_OnLoad(JavaVM *vm, void *) {
    constexpr auto version = JNI_VERSION_1_6;
    JNIEnv *env{};
    jint result = -1;
    if (vm->GetEnv((void **)&env, version) != JNI_OK) {
        return result;
    }

    if (auto level = android_get_device_api_level(); level > 0)
        android_level = level;

    // auto stream = spdlog::android_logger_st("android", "muffin");
    // stream->set_level(spdlog::level::debug);
    // spdlog::set_default_logger(stream);
    // // Logcat will report time, thread, and level. just print message without decoration
    // spdlog::set_pattern("%v");
    return version;
}

/**
 * @brief Find exception class information (type info)
 * @return java/lang/RuntimeException
 */
jclass get_runtime_exception(JNIEnv *env) {
    return env->FindClass("java/lang/RuntimeException");
}

void get_field(JNIEnv *env, //
               const char *type_name, jobject target, const char *field_name,
               jlong &ref) {
    const jclass _type = env->FindClass(type_name);
    const jfieldID _field = env->GetFieldID(_type, field_name, "J"); // long
    ref = env->GetLongField(target, _field);
}

void set_field(JNIEnv *env, //
               const char *type_name, jobject target, const char *field_name,
               jlong value) {
    const jclass _type = env->FindClass(type_name);
    const jfieldID _field = env->GetFieldID(_type, field_name, "J"); // long
    env->SetLongField(target, _field, value);
}

static_assert(sizeof(void *) <= sizeof(jlong),
              "`jlong` must be able to contain `void*` pointer");

extern "C" {

jlong Java_muffin_Compass_create(JNIEnv *env, jclass type, jstring _id) {
    // spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    // char name[60]{};
    // env->GetStringUTFRegion(_id, 0, env->GetStringLength(_id), name);
    // auto *impl = new compass_t{name};
    // return reinterpret_cast<jlong>(impl);
    return 0;
}

void Java_muffin_Compass_destroy(JNIEnv *env, jclass type, jlong value) {
    // spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    // auto *impl = reinterpret_cast<compass_t *>(value);
    // delete impl;
}

jint Java_muffin_Compass_update(JNIEnv *env, jclass type, jlong value) {
    // spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    // auto *impl = reinterpret_cast<compass_t *>(value);
    // return impl->update();
    return ENOTSUP;
}
jint Java_muffin_Compass_resume(JNIEnv *env, jclass type, jlong value) {
    // spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    // auto *impl = reinterpret_cast<compass_t *>(value);
    // return impl->resume();
    return ENOTSUP;
}
jint Java_muffin_Compass_pause(JNIEnv *env, jclass type, jlong value) {
    // spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    // auto *impl = reinterpret_cast<compass_t *>(value);
    // return impl->pause();
    return ENOTSUP;
}

jstring Java_muffin_Compass_getName(JNIEnv *env, jobject _object) {
    jlong value = 0;
    get_field(env, "muffin/Compass", _object, "impl", value);
    char buf[20]{};
    snprintf(buf, 20, "%llx", static_cast<uint64_t>(value));
    return env->NewStringUTF(buf);
}

} // extern "C"
