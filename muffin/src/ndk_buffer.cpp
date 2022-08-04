#include "ndk_buffer.hpp"

#include <stdexcept>

ndk_hardware_buffer_t::ndk_hardware_buffer_t(AImage* image) noexcept(false) {
    if (AImage_getHardwareBuffer(image, &ptr) != 0) throw std::runtime_error{"AImage_getHardwareBuffer"};
    AHardwareBuffer_acquire(ptr);
}

ndk_hardware_buffer_t::ndk_hardware_buffer_t(int socket) noexcept(false) {
    if (AHardwareBuffer_recvHandleFromUnixSocket(socket, &ptr) != 0)
        throw std::runtime_error{"AHardwareBuffer_recvHandleFromUnixSocket"};
    AHardwareBuffer_acquire(ptr);
}

ndk_hardware_buffer_t::ndk_hardware_buffer_t(const AHardwareBuffer_Desc& desc) noexcept(false) {
    if (AHardwareBuffer_allocate(&desc, &ptr) != 0) throw std::runtime_error{"AHardwareBuffer_allocate"};
}

ndk_hardware_buffer_t::ndk_hardware_buffer_t(JNIEnv* env, jobject object) noexcept(false) {
    ptr = AHardwareBuffer_fromHardwareBuffer(env, object);
    if (ptr == nullptr) throw std::runtime_error{"AHardwareBuffer_fromHardwareBuffer"};
    AHardwareBuffer_acquire(ptr);
}

ndk_hardware_buffer_t::~ndk_hardware_buffer_t() noexcept { AHardwareBuffer_release(ptr); }

ndk_hardware_buffer_t::ndk_hardware_buffer_t(const ndk_hardware_buffer_t& rhs) noexcept(false) {
    ptr = rhs.ptr;
    AHardwareBuffer_acquire(ptr);
}

ndk_hardware_buffer_t::ndk_hardware_buffer_t(ndk_hardware_buffer_t&& rhs) noexcept(false) {
    AHardwareBuffer_acquire(rhs.ptr);
    AHardwareBuffer_release(ptr);
    ptr = rhs.ptr;
}

ndk_hardware_buffer_t& ndk_hardware_buffer_t::operator=(const ndk_hardware_buffer_t& rhs) noexcept(false) {
    AHardwareBuffer_acquire(rhs.ptr);
    AHardwareBuffer_release(ptr);
    ptr = rhs.ptr;
    return *this;
}

ndk_hardware_buffer_t& ndk_hardware_buffer_t::operator=(ndk_hardware_buffer_t&& rhs) noexcept(false) {
    AHardwareBuffer_acquire(rhs.ptr);
    AHardwareBuffer_release(ptr);
    ptr = rhs.ptr;
    return *this;
}

jobject ndk_hardware_buffer_t::object(JNIEnv* env) const noexcept { return AHardwareBuffer_toHardwareBuffer(env, ptr); }

void ndk_hardware_buffer_t::send_to(int socket) noexcept(false) {
    if (AHardwareBuffer_sendHandleToUnixSocket(ptr, socket) != 0)
        throw std::runtime_error{"AHardwareBuffer_sendHandleToUnixSocket"};
}

void ndk_hardware_buffer_t::get(AHardwareBuffer_Desc& desc) const noexcept {
    AHardwareBuffer_describe(ptr, &desc);  // ...
}

void* ndk_hardware_buffer_t::lock(uint64_t usage, int32_t fence) noexcept(false) {
    void* mapping = nullptr;
    if (AHardwareBuffer_lock(ptr, usage, fence, nullptr, &mapping) != 0)
        throw std::runtime_error{"AHardwareBuffer_lock"};
    return mapping;
}

void ndk_hardware_buffer_t::unlock(int32_t* fence) noexcept(false) {
    if (AHardwareBuffer_unlock(ptr, fence) != 0) throw std::runtime_error{"AHardwareBuffer_unlock"};
}
