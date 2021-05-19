/**
 * @file    libmain.cpp
 * @author  github.com/luncliff (luncliff@gmail.com)
 */
#include <spdlog/spdlog.h>

#include <gsl/gsl>
#include <muffin.hpp>
#if __has_include(<EGL/eglext_angle.h>)
#include <EGL/eglext_angle.h>
#endif

static EGLint hardware_format_rgba32[]{EGL_RENDERABLE_TYPE,
                                       EGL_OPENGL_ES2_BIT,
                                       EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                       EGL_BLUE_SIZE,
                                       8,
                                       EGL_GREEN_SIZE,
                                       8,
                                       EGL_RED_SIZE,
                                       8,
                                       EGL_ALPHA_SIZE,
                                       8,
                                       EGL_NONE};
static EGLint hardware_format_rgbx32[]{EGL_RENDERABLE_TYPE,
                                       EGL_OPENGL_ES2_BIT,
                                       EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                       EGL_BLUE_SIZE,
                                       8,
                                       EGL_GREEN_SIZE,
                                       8,
                                       EGL_RED_SIZE,
                                       8,
                                       EGL_ALPHA_SIZE,
                                       0,
                                       EGL_NONE};
static EGLint hardware_format_rgb24[]{EGL_RENDERABLE_TYPE,
                                      EGL_OPENGL_ES2_BIT,
                                      EGL_SURFACE_TYPE,
                                      EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                      EGL_BLUE_SIZE,
                                      8,
                                      EGL_GREEN_SIZE,
                                      8,
                                      EGL_RED_SIZE,
                                      8,
                                      EGL_NONE};
static EGLint hardware_format_rgb565[]{EGL_RENDERABLE_TYPE,
                                       EGL_OPENGL_ES2_BIT,
                                       EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                       EGL_BLUE_SIZE,
                                       5,
                                       EGL_GREEN_SIZE,
                                       6,
                                       EGL_RED_SIZE,
                                       5,
                                       EGL_NONE};

EGLint choose_config(EGLDisplay display, EGLConfig &config,
                     const EGLint *attrs) noexcept {
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
    if (attrs == nullptr) attrs = backup_attrs;
    EGLint count = 0;
    if (eglChooseConfig(display, attrs, &config, 1, &count) == EGL_FALSE)
        return eglGetError();
    return 0;
}

egl_context_t::egl_context_t(EGLDisplay display,
                             EGLContext share_context) noexcept {
    spdlog::debug(__FUNCTION__);
    // remember the EGLDisplay
    this->display = display;
    if (eglInitialize(display, version + 0, version + 1) == false) {
        spdlog::error("{}: {:#x}", "eglInitialize", eglGetError());
        return;
    }
    spdlog::debug("EGLDisplay {} {}.{}", display, version[0], version[1]);

    // acquire EGLConfigs
    if (auto ec = choose_config(display, configs[0], hardware_format_rgba32))
        spdlog::error("{}: {:#x}", "eglChooseConfig", ec);
    if (auto ec = choose_config(display, configs[1], hardware_format_rgbx32))
        spdlog::error("{}: {:#x}", "eglChooseConfig", ec);
    if (auto ec = choose_config(display, configs[2], hardware_format_rgb24))
        spdlog::error("{}: {:#x}", "eglChooseConfig", ec);
    if (auto ec = choose_config(display, configs[3], hardware_format_rgb565))
        spdlog::error("{}: {:#x}", "eglChooseConfig", ec);

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
    spdlog::debug(__FUNCTION__);
    if (context == EGL_NO_CONTEXT) return EGL_NOT_INITIALIZED;
    if (es_surface == EGL_NO_SURFACE) return EGL_BAD_SURFACE;

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
    spdlog::debug(__FUNCTION__);
    if (context == EGL_NO_CONTEXT) return EGL_NOT_INITIALIZED;

    // unbind surface. OpenGL ES 3.1 will return true
    spdlog::debug("EGL current: EGL_NO_SURFACE/EGL_NO_SURFACE {}", context);
    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context) ==
        EGL_FALSE) {
        // OpenGL ES 3.0 will report error. consume it
        // then unbind both surface and context.
        spdlog::error("{}: {:#x}", "eglMakeCurrent", eglGetError());
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    // destroy/forget known surface
    if (surface != EGL_NO_SURFACE) {
        if (eglDestroySurface(display, surface) == EGL_FALSE)
            spdlog::error("{}: {:#x}", "eglDestroySurface", eglGetError());
        surface = EGL_NO_SURFACE;
        window = nullptr;
    }
    return 0;
}

void egl_context_t::destroy() noexcept {
    spdlog::debug(__FUNCTION__);
    if (display == EGL_NO_DISPLAY)  // already terminated
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
    // destroy/forget known surface
    if (surface != EGL_NO_SURFACE) {
        if (eglDestroySurface(display, surface) == EGL_FALSE)
            spdlog::error("{}: {:#x}", "eglDestroySurface", eglGetError());
        surface = EGL_NO_SURFACE;
        window = nullptr;
    }
    display = EGL_NO_DISPLAY;
}

EGLint egl_context_t::swap() noexcept {
    if (eglSwapBuffers(display, surface)) {
        spdlog::debug("EGL swap buffers: {} {}", display, surface);
        return 0;
    }
    switch (const auto ec = eglGetError()) {
        case EGL_BAD_CONTEXT:
        case EGL_CONTEXT_LOST:
            destroy();
            [[fallthrough]];
        default:
            return ec;  // EGL_BAD_SURFACE and the others ...
    }
}

EGLContext egl_context_t::handle() const noexcept { return context; }

EGLint egl_context_t::resume(JNIEnv *_env, jobject _surface) noexcept {
    spdlog::debug(__FUNCTION__);
    window = {ANativeWindow_fromSurface(_env, _surface),
              &ANativeWindow_release};
    EGLConfig config = EGL_NO_CONFIG_KHR;
    switch (const auto format = ANativeWindow_getFormat(window.get())) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
            spdlog::debug("EGL surface format: {}", "R8G8B8A8_UNORM");
            config = configs[0];
            break;
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            spdlog::debug("EGL surface format: {}", "R8G8B8X8_UNORM");
            config = configs[1];
            break;
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
            // config = configs[2];
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
            // config = configs[3];
            spdlog::warn("unsupported surface format: {}", format);
        default:
            spdlog::error("unexpected surface format: {}", format);
            return ENOTSUP;
    }
    EGLint attrs[]{EGL_RENDER_BUFFER, EGL_BACK_BUFFER, EGL_NONE};
    EGLSurface es_surface =
        eglCreateWindowSurface(display, config, window.get(), attrs);
    if (es_surface == EGL_NO_SURFACE) {
        auto ec = eglGetError();
        spdlog::error("{}: {:#x}", "eglCreateWindowSurface", ec);
        return ec;
    }
    return resume(es_surface, EGL_NO_CONFIG_KHR);
}
