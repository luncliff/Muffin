#pragma once
#include <android/hardware_buffer_jni.h>
#include <media/NdkImage.h>

class ndk_hardware_buffer_t final {
    AHardwareBuffer* ptr = nullptr;

   public:
    explicit ndk_hardware_buffer_t(AImage* image) noexcept(false);
    explicit ndk_hardware_buffer_t(int socket) noexcept(false);
    explicit ndk_hardware_buffer_t(const AHardwareBuffer_Desc& desc) noexcept(false);
    ndk_hardware_buffer_t(JNIEnv* env, jobject object) noexcept(false);
    ~ndk_hardware_buffer_t() noexcept;
    ndk_hardware_buffer_t(const ndk_hardware_buffer_t&) noexcept(false);
    ndk_hardware_buffer_t(ndk_hardware_buffer_t&&) noexcept(false);
    ndk_hardware_buffer_t& operator=(const ndk_hardware_buffer_t&) noexcept(false);
    ndk_hardware_buffer_t& operator=(ndk_hardware_buffer_t&&) noexcept(false);

    jobject object(JNIEnv* env) const noexcept;

    constexpr AHardwareBuffer* operator()() const noexcept { return ptr; }

    void get(AHardwareBuffer_Desc& desc) const noexcept;
    void send_to(int socket) noexcept(false);

    void* lock(uint64_t usage, int32_t fence = -1) noexcept(false);
    void unlock(int32_t* fence = nullptr) noexcept(false);
};
