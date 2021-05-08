/**
 * @file    muffin.h
 * @author  github.com/luncliff (luncliff@gmail.com)
 * 
 * @version 1.1
 * @see https://programming.guide/java/list-of-java-exceptions.html
 */
#pragma once
// clang-format off
#if defined(FORCE_STATIC_LINK)
#   define _INTERFACE_
#   define _HIDDEN_
#elif defined(_MSC_VER) // MSVC or clang-cl
#   define _HIDDEN_
#   ifdef _WINDLL
#       define _INTERFACE_ __declspec(dllexport)
#   else
#       define _INTERFACE_ __declspec(dllimport)
#   endif
#elif defined(__GNUC__) || defined(__clang__)
#   define _INTERFACE_ __attribute__((visibility("default")))
#   define _HIDDEN_ __attribute__((visibility("hidden")))
#else
#   error "unexpected linking configuration"
#endif
// clang-format on
#if !defined(__ANDROID__) || !defined(__ANDROID_API__)
#error "requries __ANDROID__ and __ANDROID_API__"
#endif
static_assert(__cplusplus >= 201703L, "requires C++ 17 or later");
#include <new>
#include <cerrno>
#include <experimental/coroutine>
// clang-format off
#if __has_include(<vulkan/vulkan.h>)
#  include <vulkan/vulkan.h>
#endif
#if __has_include(<GLES3/gl3.h>)
#  include <GLES3/gl3.h>
#  if __has_include(<GLES3/gl31.h>)
#    include <GLES3/gl31.h>
#  endif
#  include <EGL/egl.h>
#  include <EGL/eglext.h>
#endif
// clang-format on

/**
 * @brief `EGLContext` and `EGLSurface` owner.
 *        Bind/unbind with `EGLNativeWindowType` using `resume`/`suspend` 
 * @see   https://www.saschawillems.de/blog/2015/04/19/using-opengl-es-on-windows-desktops-via-egl/
 */
class _INTERFACE_ egl_context_t final {
  private:
    EGLDisplay display = EGL_NO_DISPLAY; // EGL_NO_DISPLAY when `terminate`d
    uint16_t major = 0, minor = 0;
    EGLContext context = EGL_NO_CONTEXT;
    EGLConfig configs[1]{};
    EGLSurface surface = EGL_NO_SURFACE; // EGLSurface for Draw/Read

  public:
    /**
     * @brief Acquire EGLDisplay and create an EGLContext for OpenGL ES 3.0+
     * 
     * @see eglInitialize https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglInitialize.xhtml
     * @see eglChooseConfig
     * @see eglCreateContext
     */
    egl_context_t(EGLDisplay display, EGLContext share_context) noexcept;
    /**
     * @see terminate
     */
    ~egl_context_t() noexcept;
    egl_context_t(egl_context_t const &) = delete;
    egl_context_t &operator=(egl_context_t const &) = delete;
    egl_context_t(egl_context_t &&) = delete;
    egl_context_t &operator=(egl_context_t &&) = delete;

    /**
     * @brief EGLContext == NULL?
     * @details It is recommended to invoke this function to check whether the construction was successful.
     *          Notice that the constructor is `noexcept`.
     */
    bool is_valid() const noexcept;

    /**
     * @brief Destroy all EGL bindings and resources
     * @note This functions in invoked in the destructor
     * @post is_valid() == false
     * 
     * @see eglMakeCurrent
     * @see eglDestroyContext
     * @see eglDestroySurface
     * @see eglTerminate(unused) https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglTerminate.xhtml
     */
    void destroy() noexcept;

    /**
     * @brief   Take ownership of the given EGLSurface
     * 
     * @param es_surface  Expect PBufferSurface.
     * @param es_config   Hint to prevent misuse of `resume(EGLNativeWindowType)`. 
     *                    Always ignored. 
     */
    EGLint resume(EGLSurface es_surface, EGLConfig es_config) noexcept;

    /**
     * @brief   Unbind EGLSurface and EGLContext.
     * 
     * @return EGLint   `0` if successful. Redirected from `eglGetError`. 
     *                  `EGL_NOT_INITIALIZED` if `terminate` is invoked.
     * @see eglMakeCurrent
     * @see eglDestroySurface
     */
    EGLint suspend() noexcept;

    /**
     * @brief   Try to swap front/back buffer. 
     * @note    This function invokes `terminate` if EGL_BAD_CONTEXT/EGL_CONTEXT_LOST.
     * @return  EGLint  `0` if successful. Redirected from `eglGetError`. 
     * @see eglSwapBuffers
     * @see terminate
     */
    EGLint swap() noexcept;

    EGLContext handle() const noexcept;

    EGLint get_configs(EGLConfig *configs, EGLint &count,
                       const EGLint *attrs = nullptr) const noexcept;
};
