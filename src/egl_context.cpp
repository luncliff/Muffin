#include "egl_context.hpp"

#include <spdlog/spdlog.h>

uint32_t choose_config(EGLDisplay display, EGLConfig &config, ANativeWindow *window) noexcept;

egl_context_t::egl_context_t(EGLDisplay display, EGLConfig config, EGLContext share_context) noexcept
    : display{display}, config{config} {
    EGLint version[2]{};  // major, minor
    if (eglInitialize(display, version + 0, version + 1) == false) {
        spdlog::error("{}: {:#x}", "eglInitialize", eglGetError());
        return;
    }
    spdlog::debug("EGLDisplay {} {}.{}", display, version[0], version[1]);
    // create context for OpenGL ES 3.0+
    EGLint attrs[]{EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE};
    if (context = eglCreateContext(display, config, share_context, attrs); context != EGL_NO_CONTEXT)
        spdlog::debug("EGL create: context {} {}", context, share_context);
}

egl_context_t::~egl_context_t() noexcept { destroy(); }

EGLContext egl_context_t::handle() const noexcept { return context; }

EGLDisplay egl_context_t::get_display() const noexcept { return display; }
EGLConfig egl_context_t::get_config() const noexcept { return config; }

EGLint egl_context_t::resume(EGLSurface _surface) noexcept {
    if (context == EGL_NO_CONTEXT) return EGL_NOT_INITIALIZED;
    if (_surface == EGL_NO_SURFACE) return EGL_BAD_SURFACE;
    this->surface = _surface;
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        const auto ec = eglGetError();
        spdlog::error("{}: {:#x}", "eglQuerySurface", ec);
        return ec;
    }
    return 0;
}

EGLint egl_context_t::suspend() noexcept {
    if (context == EGL_NO_CONTEXT) return EGL_NOT_INITIALIZED;
    // unbind surface. OpenGL ES 3.1 will return true
    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_FALSE)
        spdlog::error("{}: {:#x}", "eglMakeCurrent", eglGetError());
    // forget the known surface
    if (surface != EGL_NO_SURFACE) surface = EGL_NO_SURFACE;
    return 0;
}

void egl_context_t::destroy() noexcept {
    if (display == EGL_NO_DISPLAY)  // already terminated
        return;

    // unbind surface and context
    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_FALSE) {
        spdlog::error("{}: {:#x}", "eglMakeCurrent", eglGetError());
        return;
    }
    // destroy known context
    if (context != EGL_NO_CONTEXT) {
        spdlog::warn("EGL destroy: context {}", context);
        if (eglDestroyContext(display, context) == EGL_FALSE)
            spdlog::error("{}: {:#x}", "eglDestroyContext", eglGetError());
        context = EGL_NO_CONTEXT;
    }
    // forget known surface
    if (surface != EGL_NO_SURFACE) surface = EGL_NO_SURFACE;
    display = EGL_NO_DISPLAY;
}

uint32_t egl_context_t::swap() noexcept {
    if (eglSwapBuffers(display, surface)) return 0;
    switch (const auto ec = eglGetError()) {
        case EGL_BAD_CONTEXT:
        case EGL_CONTEXT_LOST:
            destroy();
            [[fallthrough]];
        default:
            return ec;  // EGL_BAD_SURFACE and the others ...
    }
}
