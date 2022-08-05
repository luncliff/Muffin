package dev.luncliff.muffin;

import android.opengl.EGL14;
import android.view.Surface;

import androidx.annotation.NonNull;

public class EGLSurfaceOwner implements AutoCloseable {
    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("muffin");
    }

    final long display = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY).getNativeHandle();
    final long config;
    final long surface;

    private static native long create1(long display, Surface window);

    private static native long create2(long display, int width, int height);

    private static native void destroy1(long display, long surface);

    private static native long query(long display, long surface);

    public EGLSurfaceOwner(Surface window) throws RuntimeException {
        surface = create1(display, window); // throw in native code
        config = query(display, surface);
    }

    public EGLSurfaceOwner(int width, int height) throws RuntimeException {
        surface = create2(display, width, height); // throw in native code
        config = query(display, surface);
    }

    @Override
    public void close() {
        destroy1(display, surface);
    }

    @NonNull
    @Override
    public String toString() {
        return String.format("EGLSurfaceOwner{display=%x, config=%x, ptr=%x}", display, config, surface);
    }
}
