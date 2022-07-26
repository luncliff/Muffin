#include "muffin.hpp"

#include <android/api-level.h>
#include <android/native_window_jni.h>
#include <spdlog/sinks/android_sink.h>
#include <spdlog/spdlog.h>

extern "C" jint JNI_OnLoad(JavaVM *vm, void *) {
    constexpr auto version = JNI_VERSION_1_6;
    JNIEnv *env{};
    jint result = -1;
    if (vm->GetEnv((void **)&env, version) != JNI_OK) return result;

    auto stream = spdlog::android_logger_st("android", "muffin");
    stream->set_pattern("%v");                // Logcat will report time, thread, and level.
    stream->set_level(spdlog::level::debug);  // just print messages
    spdlog::set_default_logger(stream);
    spdlog::info("Device API Level: {}", android_get_device_api_level());
    return version;
}
