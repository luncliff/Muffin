package dev.luncliff.muffin;

import android.Manifest;
import android.content.Context;
import android.content.res.AssetManager;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.rule.GrantPermissionRule;

import org.junit.Rule;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.io.InputStream;

public class TensorFlowLiteTest {
  @Rule
  public GrantPermissionRule permissions = GrantPermissionRule.grant(Manifest.permission.READ_EXTERNAL_STORAGE);

  Context context = ApplicationProvider.getApplicationContext();
  AssetManager assets = context.getAssets();

  void checkExists(String name, int size) {
    try (InputStream stream = assets.open(name)) {
      if (size == 0)
        Assertions.assertNotEquals(0, stream.available());
      else
        Assertions.assertEquals(size, stream.available());
    } catch (IOException ex) {
      Assertions.fail(ex.getMessage());
    }
  }

  @Test
  public void checkModels() {
    checkExists("face_detection_short_range.tflite", 229032);
    checkExists("face_landmark.tflite", 1241896);
    checkExists("iris_landmark.tflite", 2640568);
    checkExists("selfie_segmentation_landscape.tflite", 249792);
  }

}
