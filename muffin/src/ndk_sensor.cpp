#include "ndk_sensor.hpp"

#include <spdlog/spdlog.h>

std::string make_string(JNIEnv* env, jstring str) noexcept(false);

ndk_sensor_poll_t::ndk_sensor_poll_t(JNIEnv* env, jstring package_name) {
    auto package = make_string(env, package_name);
    manager = ASensorManager_getInstanceForPackage(package.c_str());
    spdlog::info("{}: package {} sensors {:p}", "sensor", package, static_cast<void*>(manager));
    // use uncalibrated?
    acc = ASensorManager_getDefaultSensor(manager, ASENSOR_TYPE_ACCELEROMETER);
    gyro = ASensorManager_getDefaultSensor(manager, ASENSOR_TYPE_GYROSCOPE);
}

ndk_sensor_poll_t::~ndk_sensor_poll_t() {
    if (acc_queue) ASensorManager_destroyEventQueue(manager, acc_queue);
    if (gyro_queue) ASensorManager_destroyEventQueue(manager, gyro_queue);
    if (looper) ALooper_release(looper);
}

void ndk_sensor_poll_t::replace(ALooper* _looper) noexcept {
    if (looper) ALooper_release(looper);
    looper = _looper;
    ALooper_acquire(looper);
}

void ndk_sensor_poll_t::bind(ALooper* _looper, ALooper_callbackFunc callback, void* userdata) noexcept(false) {
    this->replace(_looper);
    // replace the queue
    if (acc_queue) ASensorManager_destroyEventQueue(manager, acc_queue);
    acc_queue = ASensorManager_createEventQueue(manager, looper, ALOOPER_POLL_CALLBACK, callback, userdata);
    spdlog::info("{}: {} {:p}", "sensor", "acc_queue", static_cast<void*>(acc_queue));
    // replace the queue
    if (gyro_queue) ASensorManager_destroyEventQueue(manager, gyro_queue);
    gyro_queue = ASensorManager_createEventQueue(manager, looper, ALOOPER_POLL_CALLBACK, callback, userdata);
    spdlog::info("{}: {} {:p}", "sensor", "gyro_queue", static_cast<void*>(acc_queue));
}

void ndk_sensor_poll_t::bind(ALooper* _looper) noexcept(false) {
    this->replace(_looper);
    // replace the queue
    if (acc_queue) ASensorManager_destroyEventQueue(manager, acc_queue);
    acc_queue = ASensorManager_createEventQueue(manager, looper, 0xBEEF, nullptr, nullptr);
    spdlog::info("{}: {} {:p}", "sensor", "acc_queue", static_cast<void*>(acc_queue));
    // replace the queue
    if (gyro_queue) ASensorManager_destroyEventQueue(manager, gyro_queue);
    gyro_queue = ASensorManager_createEventQueue(manager, looper, 0xBEEF, nullptr, nullptr);
    spdlog::info("{}: {} {:p}", "sensor", "gyro_queue", static_cast<void*>(acc_queue));
}

/// @note consider ASensorEventQueue_registerSensor?
void ndk_sensor_poll_t::enable() noexcept(false) {
    spdlog::debug("{}: enable {}", "sensor", "acc_queue");
    // start with default sampling rate
    if (int ec = ASensorEventQueue_enableSensor(acc_queue, acc); ec != 0)
        throw std::system_error{-ec, std::system_category(), "ASensorEventQueue_enableSensor"};
    spdlog::debug("{}: enable {}", "sensor", "gyro_queue");
    if (int ec = ASensorEventQueue_enableSensor(gyro_queue, gyro); ec != 0)
        throw std::system_error{-ec, std::system_category(), "ASensorEventQueue_enableSensor"};
}

void ndk_sensor_poll_t::disable() noexcept(false) {
    if (acc_queue) {
        spdlog::debug("{}: disable {}", "sensor", "acc_queue");
        if (int ec = ASensorEventQueue_disableSensor(acc_queue, acc); ec != 0)
            throw std::system_error{-ec, std::system_category(), "ASensorEventQueue_disableSensor"};
    }
    if (gyro_queue) {
        spdlog::debug("{}: disable {}", "sensor", "gyro_queue");
        if (int ec = ASensorEventQueue_disableSensor(gyro_queue, gyro); ec != 0)
            throw std::system_error{-ec, std::system_category(), "ASensorEventQueue_disableSensor"};
    }
}

void ndk_sensor_poll_t::poll(ASensorEvent* events, size_t count) noexcept(false) {
    int fd = 0;
    int type = 0;
    void* context = nullptr;
    switch (ALooper_pollAll(300, &fd, &type, &context)) {
        case ALOOPER_POLL_WAKE:
            if (ASensorEventQueue_hasEvents(acc_queue)) ASensorEventQueue_getEvents(acc_queue, events, count);
            if (ASensorEventQueue_hasEvents(gyro_queue)) ASensorEventQueue_getEvents(gyro_queue, events, count);
            return;
        case ALOOPER_POLL_TIMEOUT:
            spdlog::debug("{}: {}", "sensor", "timeout");
            return;
        case ALOOPER_POLL_ERROR:
        default:
            spdlog::error("{}: {}", "sensor", "ALooper_pollAll");
            return;
    }
}

extern "C" {

JNIEXPORT void Java_dev_luncliff_muffin_SensorThreadTest_test0(JNIEnv* env, jobject, jstring package_name, jobject) {
    ALooper* looper = ALooper_forThread();
    if (looper == nullptr) looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);

    std::array<ASensorEvent, 20> events{};
    try {
        ndk_sensor_poll_t sp{env, package_name};
        sp.bind(looper);
        sp.enable();
        sp.poll(events.data(), events.size());
        sp.disable();
    } catch (const std::exception& ex) {
        spdlog::error(ex.what());
    }
}
}
