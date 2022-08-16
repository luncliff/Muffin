package dev.luncliff.muffin;

import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;

import androidx.annotation.NonNull;

public class EGLContextOwner implements AutoCloseable {
    long display = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY).getNativeHandle();
    long config;
    long context;

    private static native long create1(long display, long config, long shared);
    private static native long create2(long display, long config, long ptr);

    private static native void destroy1(long display, long context);

    private static native int resume1(long display, long context, long surface);

    private static native int resume2(long display, long context, long ptr);

    private static native int suspend(long display, long context);

    private static native int present(long display, long context);

    EGLContextOwner(EGLDisplay display, EGLConfig config, EGLContext shared) throws RuntimeException, UnsatisfiedLinkError {
        Environment.Init();
        this.display = display.getNativeHandle();
        this.config = config.getNativeHandle();
        this.context = create1(this.display, this.config, shared.getNativeHandle());
    }

    EGLContextOwner(EGLSurfaceOwner surface) throws RuntimeException, UnsatisfiedLinkError {
        Environment.Init();
        this.display = surface.display;
        this.config = surface.config;
        this.context = create1(this.display, this.config, 0); // use EGL14.EGL_NO_CONTEXT
    }

    /**
     * @apiNote Create an EGL context with the shared context. It works like a clone.
     */
    EGLContextOwner(EGLContextOwner shared) throws RuntimeException {
        this.display = shared.display;
        this.config = shared.config;
        this.context = create2(this.display, this.config, shared.context);
    }

    @Override
    public void close() {
        destroy1(display, context);
    }

    int resume(EGLSurface surface) {
        return resume1(this.display, this.context, surface.getNativeHandle());
    }

    int resume(EGLSurfaceOwner surface) {
        return resume2(this.display, this.context, surface.surface);
    }

    int suspend() {
        return suspend(this.display, this.context);
    }

    int present(){
        return present(this.display, this.context);
    }

    @NonNull
    @Override
    public String toString() {
        return String.format("EGLContextOwner{display=%x, config=%x, ptr=%x}", display, config, context);
    }
}
