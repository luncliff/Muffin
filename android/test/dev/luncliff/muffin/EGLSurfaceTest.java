package dev.luncliff.muffin;

import android.content.Context;
import android.graphics.PixelFormat;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import androidx.test.core.app.ApplicationProvider;
import java.util.concurrent.atomic.AtomicInteger;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;

public class EGLSurfaceTest implements ImageReader.OnImageAvailableListener {
  static Handler handler;

  @BeforeAll
  static void makeHandler() {
    Context context = ApplicationProvider.getApplicationContext();
    Looper looper = context.getMainLooper();
    if (android.os.Build.VERSION.SDK_INT >= 28)
      handler = Handler.createAsync(looper);
    else
      handler = new Handler(looper);
    Assertions.assertNotNull(handler);
  }

  AtomicInteger counter = new AtomicInteger();
  EGLSurfaceOwner surface0;
  EGLContextOwner context0;

  @AfterEach
  public void teardown() {
    if (context0 != null)
      context0.close();
    if (surface0 != null)
      surface0.close();
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
    try (ImageReader reader = ImageReader.newInstance(400, 400, PixelFormat.RGBA_8888, 3)) {
      reader.setOnImageAvailableListener(this, handler);
      surface0 = new EGLSurfaceOwner(reader.getSurface());
      Assertions.assertNotEquals(0, surface0.config);
      context0 = new EGLContextOwner(surface0);
      Assertions.assertEquals(0, context0.resume(surface0));
      for (int i = 0; i < 4; ++i)
        Assertions.assertEquals(0, context0.present());
      Assertions.assertEquals(0, context0.suspend());
    } catch (RuntimeException ex) {
      Assertions.fail(ex.getMessage());
    }
  }

  @Test
  public void test_RGBX_8888() {
    try (ImageReader reader = ImageReader.newInstance(400, 400, PixelFormat.RGBX_8888, 3)) {
      reader.setOnImageAvailableListener(this, handler);
      surface0 = new EGLSurfaceOwner(reader.getSurface());
      Assertions.assertNotEquals(0, surface0.config);
      context0 = new EGLContextOwner(surface0);
      Assertions.assertEquals(0, context0.resume(surface0));
      for (int i = 0; i < 4; ++i)
        Assertions.assertEquals(0, context0.present());
      Assertions.assertEquals(0, context0.suspend());
    } catch (RuntimeException ex) {
      Assertions.fail(ex.getMessage());
    }
  }

  /**
   * @implNote Format mismatch(UnsupportedOperationException) error when EGL swap
   *           buffer operation
   */
  @Test
  public void test_RGB_888() {
    try (ImageReader reader = ImageReader.newInstance(400, 400, PixelFormat.RGB_888, 3)) {
      reader.setOnImageAvailableListener(this, handler);
      surface0 = new EGLSurfaceOwner(reader.getSurface());
    } catch (RuntimeException ex) {
      // we expect error code 95 (ENOTSUP)
      Log.i("EGLSurfaceTest", ex.getMessage());
    }
  }

  @Test
  public void test_RGB_565() {
    try (ImageReader reader = ImageReader.newInstance(400, 400, PixelFormat.RGB_565, 3)) {
      reader.setOnImageAvailableListener(this, handler);
      surface0 = new EGLSurfaceOwner(reader.getSurface());
    } catch (RuntimeException ex) {
      // we expect error code 95 (ENOTSUP)
      Log.i("EGLSurfaceTest", ex.getMessage());
    }
  }
}
