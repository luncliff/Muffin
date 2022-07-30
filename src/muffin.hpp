#pragma once
#if !defined(__ANDROID__) || !defined(__ANDROID_API__)
#error "requries __ANDROID__ and __ANDROID_API__"
#endif
static_assert(__cplusplus >= 201703L, "requires C++ 17 or later");

#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <android/api-level.h>
#include <android/hardware_buffer.h>

#include <cerrno>
#include <experimental/coroutine>

#include "egl_context.hpp"
#include "egl_surface.hpp"

class native_loader_t final {
    void* handle;

   public:
    native_loader_t() noexcept = default;
    explicit native_loader_t(const char* libname) noexcept(false);
    ~native_loader_t() noexcept;

    void load(const char* libname) noexcept(false);
    void* get_proc_address(const char* proc) const noexcept;
};
