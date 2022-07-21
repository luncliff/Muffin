package dev.luncliff.muffin;

import android.content.Context;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.util.Log;
import android.view.Surface;
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

  private final long pcontext;
  private long psurface;
  private final Executor background;

  public Renderer1(Context context, Executor executor, EGLDisplay display, EGLContext shared)
      throws RuntimeException {
    if (executor == null)
      executor = ContextCompat.getMainExecutor(context);
    background = executor;
    pcontext = create1(background, display.getNativeHandle(), shared.getNativeHandle());
    if (pcontext == 0)
      throw new RuntimeException("failed to create");
  }

  private static native long create1(Executor executor, long display, long context)
      throws RuntimeException;
  private static native void destroy1(long surface);

  @Override
  public void close() throws RuntimeException {
    final int ec = suspend();
    if (ec != 0)
      Log.w("Renderer1", String.format("failed to pause: %d", ec));
    destroy1(pcontext);
  }

  private static native long create2(long context, Surface surface, Executor executor)
      throws RuntimeException;
  private static native void destroy2(long surface);

  private static native int resume(long context, long surface);
  // @todo throw if error code
  public int resume(Surface surface) throws RuntimeException {
    psurface = create2(pcontext, surface, background);
    return resume(pcontext, psurface);
  }
  private static native int suspend(long context, Executor executor);
  // @todo throw if error code
  public int suspend() throws RuntimeException {
    final int ec = suspend(pcontext, background);
    if (psurface != 0)
      destroy2(psurface);
    psurface = 0;
    return ec;
  }

  private static native int present(long context);
  public int present() throws RuntimeException {
    return present(pcontext);
  }

  public native String toString();
}
