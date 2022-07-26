package dev.luncliff.muffin;

import android.Manifest;
import android.graphics.ImageFormat;
import android.media.Image;
import android.media.ImageReader;
import android.os.Environment;
import android.util.Log;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.jupiter.api.Assertions;
import org.junit.runner.RunWith;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeoutException;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.rule.GrantPermissionRule;

@RunWith(AndroidJUnit4.class)
public class SaveImageTest {
    @Rule
    public GrantPermissionRule permissions = GrantPermissionRule.grant(Manifest.permission.CAMERA,
            Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE);

    File workspace = null;
    File outFile = null;

    /**
     * Get the directory to save captured images
     */
    @Before
    public void setup() {
        File downloads = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        File folder = new File(downloads, "muffin-test");
        if (folder.exists() == false)
            folder.mkdirs();
        workspace = folder;
        outFile = new File(workspace, "image.ycc");
        if (outFile.exists())
            Assertions.assertTrue(outFile.delete());
    }

    static DeviceHandle getAnyCamera() {
        DeviceQuery.Init();
        Assertions.assertNotEquals(0, DeviceQuery.GetDeviceCount());
        for (DeviceHandle device : DeviceQuery.GetDevices()) {
            return device;
        }
        return null;
    }

    @Test
    public void sameImageToFile() {
        DeviceHandle camera = getAnyCamera();
        Assertions.assertNotNull(camera);

        // camera image reader
        try (ImageReader imageReader = ImageReader.newInstance(640, 480, ImageFormat.YUV_420_888, 2)) {
            // start capture operation
            camera.capture(imageReader.getSurface());
            ByteBuffer blob = null;
            // expect image in 2 sec
            try (Image image = TestHelper.WaitForImage(imageReader, 2)) {
                Assertions.assertNotNull(image);
                camera.stopCapture(); // stop after capture
                blob = TestHelper.ValidateAndGetBlob(image);
            }
            try (DataOutputStream stream = new DataOutputStream(new FileOutputStream(outFile))) {
                Log.i("SaveImageTest", outFile.getPath());
                stream.write(blob.array());
            }
        } catch (IOException | ExecutionException | InterruptedException | TimeoutException ex) {
            Assertions.fail(ex.getMessage());
        }
    }

}