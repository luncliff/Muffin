package dev.luncliff.muffin;

import android.Manifest;
import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

import androidx.test.core.app.ApplicationProvider;
import androidx.test.rule.GrantPermissionRule;

import org.junit.Rule;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

public class SensorThreadTest {
    // @Rule public GrantPermissionRule permissions = GrantPermissionRule.grant(Manifest.permission.HIGH_SAMPLING_RATE_SENSORS);
    HandlerThread thread0 = new HandlerThread("sensor-test-thread");
    Looper looper = null;
    Handler handler = null;

    @BeforeAll
    public static void LoadLibrary() {
        Environment.Init();
    }

    @BeforeEach
    public void setup() {
        thread0.start();
        looper = thread0.getLooper();
        if (android.os.Build.VERSION.SDK_INT >= 28)
            handler = Handler.createAsync(looper);
        else
            handler = new Handler(looper);
        Assertions.assertNotNull(handler);
    }

    @AfterEach
    public void teardown() {
        thread0.quitSafely();
    }

    native void test0(String name, Looper looper);

    @Test
    public void testCreateInHandlerThread() {
        Context context = ApplicationProvider.getApplicationContext();
        String name = context.getPackageName();
        Assertions.assertNotNull(name);
        try {
            Assertions.assertTrue(handler.post(() -> {
                test0(name, looper);
            }));
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            Assertions.fail(e.getMessage());
        }
    }

}
