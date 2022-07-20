/**
 * @file    muffin.h
 * @author  github.com/luncliff (luncliff@gmail.com)
 * 
 * @version 1.1
 * @see https://programming.guide/java/list-of-java-exceptions.html
 */
#pragma once
// clang-format off
#if defined(FORCE_STATIC_LINK)
#   define _INTERFACE_
#   define _HIDDEN_
#elif defined(_MSC_VER) // MSVC or clang-cl
#   define _HIDDEN_
#   ifdef _WINDLL
#       define _INTERFACE_ __declspec(dllexport)
#   else
#       define _INTERFACE_ __declspec(dllimport)
#   endif
#elif defined(__GNUC__) || defined(__clang__)
#   define _INTERFACE_ __attribute__((visibility("default")))
#   define _HIDDEN_ __attribute__((visibility("hidden")))
#else
#   error "unexpected linking configuration"
#endif
// clang-format on
static_assert(__cplusplus >= 201703L, "requires C++ 17 or later");
#include <cerrno>
#include <experimental/coroutine>
// clang-format off
#if __has_include(<vulkan/vulkan.h>)
#  define VK_USE_PLATFORM_ANDROID_KHR
#  include <vulkan/vulkan.h>
#endif
#include <GLES3/gl3.h>
#if __has_include(<GLES3/gl31.h>)
#  include <GLES3/gl31.h>
#endif
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_angle.h>
// clang-format on
#if !defined(__ANDROID__) || !defined(__ANDROID_API__)
#error "requries __ANDROID__ and __ANDROID_API__"
#endif
#include <android/native_window_jni.h>

class _INTERFACE_ egl_surface_t final {
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLNativeWindowType window = nullptr;

   public:
    egl_surface_t(EGLDisplay display, EGLConfig config, EGLSurface surface,
                  EGLNativeWindowType window = nullptr) noexcept;
    ~egl_surface_t() noexcept;
    egl_surface_t(egl_surface_t const &) = delete;
    egl_surface_t &operator=(egl_surface_t const &) = delete;
    egl_surface_t(egl_surface_t &&) = delete;
    egl_surface_t &operator=(egl_surface_t &&) = delete;

    EGLSurface handle() const noexcept;
    EGLConfig get_config() const noexcept;
    uint32_t get_size(EGLint &width, EGLint &height) const noexcept;
};

_INTERFACE_
std::unique_ptr<egl_surface_t> make_egl_surface(EGLDisplay display,  //
                                                EGLint width,
                                                EGLint height) noexcept(false);

_INTERFACE_
std::unique_ptr<egl_surface_t> make_egl_surface(
    EGLDisplay display,  //
    JNIEnv *_env, jobject _surface) noexcept(false);

class _INTERFACE_ egl_context_t final {
   private:
    EGLDisplay display = EGL_NO_DISPLAY;  // EGL_NO_DISPLAY when `terminate`d
    EGLint version[2]{};                  // major, minor
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;  // EGLSurface for Draw/Read
    EGLConfig configs[2]{};

   public:
    egl_context_t(EGLDisplay display, EGLContext share_context) noexcept;
    ~egl_context_t() noexcept;
    egl_context_t(egl_context_t const &) = delete;
    egl_context_t &operator=(egl_context_t const &) = delete;
    egl_context_t(egl_context_t &&) = delete;
    egl_context_t &operator=(egl_context_t &&) = delete;

    EGLContext handle() const noexcept;
    EGLDisplay get_display() const noexcept;
    EGLConfig get_config(EGLNativeWindowType window) const noexcept;

    EGLint resume(EGLSurface surface) noexcept;
    EGLint suspend() noexcept;
    void destroy() noexcept;

    uint32_t swap() noexcept;
};
