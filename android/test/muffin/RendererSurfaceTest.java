package muffin;

import android.content.Context;
import android.graphics.PixelFormat;
import android.media.Image;
import android.media.ImageReader;
import android.opengl.EGL14;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.os.Handler;
import android.util.Log;
import androidx.test.core.app.ApplicationProvider;
import java.util.concurrent.atomic.AtomicInteger;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

public class RendererSurfaceTest implements ImageReader.OnImageAvailableListener {
  Context context = ApplicationProvider.getApplicationContext();
  Renderer1 renderer = null;
  AtomicInteger counter = new AtomicInteger();

  @BeforeEach
  public void setup() {
    final EGLDisplay display = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
    final EGLContext shared = EGL14.eglGetCurrentContext();
    Assertions.assertEquals(EGL14.EGL_NO_CONTEXT, shared);
    renderer = new Renderer1(context, null, display, shared);
    Assertions.assertNotNull(renderer);
  }
  @AfterEach
  public void teardown() {
    if (renderer != null)
      renderer.close();
  }

  @Override
  public void onImageAvailable(ImageReader reader) {
    try (Image image = reader.acquireNextImage()) {
      final int format = image.getFormat();
      Assertions.assertNotNull(image.getHardwareBuffer());
      final int count = counter.incrementAndGet(); // load(acquire)
      Log.d("RendererSurfaceTest", String.format("format %x, count %d", format, count));
    } catch (UnsupportedOperationException ex) {
      Assertions.fail(ex.getMessage());
    }
  }

  @Test
  public void test_RGBA_8888() {
    ImageReader reader = ImageReader.newInstance(400, 400, PixelFormat.RGBA_8888, 3);
    reader.setOnImageAvailableListener(this, Handler.createAsync(context.getMainLooper()));
    Assertions.assertEquals(0, renderer.resume(reader.getSurface()));
    for (int i = 0; i < 4; i++) Assertions.assertEquals(0, renderer.present());
    Assertions.assertEquals(0, renderer.suspend());
  }
  @Test
  public void test_RGBX_8888() {
    ImageReader reader = ImageReader.newInstance(400, 400, PixelFormat.RGBX_8888, 3);
    reader.setOnImageAvailableListener(this, Handler.createAsync(context.getMainLooper()));
    Assertions.assertEquals(0, renderer.resume(reader.getSurface()));
    for (int i = 0; i < 4; i++) Assertions.assertEquals(0, renderer.present());
    Assertions.assertEquals(0, renderer.suspend());
  }
  // TODO: format mismatch(UnsupportedOperationException) when EGL swap buffer operation
  @Test
  public void test_RGB_888() {
    ImageReader reader = ImageReader.newInstance(400, 400, PixelFormat.RGB_888, 3);
    reader.setOnImageAvailableListener(this, Handler.createAsync(context.getMainLooper()));
    // this format is NOT supported. expect error code 95
    Assertions.assertEquals(95, renderer.resume(reader.getSurface()));
    // Assertions.assertEquals(0, renderer.resume(reader.getSurface()));
    // for (int i = 0; i < 4; i++) Assertions.assertEquals(0, renderer.present());
    // Assertions.assertEquals(0, renderer.suspend());
  }
  @Test
  public void test_RGB_565() {
    ImageReader reader = ImageReader.newInstance(400, 400, PixelFormat.RGB_565, 3);
    reader.setOnImageAvailableListener(this, Handler.createAsync(context.getMainLooper()));
    // this format is NOT supported. expect error code 95
    Assertions.assertEquals(95, renderer.resume(reader.getSurface()));
  }
}
