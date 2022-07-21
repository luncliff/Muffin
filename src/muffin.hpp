/**
 * @file    muffin.h
 * @author  github.com/luncliff (luncliff@gmail.com)
 * 
 * @see https://programming.guide/java/list-of-java-exceptions.html
 */
#pragma once
#include <EGL/egl.h>
#include <EGL/eglext_angle.h>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>

#include <cerrno>
#include <experimental/coroutine>
// clang-format on
#if !defined(__ANDROID__) || !defined(__ANDROID_API__)
#error "requries __ANDROID__ and __ANDROID_API__"
#endif
#include <android/native_window_jni.h>

static_assert(__cplusplus >= 201703L, "requires C++ 17 or later");

class JNIEXPORT egl_surface_t final {
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

JNIEXPORT
std::unique_ptr<egl_surface_t> make_egl_surface(EGLDisplay display,  //
                                                EGLint width,
                                                EGLint height) noexcept(false);

JNIEXPORT
std::unique_ptr<egl_surface_t> make_egl_surface(
    EGLDisplay display,  //
    JNIEnv *_env, jobject _surface) noexcept(false);

class JNIEXPORT egl_context_t final {
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
