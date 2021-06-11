package muffin;

import android.Manifest;
import android.content.Context;
import android.graphics.ImageFormat;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Process;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import androidx.annotation.NonNull;
import androidx.camera.core.Camera;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageInfo;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.core.SurfaceRequest;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.core.util.Consumer;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleOwner;
import androidx.lifecycle.LifecycleRegistry;
import androidx.test.annotation.UiThreadTest;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.rule.GrantPermissionRule;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicInteger;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.jupiter.api.Assertions;
import org.junit.runner.RunWith;

/**
 * @see "https://source.android.com/devices/camera/stream-config"
 */
@RunWith(AndroidJUnit4.class)
public class CameraInputTest
    implements LifecycleOwner, Preview.SurfaceProvider, ImageReader.OnImageAvailableListener,
               ImageAnalysis.Analyzer, Consumer<SurfaceRequest.Result>,
               SurfaceRequest.TransformationInfoListener {
  @Rule
  public GrantPermissionRule permissions = GrantPermissionRule.grant(Manifest.permission.CAMERA);

  final Context context = ApplicationProvider.getApplicationContext();
  final ExecutorService executor = Executors.newSingleThreadExecutor();
  HandlerThread thread = new HandlerThread("CameraInputTest-0", Process.THREAD_PRIORITY_BACKGROUND);

  LifecycleRegistry registry;
  ProcessCameraProvider provider;

  ImageReader reader0;
  Preview.Builder preview0;
  AtomicInteger counter0 = new AtomicInteger();

  ImageAnalysis analysis1;
  AtomicInteger counter1 = new AtomicInteger();

  @Before
  public void setup0() throws ExecutionException, InterruptedException {
    Future<ProcessCameraProvider> future = ProcessCameraProvider.getInstance(context);
    provider = future.get();
    Assertions.assertNotNull(provider);
  }
  @Before
  public void setup1() {
    registry = new LifecycleRegistry(this);
    registry.markState(Lifecycle.State.INITIALIZED);
    registry.markState(Lifecycle.State.CREATED);
  }
  @Before
  public void setup2() {
    thread.start();
    reader0 = ImageReader.newInstance(1280, 720, ImageFormat.YUV_420_888, 10);
    reader0.setOnImageAvailableListener(this, Handler.createAsync(thread.getLooper()));
    {
      preview0 = new Preview.Builder();
      preview0.setTargetResolution(new Size(reader0.getWidth(), reader0.getHeight()));
    }
    {
      ImageAnalysis.Builder builder = new ImageAnalysis.Builder();
      builder.setTargetResolution(new Size(640, 480));
      builder.setBackpressureStrategy(ImageAnalysis.STRATEGY_BLOCK_PRODUCER);
      builder.setTargetRotation(0);
      analysis1 = builder.build();
      analysis1.setAnalyzer(executor, this);
    }
  }
  @After
  public void teardown1() {
    if (registry != null)
      registry.markState(Lifecycle.State.DESTROYED);
  }
  @After
  public void teardown2() {
    thread.quit();
    executor.shutdown();
    if (reader0 != null)
      reader0.close();
  }

  // from CameraX Preview (Surface -> ImageReader)
  @Override
  public void onImageAvailable(ImageReader reader) {
    // Assertions.assertEquals(reader, reader0);
    try (Image image = reader.acquireNextImage()) {
      final int format = image.getFormat();
      Log.d("CameraInputTest", String.format("image: format %d", format));
      Assertions.assertEquals(ImageFormat.YUV_420_888, format);
      counter0.incrementAndGet();
    }
  }
  // from CameraX ImageAnalysis
  @Override
  public void analyze(@NonNull ImageProxy proxy) {
    try (ImageProxy image = proxy) {
      final int format = image.getFormat();
      final ImageInfo info = image.getImageInfo();
      final int degree = info.getRotationDegrees();
      Log.d("CameraInputTest", String.format("image: format %d degree %d", format, degree));
      Assertions.assertEquals(ImageFormat.YUV_420_888, format);
      counter1.incrementAndGet();
    }
  }

  @NonNull
  @Override
  public Lifecycle getLifecycle() {
    return registry;
  }

  @Override
  public void accept(SurfaceRequest.Result result) {
    final String message = result.toString();
    switch (result.getResultCode()) {
      case SurfaceRequest.Result.RESULT_SURFACE_USED_SUCCESSFULLY:
        Log.i("CameraInputTest", message);
        return;
      case SurfaceRequest.Result.RESULT_REQUEST_CANCELLED:
      case SurfaceRequest.Result.RESULT_INVALID_SURFACE:
      case SurfaceRequest.Result.RESULT_SURFACE_ALREADY_PROVIDED:
      case SurfaceRequest.Result.RESULT_WILL_NOT_PROVIDE_SURFACE:
        Log.e("CameraInputTest", message);
        Assertions.fail(message);
    }
  }

  @Override
  public void onSurfaceRequested(@NonNull SurfaceRequest request) {
    Surface surface = reader0.getSurface();
    Log.i("CameraInputTest", String.format("providing: %s", surface));
    if (executor.isShutdown()) {
      request.willNotProvideSurface();
      return;
    }
    request.provideSurface(surface, executor, this);
    request.setTransformationInfoListener(executor, this);
  }
  @Override
  public void onTransformationInfoUpdate(@NonNull SurfaceRequest.TransformationInfo info) {
    Log.d("CameraInputTest", String.format("rotation: %d", info.getRotationDegrees()));
  }

  @UiThreadTest
  @Test
  public void test_Front_90() throws InterruptedException {
    registry.markState(Lifecycle.State.STARTED);
    provider.unbindAll();

    preview0.setTargetRotation(Surface.ROTATION_90);
    Preview preview = preview0.build();
    Camera device =
        provider.bindToLifecycle(this, CameraSelector.DEFAULT_FRONT_CAMERA, analysis1, preview);
    Assertions.assertNotNull(device);
    preview.setSurfaceProvider(executor, this);
    Thread.sleep(3000);
    provider.unbindAll();
    Assertions.assertFalse(executor.isTerminated());
    Assertions.assertTrue(thread.isAlive());
    if (counter0.get() == 0)
      Log.w("CameraInputTest", "preview count is 0");
    if (counter1.get() == 0)
      Log.w("CameraInputTest", "analysis count is 0");
  }

  @UiThreadTest
  @Test
  public void test_Back_270() throws InterruptedException {
    registry.markState(Lifecycle.State.STARTED);
    provider.unbindAll();

    preview0.setTargetRotation(Surface.ROTATION_270);
    Preview preview = preview0.build();
    Camera device =
        provider.bindToLifecycle(this, CameraSelector.DEFAULT_BACK_CAMERA, analysis1, preview);
    Assertions.assertNotNull(device);
    preview.setSurfaceProvider(executor, this);
    Thread.sleep(3000);
    provider.unbindAll();
    Assertions.assertFalse(executor.isTerminated());
    Assertions.assertTrue(thread.isAlive());
    if (counter0.get() == 0)
      Log.w("CameraInputTest", "preview count is 0");
    if (counter1.get() == 0)
      Log.w("CameraInputTest", "analysis count is 0");
  }
}
