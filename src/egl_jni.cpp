#include <android/native_window_jni.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <string>
#include <string_view>

#include "muffin.hpp"

void store_runtime_exception(JNIEnv* env, const char* message) noexcept;

extern "C" {

JNIEXPORT jlong Java_dev_luncliff_muffin_EGLSurfaceOwner_query(JNIEnv*, jclass, jlong, jlong s) {
    auto surface = reinterpret_cast<egl_surface_t*>(s);
    return reinterpret_cast<jlong>(surface->get_config());
}

JNIEXPORT void Java_dev_luncliff_muffin_EGLSurfaceOwner_destroy1(JNIEnv*, jclass, jlong, jlong s) {
    auto surface = reinterpret_cast<egl_surface_t*>(s);
    delete surface;
}

JNIEXPORT jlong Java_dev_luncliff_muffin_EGLSurfaceOwner_create2(JNIEnv* env, jclass, jlong d, jint w, jint h) {
    try {
        auto es_display = reinterpret_cast<EGLDisplay>(d);
        if (eglInitialize(es_display, nullptr, nullptr) == EGL_FALSE)
            throw std::system_error(eglGetError(), std::generic_category(), "eglInitialize");

        auto surface = make_egl_surface(es_display, w, h);
        return reinterpret_cast<jlong>(surface.release());
    } catch (const std::exception& ex) {
        spdlog::error("{:s}: {:s}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
        return reinterpret_cast<jlong>(EGL_NO_SURFACE);
    }
}

JNIEXPORT jlong Java_dev_luncliff_muffin_EGLSurfaceOwner_create1(JNIEnv* env, jclass, jlong d, jobject window) {
    try {
        auto es_display = reinterpret_cast<EGLDisplay>(d);
        if (eglInitialize(es_display, nullptr, nullptr) == EGL_FALSE)
            throw std::system_error(eglGetError(), std::generic_category(), "eglInitialize");

        auto surface = make_egl_surface(es_display, env, window);
        return reinterpret_cast<jlong>(surface.release());
    } catch (const std::exception& ex) {
        spdlog::error("{:s}: {:s}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
        return reinterpret_cast<jlong>(EGL_NO_SURFACE);
    }
}

JNIEXPORT jlong Java_dev_luncliff_muffin_EGLContextOwner_create1(JNIEnv* env, jclass, jlong d, jlong c, jlong shared) {
    try {
        auto es_display = reinterpret_cast<EGLDisplay>(d);
        if (eglInitialize(es_display, nullptr, nullptr) == EGL_FALSE)
            throw std::system_error(eglGetError(), std::generic_category(), "eglInitialize");

        auto es_config = reinterpret_cast<EGLConfig>(c);
        auto owner = std::make_unique<egl_context_t>(es_display, es_config, reinterpret_cast<EGLContext>(shared));
        return reinterpret_cast<jlong>(owner.release());
    } catch (const std::exception& ex) {
        spdlog::error("{:s}: {:s}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
        return reinterpret_cast<jlong>(EGL_NO_CONTEXT);
    }
}

JNIEXPORT jlong Java_dev_luncliff_muffin_EGLContextOwner_create2(JNIEnv* env, jclass clazz, jlong d, jlong c, jlong p) {
    auto ctx = reinterpret_cast<egl_context_t*>(p);
    return Java_dev_luncliff_muffin_EGLContextOwner_create1(env, clazz, d, c, reinterpret_cast<jlong>(ctx->handle()));
}

JNIEXPORT void Java_dev_luncliff_muffin_EGLContextOwner_destroy1(JNIEnv*, jclass, jlong, jlong c) {
    auto ptr = reinterpret_cast<egl_context_t*>(c);
    delete ptr;
}

JNIEXPORT jint Java_dev_luncliff_muffin_EGLContextOwner_resume1(JNIEnv* env, jclass, jlong, jlong c, jlong s) {
    try {
        auto ptr = reinterpret_cast<egl_context_t*>(c);
        if (EGLint ec = ptr->resume(reinterpret_cast<EGLSurface>(s)); ec != 0) {
            spdlog::error("{}: {}", "egl_context_t::resume", ec);
            return ec;
        }
        return 0;
    } catch (const std::exception& ex) {
        spdlog::error("{:s}: {:s}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
        return EXIT_FAILURE;
    }
}

JNIEXPORT jint Java_dev_luncliff_muffin_EGLContextOwner_resume2(JNIEnv* env, jclass clazz, jlong d, jlong c, jlong s) {
    auto surface = reinterpret_cast<egl_surface_t*>(s);
    return Java_dev_luncliff_muffin_EGLContextOwner_resume1(env, clazz, d, c,
                                                            reinterpret_cast<jlong>(surface->handle()));
}

JNIEXPORT jint Java_dev_luncliff_muffin_EGLContextOwner_suspend(JNIEnv* env, jclass, jlong, jlong c) {
    try {
        auto ptr = reinterpret_cast<egl_context_t*>(c);
        if (EGLint ec = ptr->suspend(); ec != 0) {
            spdlog::error("{}: {}", "egl_context_t::resume", ec);
            return ec;
        }
        return 0;
    } catch (const std::exception& ex) {
        spdlog::error("{:s}: {:s}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
        return EXIT_FAILURE;
    }
}

JNIEXPORT jint Java_dev_luncliff_muffin_EGLContextOwner_present(JNIEnv* env, jclass, jlong, jlong c) {
    try {
        auto ptr = reinterpret_cast<egl_context_t*>(c);
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        return ptr->swap();
    } catch (const std::exception& ex) {
        spdlog::error("{:s}: {:s}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
        return EXIT_FAILURE;
    }
}

}  // extern "C"
