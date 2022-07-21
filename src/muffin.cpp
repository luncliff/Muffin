#include <spdlog/spdlog.h>

#include <muffin.hpp>

std::string make_egl_message(EGLint ec) noexcept {
    return fmt::format("EGL {}({:x})", ec, ec);
}

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

uint32_t choose_config(EGLDisplay display, EGLConfig &config,
                       ANativeWindow *window) noexcept {
    switch (const auto format = ANativeWindow_getFormat(window)) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
            spdlog::debug("EGL surface format: {}", "R8G8B8A8_UNORM");
            return choose_config(display, config, hardware_format_rgba32);
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            spdlog::debug("EGL surface format: {}", "R8G8B8X8_UNORM");
            return choose_config(display, config, hardware_format_rgbx32);
        default:
            spdlog::error("unexpected surface format: {}", format);
            return ENOTSUP;
    }
}

egl_surface_t::egl_surface_t(EGLDisplay display, EGLConfig config,
                             EGLSurface surface,
                             EGLNativeWindowType window) noexcept
    : display{display}, config{config}, surface{surface}, window{window} {
    // ...
}

egl_surface_t::~egl_surface_t() noexcept {
    eglDestroySurface(display, surface);
    if (window) ANativeWindow_release(window);
}

EGLSurface egl_surface_t::handle() const noexcept { return surface; }
EGLConfig egl_surface_t::get_config() const noexcept { return config; }

uint32_t egl_surface_t::get_size(EGLint &width, EGLint &height) const noexcept {
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    if (auto ec = eglGetError(); ec != EGL_SUCCESS) return ec;
    return 0;
}

std::unique_ptr<egl_surface_t> make_egl_surface(EGLDisplay display,
                                                EGLint width,
                                                EGLint height) noexcept(false) {
    EGLConfig config = EGL_NO_CONFIG_KHR;
    if (auto ec =
            choose_config(display, config, static_cast<EGLint *>(nullptr)))
        throw std::runtime_error{make_egl_message(ec)};
    EGLint attrs[]{EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};
    EGLSurface surface = eglCreatePbufferSurface(display, config, attrs);
    if (auto ec = eglGetError(); ec != EGL_SUCCESS)
        throw std::runtime_error{make_egl_message(ec)};
    return std::make_unique<egl_surface_t>(display, config, surface);
}

std::unique_ptr<egl_surface_t> make_egl_surface(
    EGLDisplay display, JNIEnv *_env, jobject _surface) noexcept(false) {
    auto window = std::unique_ptr<ANativeWindow, void (*)(ANativeWindow *)>{
        ANativeWindow_fromSurface(_env, _surface), &ANativeWindow_release};
    EGLConfig config = EGL_NO_CONFIG_KHR;
    if (auto ec = choose_config(display, config, window.get()))
        throw std::runtime_error{make_egl_message(ec)};
    EGLint attrs[]{EGL_RENDER_BUFFER, EGL_BACK_BUFFER, EGL_NONE};
    EGLSurface surface =
        eglCreateWindowSurface(display, config, window.get(), attrs);
    if (auto ec = eglGetError(); ec != EGL_SUCCESS)
        throw std::runtime_error{make_egl_message(ec)};
    return std::make_unique<egl_surface_t>(display, config, surface,
                                           window.release());
}

egl_context_t::egl_context_t(EGLDisplay display,
                             EGLContext share_context) noexcept
    : display{display} {
    if (eglInitialize(display, version + 0, version + 1) == false) {
        spdlog::error("{}: {:#x}", "eglInitialize", eglGetError());
        return;
    }
    spdlog::debug("EGLDisplay {} {}.{}", display, version[0], version[1]);
    if (auto ec = choose_config(display, configs[0], hardware_format_rgba32))
        spdlog::error("{}: {:#x}", "eglChooseConfig", ec);
    if (auto ec = choose_config(display, configs[1], hardware_format_rgbx32))
        spdlog::error("{}: {:#x}", "eglChooseConfig", ec);

    // create context for OpenGL ES 3.0+
    EGLint attrs[]{EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0,
                   EGL_NONE};
    if (context = eglCreateContext(display, configs[0], share_context, attrs);
        context != EGL_NO_CONTEXT)
        spdlog::debug("EGL create: context {} {}", context, share_context);
}

egl_context_t::~egl_context_t() noexcept { destroy(); }

EGLContext egl_context_t::handle() const noexcept { return context; }

EGLDisplay egl_context_t::get_display() const noexcept { return display; }
EGLConfig egl_context_t::get_config(EGLNativeWindowType window) const noexcept {
    if (window == nullptr) return configs[0];
    return EGL_NO_CONFIG_KHR;
}

EGLint egl_context_t::resume(EGLSurface _surface) noexcept {
    if (context == EGL_NO_CONTEXT) return EGL_NOT_INITIALIZED;
    if (_surface == EGL_NO_SURFACE) return EGL_BAD_SURFACE;
    this->surface = _surface;
    spdlog::debug("EGL current: {}/{} {}", surface, surface, context);
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
    spdlog::debug("EGL current: EGL_NO_SURFACE/EGL_NO_SURFACE {}", context);
    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                       EGL_NO_CONTEXT) == EGL_FALSE)
        spdlog::error("{}: {:#x}", "eglMakeCurrent", eglGetError());
    // forget the known surface
    if (surface != EGL_NO_SURFACE) surface = EGL_NO_SURFACE;
    return 0;
}

void egl_context_t::destroy() noexcept {
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
    // forget known surface
    if (surface != EGL_NO_SURFACE) surface = EGL_NO_SURFACE;
    display = EGL_NO_DISPLAY;
}

uint32_t egl_context_t::swap() noexcept {
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
