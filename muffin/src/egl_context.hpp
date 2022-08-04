#pragma once
#include <EGL/egl.h>
#include <EGL/eglext_angle.h>

class egl_context_t final {
   private:
    EGLDisplay display = EGL_NO_DISPLAY;  // EGL_NO_DISPLAY when `terminate`d
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;  // EGLSurface for Draw/Read
    EGLConfig config = EGL_NO_CONFIG_KHR;

   public:
    egl_context_t(EGLDisplay display, EGLConfig config, EGLContext share_context) noexcept;
    ~egl_context_t() noexcept;
    egl_context_t(egl_context_t const &) = delete;
    egl_context_t &operator=(egl_context_t const &) = delete;
    egl_context_t(egl_context_t &&) = delete;
    egl_context_t &operator=(egl_context_t &&) = delete;

    EGLContext handle() const noexcept;
    EGLDisplay get_display() const noexcept;
    EGLConfig get_config() const noexcept;

    EGLint resume(EGLSurface surface) noexcept;
    EGLint suspend() noexcept;
    void destroy() noexcept;

    uint32_t swap() noexcept;
};
