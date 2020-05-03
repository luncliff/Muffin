package muffin;

import org.junit.Before;
import org.junit.Rule;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import android.Manifest;
import android.content.Context;
import android.content.res.AssetManager;
import android.os.Looper;

import android.content.Context;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.rule.GrantPermissionRule;

import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class CompassTest {

    @Test
    public void checkPackageName() {
        Context context = ApplicationProvider.getApplicationContext();
        Assertions.assertNotNull(context);

        Assertions.assertEquals("dev.muffin.test", context.getPackageName());
    }

    @Test
    public void checkMainLooper() {
        Context context = ApplicationProvider.getApplicationContext();
        Assertions.assertNotNull(context);

        Looper looper = context.getMainLooper();
        Assertions.assertNotNull(looper);
    }

    @Test
    public void checkCompassAutoClose() {
        Context context = ApplicationProvider.getApplicationContext();
        Assertions.assertNotNull(context);

        try(Compass compass = new Compass(context)){
            // must guarantee non empty string for its name
            Assertions.assertNotEquals("", compass.getName());
        }
    }
}
