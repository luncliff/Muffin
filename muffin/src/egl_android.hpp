#pragma once
#include <EGL/egl.h>
#include <EGL/eglext_angle.h>

#include <string_view>
#include <vector>

std::vector<std::string_view> get_egl_extensions(EGLDisplay display) noexcept;

struct egl_android_t final {
    bool blob_cache;
    bool create_native_client_buffer;
    bool framebuffer_target;
    bool front_buffer_auto_refresh;
    bool get_frame_timestamps;
    bool get_native_client_buffer;
    bool GLES_layers;
    bool image_native_buffer;
    bool native_fence_sync;
    bool presentation_time;
    bool recordable;
};

void get_egl_extensions(EGLDisplay display, egl_android_t& info) noexcept;
