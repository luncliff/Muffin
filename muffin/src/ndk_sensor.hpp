#pragma once
#include <android/looper.h>
#include <android/sensor.h>

#include "muffin.hpp"

class ndk_sensor_poll_t final {
    // epoll_owner_t ep{};
    ALooper* looper = nullptr;
    ASensorManager* manager = nullptr;
    const ASensor* acc = nullptr;
    ASensorEventQueue* acc_queue = nullptr;
    const ASensor* gyro = nullptr;
    ASensorEventQueue* gyro_queue = nullptr;

   public:
    ndk_sensor_poll_t(JNIEnv* env, jstring package_name);
    ~ndk_sensor_poll_t();
    ndk_sensor_poll_t(const ndk_sensor_poll_t&) = delete;
    ndk_sensor_poll_t(ndk_sensor_poll_t&&) = delete;
    ndk_sensor_poll_t& operator=(const ndk_sensor_poll_t&) = delete;
    ndk_sensor_poll_t& operator=(ndk_sensor_poll_t&&) = delete;

    /**
     * @brief Create sensor event queue with the looper info
     */
    void bind(ALooper* looper, ALooper_callbackFunc callback, void* userdata) noexcept(false);
    void bind(ALooper* looper) noexcept(false);

    /**
     * @brief Enable the known sensor queues. Must success `bind` before the function.
     * @see bind
     */
    void enable() noexcept(false);

    /**
     * @brief Enable the known sensor queues
     * @see enable
     */
    void disable() noexcept(false);
};
