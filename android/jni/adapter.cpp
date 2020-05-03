/**
 * @file    adapter.cpp
 * @author  github.com/luncliff (luncliff@gmail.com)
 */
#include "adapter.h"

#include <array>
#include <cerrno>

extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
    constexpr auto version = JNI_VERSION_1_6;
    JNIEnv* env{};
    jint result = -1;
    if (vm->GetEnv((void**)&env, version) != JNI_OK) {
        return result;
    }
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
        __android_log_print(ANDROID_LOG_INFO, "[muffin]", //
                            "sensor count: %u", get_count());
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
            __android_log_print(ANDROID_LOG_INFO, "[muffin]", //
                                "created looper: %p", looper);
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
            __android_log_print(ANDROID_LOG_WARN, "[muffin]",
                                "ASensorManager_destroyEventQueue: %d", ec);
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

    void consume(const ASensorEvent* events, size_t count) noexcept {
        constexpr auto alpha = 0.173205f;
    }

    uint32_t update() noexcept {
        switch (auto ec = ALooper_pollAll(0, nullptr, nullptr, nullptr)) {
        case ALOOPER_POLL_WAKE:
        case ALOOPER_POLL_CALLBACK:
        case ALOOPER_POLL_TIMEOUT:
        case ALOOPER_POLL_ERROR:
            return static_cast<uint32_t>(-ec);
        default:
            break; // good to update
        };
        constexpr auto capacity = 1000 / sizeof(ASensorEvent);
        std::array<ASensorEvent, capacity> events{};
        const auto count = ASensorEventQueue_getEvents(queue, //
                                                       events.data(), capacity);
        if (count < 0) {
            return static_cast<uint32_t>(-count); // == error code
        }
        consume(events.data(), static_cast<size_t>(count));
        return 0;
    }
};

void get_field(JNIEnv* env,                    //
               const char* t, jobject _object, //
               const char* f, jlong& ref) {
    const jclass _type = env->FindClass(t);
    const jfieldID _field = env->GetFieldID(_type, f, "J"); // long
    ref = env->GetLongField(_object, _field);
}

void set_field(JNIEnv* env,                    //
               const char* t, jobject _object, //
               const char* f, jlong value) {
    const jclass _type = env->FindClass(t);
    const jfieldID _field = env->GetFieldID(_type, f, "J"); // long
    env->SetLongField(_object, _field, value);
}

static_assert(sizeof(void*) <= sizeof(jlong),
              "`jlong` must be able to contain `void*` pointer");

extern "C" {

jlong Java_muffin_Compass_create(JNIEnv* env, jclass type, jstring _id) {
    char name[60]{};
    env->GetStringUTFRegion(_id, 0, env->GetStringLength(_id), name);

    auto* impl = new compass_t{name};
    return reinterpret_cast<jlong>(impl);
}

void Java_muffin_Compass_destroy(JNIEnv* env, jclass type, jlong value) {
    auto* impl = reinterpret_cast<compass_t*>(value);
    delete impl;
}

jint Java_muffin_Compass_update(JNIEnv* env, jclass type, jlong value) {
    auto* impl = reinterpret_cast<compass_t*>(value);
    return impl->update();
}
jint Java_muffin_Compass_resume(JNIEnv* env, jclass type, jlong value) {
    auto* impl = reinterpret_cast<compass_t*>(value);
    return impl->resume();
}
jint Java_muffin_Compass_pause(JNIEnv* env, jclass type, jlong value) {
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
