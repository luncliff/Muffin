package dev.luncliff.muffin;

import android.content.Context;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.util.Log;
import android.view.Surface;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import java.util.concurrent.Executor;

/**
 * @author luncliff@gmail.com
 */
public class Renderer1 implements AutoCloseable {
  static {
    System.loadLibrary("c++_shared");
    System.loadLibrary("muffin");
  }

  private final long context;
  private long surface;

  private static native long create1(long display, long context);
  private static native void destroy1(long surface);
  private static native long create2(long context, Surface window);
  private static native void destroy2(long surface);
  private static native int resume(long context, long surface);
  private static native int suspend(long context);
  private static native int present(long context);

  public Renderer1(Context ctx, EGLDisplay display, EGLContext shared){
    context = create1(display.getNativeHandle(), shared.getNativeHandle());
    if (context == 0)
      throw new RuntimeException("failed to create");
  }

  @Override
  public void close(){
    final int ec = suspend();
    if (ec != 0)
      Log.w("Renderer1", String.format("failed to pause: %d", ec));
    destroy1(context);
  }

  public int resume(Surface window) {
    surface = create2(context, window);
    return resume(context, surface);
  }
  public int suspend() {
    final int ec = suspend(context);
    if (surface != 0)
      destroy2(surface);
    surface = 0;
    return ec;
  }

  public int present() {
    return present(context);
  }

  @NonNull
  public native String toString();
}
