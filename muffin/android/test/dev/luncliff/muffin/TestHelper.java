package dev.luncliff.muffin;

import android.content.Context;
import android.graphics.ImageFormat;
import android.media.Image;
import android.media.ImageReader;
import android.os.Environment;
import android.os.Handler;

import androidx.core.content.ContextCompat;
import androidx.test.core.app.ApplicationProvider;

import org.junit.jupiter.api.Assertions;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class TestHelper {
    static Executor foreground = ContextCompat.getMainExecutor(ApplicationProvider.getApplicationContext());
    static ExecutorService executorService = Executors.newFixedThreadPool(1);

    static Image WaitForImage(ImageReader reader, int sec)
            throws ExecutionException, InterruptedException, TimeoutException {
        Future<Image> f = executorService.submit(() -> {
            Image image = null;
            // try 50 times
            int repeatCount = 0;
            while (repeatCount < 50) {
                // Give some time to Camera Framework in background
                Thread.sleep(30);
                // acquire image...
                image = reader.acquireNextImage();
                if (image != null)
                    break; // acquired !
                repeatCount += 1;
            }
            return image;
        });
        return f.get(sec, TimeUnit.SECONDS);
    }

    static ByteBuffer ValidateAndGetBlob(Image image) {
        Assertions.assertEquals(ImageFormat.YUV_420_888, image.getFormat());
        Image.Plane[] planes = image.getPlanes();
        Assertions.assertEquals(3, planes.length);
        ByteBuffer y = planes[0].getBuffer();
        ByteBuffer cc1 = planes[1].getBuffer();
        ByteBuffer cc2 = planes[2].getBuffer();
        int planeSize = image.getWidth() * image.getHeight();
        int channel = planes.length;
        byte[] output = new byte[planeSize * channel / 2]; // allocate contiguous memory
        y.get(output, 0, planeSize);
        cc1.get(output, planeSize, 1);
        cc2.get(output, planeSize + 1, cc2.remaining());
        return ByteBuffer.wrap(output);
    }

    static void CheckStorageWritable() {
        String state = Environment.getExternalStorageState();
        Assertions.assertEquals(Environment.MEDIA_MOUNTED, state);
    }

}
