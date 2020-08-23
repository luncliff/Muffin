/**
 * @file    adapter.cpp
 * @author  github.com/luncliff (luncliff@gmail.com)
 */
#include "adapter.h"
#include <array>
#include <cerrno>
#define SPDLOG_FMT_EXTERNAL
#include <spdlog/sinks/android_sink.h>
#include <spdlog/spdlog.h>

using namespace std::string_literals;
using namespace std::string_view_literals;

extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
    constexpr auto version = JNI_VERSION_1_6;
    JNIEnv* env{};
    jint result = -1;
    if (vm->GetEnv((void**)&env, version) != JNI_OK) {
        return result;
    }
    auto stream = spdlog::android_logger_st("android", "muffin");
    stream->set_level(spdlog::level::debug);
    spdlog::set_default_logger(stream);
    // Logcat will report time, thread, and level. just print message without decoration
    spdlog::set_pattern("%v");
    return version;
}

/**
 * @brief Find exception class information (type info)
 * @return java/lang/RuntimeException
 */
jclass get_runtime_exception(JNIEnv* env) {
    return env->FindClass("java/lang/RuntimeException");
}

class sensor_owner_t {
  protected:
    ALooper* const looper;
    ASensorManager* const manager;
    ASensorList sensors = nullptr;
    int count = 0;

  public:
    explicit sensor_owner_t(const char* _id, ALooper* _looper) noexcept(false)
        : looper{_looper}, manager{ASensorManager_getInstanceForPackage(_id)} {
        ALooper_acquire(looper);
        // sensors[0...count]
        count = ASensorManager_getSensorList(manager, &sensors);
        spdlog::info("sensor:"sv);
        for (int i = 0; i < count; i++) {
            spdlog::info(" - {}"sv, ASensor_getName(sensors[i]));
        }
    }
    ~sensor_owner_t() noexcept {
        ALooper_release(looper);
    }

    uint32_t get_count() const noexcept {
        return static_cast<uint32_t>(count);
    }
};

/**
 * @todo caliberate multiple sensor input
 */
class compass_t : public sensor_owner_t {
    ASensorEventQueue* queue;
    const ASensor* acc;
    const ASensor* gyro;

  private:
    static ALooper* get_looper() noexcept {
        auto* looper = ALooper_forThread();
        if (looper == nullptr) {
            looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
            spdlog::info("created looper: {:p}"sv,
                         reinterpret_cast<void*>(looper));
        }
        return looper;
    }

  public:
    explicit compass_t(const char* _id) noexcept(false)
        : sensor_owner_t{_id, get_looper()},
          acc{ASensorManager_getDefaultSensor(manager,
                                              ASENSOR_TYPE_ACCELEROMETER)},
          gyro{ASensorManager_getDefaultSensor(manager,
                                               ASENSOR_TYPE_GYROSCOPE)} {
        ALooper_callbackFunc callback = nullptr;
        void* user_data = nullptr;
        queue = ASensorManager_createEventQueue(manager, looper, 0xAAAA,
                                                callback, user_data);
    }
    ~compass_t() noexcept {
        if (auto ec = ASensorManager_destroyEventQueue(manager, queue))
            spdlog::warn("ASensorManager_destroyEventQueue: {}"sv, ec);
    }

    /// @todo error code identification
    uint32_t resume() noexcept {
        constexpr auto usec = 1'000'000 / 120; // 120 hz
        {
            if (int ec = ASensorEventQueue_enableSensor(queue, acc))
                return static_cast<uint32_t>(-ec);
            if (int ec = ASensorEventQueue_setEventRate(queue, acc, usec))
                return static_cast<uint32_t>(-ec);
        }
        {
            if (int ec = ASensorEventQueue_enableSensor(queue, gyro))
                return static_cast<uint32_t>(-ec);
            if (int ec = ASensorEventQueue_setEventRate(queue, gyro, usec))
                return static_cast<uint32_t>(-ec);
        }
        return 0;
    }

    /// @todo error code identification
    uint32_t pause() noexcept {
        if (int ec = ASensorEventQueue_disableSensor(queue, acc))
            return static_cast<uint32_t>(-ec);
        if (int ec = ASensorEventQueue_disableSensor(queue, gyro))
            return static_cast<uint32_t>(-ec);
        return 0;
    }

    /**
     * @see https://developer.android.com/reference/android/hardware/SensorEvent
     * @see https://developer.android.com/guide/topics/sensors/sensors_motion
     */
    void consume(const ASensorEvent* events, size_t count) noexcept {
        // constexpr auto alpha = 0.173205f;
        for (auto i = 0u; i < count; ++i) {
            const auto& e = events[i];
            switch (e.type) {
            case ASENSOR_TYPE_ACCELEROMETER: // m/s2
                // e.acceleration;
            case ASENSOR_TYPE_GYROSCOPE: // m/s2
                // e.data[0...2];
            case ASENSOR_TYPE_GRAVITY: // m/s2
            default:
                break;
            }
            spdlog::warn("event discard: type %d"sv, e.type);
        }
    }

    uint32_t update() noexcept {
        switch (auto ec = ALooper_pollAll(0, nullptr, nullptr, nullptr)) {
        case ALOOPER_POLL_WAKE:
        case ALOOPER_POLL_CALLBACK:
        case ALOOPER_POLL_ERROR:
            spdlog::warn("ALooper_pollAll: error {}"sv, ec);
            return static_cast<uint32_t>(-ec);
        case ALOOPER_POLL_TIMEOUT:
            return 0;
        default:
            break; // good to update
        };
        constexpr auto capacity = 1000 / sizeof(ASensorEvent); // nearly 10
        std::array<ASensorEvent, capacity> events{};
        const auto count = ASensorEventQueue_getEvents(queue, //
                                                       events.data(), capacity);
        if (count < 0) {
            return static_cast<uint32_t>(-count); // == error code
        }
        spdlog::debug("ASensorEventQueue_getEvents: count {}"sv, count);
        const auto b = events.data();
        consume(b, static_cast<size_t>(count));
        memset(b + count, 0, sizeof(ASensorEvent) * (capacity - count));
        return 0;
    }
};

void get_field(JNIEnv* env, //
               const char* t, jobject _object, const char* f, jlong& ref) {
    const jclass _type = env->FindClass(t);
    const jfieldID _field = env->GetFieldID(_type, f, "J"); // long
    ref = env->GetLongField(_object, _field);
}

void set_field(JNIEnv* env, //
               const char* t, jobject _object, const char* f, jlong value) {
    const jclass _type = env->FindClass(t);
    const jfieldID _field = env->GetFieldID(_type, f, "J"); // long
    env->SetLongField(_object, _field, value);
}

static_assert(sizeof(void*) <= sizeof(jlong),
              "`jlong` must be able to contain `void*` pointer");

extern "C" {

jlong Java_muffin_Compass_create(JNIEnv* env, jclass type, jstring _id) {
    spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    char name[60]{};
    env->GetStringUTFRegion(_id, 0, env->GetStringLength(_id), name);
    auto* impl = new compass_t{name};
    return reinterpret_cast<jlong>(impl);
}

void Java_muffin_Compass_destroy(JNIEnv* env, jclass type, jlong value) {
    spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    auto* impl = reinterpret_cast<compass_t*>(value);
    delete impl;
}

jint Java_muffin_Compass_update(JNIEnv* env, jclass type, jlong value) {
    spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    auto* impl = reinterpret_cast<compass_t*>(value);
    return impl->update();
}
jint Java_muffin_Compass_resume(JNIEnv* env, jclass type, jlong value) {
    spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    auto* impl = reinterpret_cast<compass_t*>(value);
    return impl->resume();
}
jint Java_muffin_Compass_pause(JNIEnv* env, jclass type, jlong value) {
    spdlog::trace(std::string_view{__PRETTY_FUNCTION__});
    auto* impl = reinterpret_cast<compass_t*>(value);
    return impl->pause();
}

jstring Java_muffin_Compass_getName(JNIEnv* env, jobject _object) {
    jlong value = 0;
    get_field(env, "muffin/Compass", _object, "impl", value);
    char buf[20]{};
    snprintf(buf, 20, "%lx", static_cast<uint64_t>(value));
    return env->NewStringUTF(buf);
}

} // extern "C"
