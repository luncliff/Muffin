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
    public void checkCompassHasName() {
        Context context = ApplicationProvider.getApplicationContext();
        Assertions.assertNotNull(context);
        try(Compass compass = new Compass(context)){
            // must guarantee non empty string for its name
            Assertions.assertNotEquals("", compass.getName());
        }
    }

    @Test
    public void tryMultiplePause() {
        try(Compass compass = new Compass(ApplicationProvider.getApplicationContext())){
            compass.pause();
            compass.pause();
        }
    }
    @Test
    public void tryUpdateWhenPaused() {
        try(Compass compass = new Compass(ApplicationProvider.getApplicationContext())){
            compass.pause();
            compass.update();
        }
    }

    @Test
    public void tryMultipleResume() {
        try(Compass compass = new Compass(ApplicationProvider.getApplicationContext())){
            compass.resume();
            compass.resume();
        }
    }
    @Test
    public void tryUpdateWhenResumed() throws Exception {
        try(Compass compass = new Compass(ApplicationProvider.getApplicationContext())){
            compass.resume();
            Thread.sleep(30); // give some time to sendsors
            compass.update();
        }
    }

}
