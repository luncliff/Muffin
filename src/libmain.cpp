/**
 * @author Park DongHa (luncliff@gmail.com)
 */
#include <muffin.hpp>

#include <gsl/gsl>
#include <spdlog/spdlog.h>
#if __has_include(<EGL/eglext_angle.h>)
#include <EGL/eglext_angle.h>
#endif

#define LIB_PROLOGUE __attribute__((constructor))
#define LIB_EPILOGUE __attribute__((destructor))

egl_context_t::egl_context_t(EGLDisplay display,
                             EGLContext share_context) noexcept {
    spdlog::debug(__FUNCTION__);
    // remember the EGLDisplay
    this->display = display;
    EGLint version[2]{};
    if (eglInitialize(display, version + 0, version + 1) == false) {
        spdlog::error("{}: {:#x}", "eglInitialize", eglGetError());
        return;
    }
    major = gsl::narrow<uint16_t>(version[0]);
    minor = gsl::narrow<uint16_t>(version[1]);
    spdlog::debug("EGLDisplay {} {}.{}", display, major, minor);

    // acquire EGLConfigs
    EGLint num_config = 1;
    if (auto ec = get_configs(configs, num_config, nullptr)) {
        spdlog::error("{}: {:#x}", "eglChooseConfig", eglGetError());
        return;
    }

    // create context for OpenGL ES 3.0+
    EGLint attrs[]{EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0,
                   EGL_NONE};
    if (context = eglCreateContext(display, configs[0], share_context, attrs);
        context != EGL_NO_CONTEXT)
        spdlog::debug("EGL create: context {} {}", context, share_context);
}

bool egl_context_t::is_valid() const noexcept {
    return context != EGL_NO_CONTEXT;
}

egl_context_t::~egl_context_t() noexcept {
    spdlog::debug(__FUNCTION__);
    destroy();
}

EGLint egl_context_t::resume(EGLSurface es_surface, EGLConfig) noexcept {
    spdlog::trace(__FUNCTION__);
    if (context == EGL_NO_CONTEXT)
        return EGL_NOT_INITIALIZED;
    if (es_surface == EGL_NO_SURFACE)
        return GL_INVALID_VALUE;

    surface = es_surface;
    spdlog::debug("EGL current: {}/{} {}", surface, surface, context);
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        const auto ec = eglGetError();
        spdlog::error("{}: {:#x}", "eglQuerySurface", ec);
        return ec;
    }
    return 0;
}

EGLint egl_context_t::suspend() noexcept {
    spdlog::trace(__FUNCTION__);
    if (context == EGL_NO_CONTEXT)
        return EGL_NOT_INITIALIZED;

    // unbind surface. OpenGL ES 3.1 will return true
    spdlog::debug("EGL current: EGL_NO_SURFACE/EGL_NO_SURFACE {}", context);
    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context) ==
        EGL_FALSE) {
        // OpenGL ES 3.0 will report error. consume it
        // then unbind both surface and context.
        spdlog::error("{}: {:#x}", "eglMakeCurrent", eglGetError());
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    // forget known surface
    if (surface != EGL_NO_SURFACE)
        surface = EGL_NO_SURFACE;
    return 0;
}

void egl_context_t::destroy() noexcept {
    spdlog::trace(__FUNCTION__);
    if (display == EGL_NO_DISPLAY) // already terminated
        return;

    // unbind surface and context
    spdlog::debug("EGL current: EGL_NO_SURFACE/EGL_NO_SURFACE EGL_NO_CONTEXT");
    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                       EGL_NO_CONTEXT) == EGL_FALSE) {
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
    if (surface != EGL_NO_SURFACE)
        surface = EGL_NO_SURFACE;
    display = EGL_NO_DISPLAY;
}

EGLint egl_context_t::swap() noexcept {
    if (eglSwapBuffers(display, surface))
        return 0;
    switch (const auto ec = eglGetError()) {
    case EGL_BAD_CONTEXT:
    case EGL_CONTEXT_LOST:
        destroy();
        [[fallthrough]];
    default:
        return ec; // EGL_BAD_SURFACE and the others ...
    }
}

EGLContext egl_context_t::handle() const noexcept { return context; }

EGLint egl_context_t::get_configs(EGLConfig *configs, EGLint &count,
                                  const EGLint *attrs) const noexcept {
    constexpr auto color_size = 8;
    constexpr auto depth_size = 16;
    EGLint backup_attrs[]{EGL_RENDERABLE_TYPE,
                          EGL_OPENGL_ES2_BIT,
                          EGL_SURFACE_TYPE,
                          EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                          EGL_BLUE_SIZE,
                          color_size,
                          EGL_GREEN_SIZE,
                          color_size,
                          EGL_RED_SIZE,
                          color_size,
                          EGL_ALPHA_SIZE,
                          color_size,
                          EGL_DEPTH_SIZE,
                          depth_size,
                          EGL_NONE};
    if (attrs == nullptr)
        attrs = backup_attrs;
    if (eglChooseConfig(this->display, attrs, configs, count, &count) ==
        EGL_FALSE)
        return eglGetError();
    return 0;
}
