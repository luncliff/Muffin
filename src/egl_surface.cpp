#include "egl_surface.hpp"

#include <spdlog/spdlog.h>

std::string make_egl_message(EGLint ec) noexcept { return fmt::format("EGL {}({:x})", ec, ec); }

EGLint hardware_format_rgba32[]{EGL_RENDERABLE_TYPE,
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
EGLint hardware_format_rgbx32[]{EGL_RENDERABLE_TYPE,
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

EGLint choose_config(EGLDisplay display, EGLConfig &config, const EGLint *attrs) noexcept {
    constexpr auto color_size = 8;
    constexpr auto depth_size = 16;
    EGLint backup_attrs[]{EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                          EGL_BLUE_SIZE,       color_size,         EGL_GREEN_SIZE,   color_size,
                          EGL_RED_SIZE,        color_size,         EGL_ALPHA_SIZE,   color_size,
                          EGL_DEPTH_SIZE,      depth_size,         EGL_NONE};
    if (attrs == nullptr) attrs = backup_attrs;
    EGLint count = 0;
    if (eglChooseConfig(display, attrs, &config, 1, &count) == EGL_FALSE) return eglGetError();
    return 0;
}

uint32_t choose_config(EGLDisplay display, EGLConfig &config, ANativeWindow *window) noexcept {
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

EGLConfig get_default_config(EGLDisplay display) noexcept(false) {
    EGLint *attrs = nullptr;
    EGLConfig config = EGL_NO_CONFIG_KHR;
    if (auto ec = choose_config(display, config, attrs); ec != 0)
        throw std::system_error{ec, std::generic_category(), "eglChooseConfig"};
    return config;
}

egl_surface_t::egl_surface_t(EGLDisplay display, EGLSurface surface, EGLNativeWindowType window) noexcept(false)
    : egl_surface_t{display, EGL_NO_CONFIG_KHR, surface, window} {
    if (int ec = choose_config(display, config, window); ec)
        throw std::system_error{ec, std::generic_category(), __func__};
}

egl_surface_t::egl_surface_t(EGLDisplay display, EGLConfig config, EGLSurface surface,
                             EGLNativeWindowType window) noexcept(false)
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

std::unique_ptr<egl_surface_t> make_egl_surface(EGLDisplay display, EGLint width, EGLint height) noexcept(false) {
    EGLNativeWindowType window = nullptr;
    EGLConfig config = EGL_NO_CONFIG_KHR;
    if (auto ec = choose_config(display, config, static_cast<EGLint *>(nullptr)))
        throw std::runtime_error{make_egl_message(ec)};
    EGLint attrs[]{EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};
    EGLSurface surface = eglCreatePbufferSurface(display, config, attrs);
    if (auto ec = eglGetError(); ec != EGL_SUCCESS) throw std::runtime_error{make_egl_message(ec)};
    return std::make_unique<egl_surface_t>(display, config, surface, window);
}

std::unique_ptr<egl_surface_t> make_egl_surface(EGLDisplay display, JNIEnv *_env, jobject _surface) noexcept(false) {
    auto window = std::unique_ptr<ANativeWindow, void (*)(ANativeWindow *)>{ANativeWindow_fromSurface(_env, _surface),
                                                                            &ANativeWindow_release};
    EGLConfig config = EGL_NO_CONFIG_KHR;
    if (auto ec = choose_config(display, config, window.get())) throw std::runtime_error{make_egl_message(ec)};
    EGLint attrs[]{EGL_RENDER_BUFFER, EGL_BACK_BUFFER, EGL_NONE};
    EGLSurface surface = eglCreateWindowSurface(display, config, window.get(), attrs);
    if (auto ec = eglGetError(); ec != EGL_SUCCESS) throw std::runtime_error{make_egl_message(ec)};
    return std::make_unique<egl_surface_t>(display, config, surface, window.release());
}
