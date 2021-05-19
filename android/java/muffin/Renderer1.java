package muffin;

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

  private final long ptr;

  public Renderer1(Context context, Executor executor, EGLDisplay display, EGLContext shared)
      throws RuntimeException {
    if (executor == null)
      executor = ContextCompat.getMainExecutor(context);
    ptr = create(executor, display.getNativeHandle(), shared.getNativeHandle());
    if (ptr == 0)
      throw new RuntimeException("failed to create");
  }

  private static native long create(Executor executor, long display, long context)
      throws RuntimeException;
  private static native void destroy(long ptr);
  @Override
  public void close() throws RuntimeException {
    final int ec = suspend(ptr);
    if (ec != 0)
      Log.w("Renderer1", String.format("failed to pause: %d", ec));
    destroy(ptr);
  }

  private static native int resume(long ptr, Surface surface);
  // @todo throw if error code
  public int resume(Surface surface) throws RuntimeException {
    return resume(ptr, surface);
  }
  private static native int suspend(long ptr);
  // @todo throw if error code
  public int suspend() throws RuntimeException {
    return suspend(ptr);
  }

  private static native int present(long ptr);
  public int present() throws RuntimeException {
    return present(ptr);
  }

  public native String toString();
}
