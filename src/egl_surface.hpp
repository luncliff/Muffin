#pragma once
#include <EGL/egl.h>
#include <EGL/eglext_angle.h>
#include <android/native_window_jni.h>

#include <memory>

class egl_surface_t final {
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLNativeWindowType window = nullptr;

   public:
    egl_surface_t(EGLDisplay display, EGLConfig config, EGLSurface surface, EGLNativeWindowType window) noexcept(false);
    egl_surface_t(EGLDisplay display, EGLSurface surface, EGLNativeWindowType window) noexcept(false);
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
EGLConfig get_default_config(EGLDisplay display) noexcept(false);

JNIEXPORT
std::unique_ptr<egl_surface_t> make_egl_surface(EGLDisplay display,  //
                                                EGLint width, EGLint height) noexcept(false);

JNIEXPORT
std::unique_ptr<egl_surface_t> make_egl_surface(EGLDisplay display,  //
                                                JNIEnv *_env, jobject _surface) noexcept(false);
