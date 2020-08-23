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


/**
 * @author luncliff@gmail.com
 * @link https://developer.android.com/training/testing/unit-testing/local-unit-tests#java
 * @link https://www.baeldung.com/junit-5
 */
public class ContextFeatureTest {
    @Rule
    public GrantPermissionRule writeStorage =
            GrantPermissionRule.grant(Manifest.permission.WRITE_EXTERNAL_STORAGE);
    @Rule
    public GrantPermissionRule readStorage =
            GrantPermissionRule.grant(Manifest.permission.READ_EXTERNAL_STORAGE);

    @Test
    public void checkExecutor() {
        Context context = ApplicationProvider.getApplicationContext();
        Assertions.assertNotNull(context);
        Executor executor =  context.getMainExecutor();
        Assertions.assertNotNull(executor);
    }

    @Test
    public void checkMainLooper() {
        Context context = ApplicationProvider.getApplicationContext();
        Assertions.assertNotNull(context);
        Looper looper = context.getMainLooper();
        Assertions.assertNotNull(looper);
    }

    @Test
    public void checkAssetManager() {
        Context context = ApplicationProvider.getApplicationContext();
        Assertions.assertNotNull(context);
        AssetManager assets = context.getAssets();
        Assertions.assertNotNull(assets);
    }
}
