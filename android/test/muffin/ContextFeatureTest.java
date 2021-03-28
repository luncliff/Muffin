package muffin;

import org.junit.Before;
import org.junit.Rule;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import android.Manifest;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Looper;

import android.util.Log;

import androidx.test.core.app.ApplicationProvider;
import androidx.test.rule.GrantPermissionRule;

import java.io.File;
import java.util.concurrent.Executor;

import static android.content.pm.PackageManager.GET_SHARED_LIBRARY_FILES;


/**
 * @author luncliff@gmail.com
 * @link https://developer.android.com/training/testing/unit-testing/local-unit-tests#java
 * @link https://www.baeldung.com/junit-5
 */
public class ContextFeatureTest {
    @Rule
    public GrantPermissionRule permissions =
            GrantPermissionRule.grant(Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.READ_EXTERNAL_STORAGE);

    Context context = ApplicationProvider.getApplicationContext();
    @Before
    public void setup(){
        Assertions.assertNotNull(context);
    }

    /**
     * @implNote The info is from AndroidManifest.xml
     * @see "https://stackoverflow.com/a/13790076"
     */
    @Test
    public void checkPackageInfo() throws PackageManager.NameNotFoundException {
        final PackageManager pm = context.getPackageManager();
        final PackageInfo info = pm.getPackageInfo(context.getPackageName(), 0);
        Assertions.assertNotNull(info);
        Assertions.assertNull(info.versionName);
    }

    @Test
    public void checkApplicationInfo() throws PackageManager.NameNotFoundException {
        final PackageManager pm = context.getPackageManager();
        final ApplicationInfo info = pm.getApplicationInfo(context.getPackageName(), GET_SHARED_LIBRARY_FILES);
        Assertions.assertNotNull(info);
        Assertions.assertNotNull(info.sharedLibraryFiles);
        Assertions.assertNotEquals(0, info.sharedLibraryFiles.length);
        for (final String filename: info.sharedLibraryFiles) {
            final File file = new File(filename);
            Assertions.assertTrue(file.exists());
            Log.i("ContextFeatureTest", String.format("shared lib: %s", file.getAbsolutePath()));
        }
    }

    @Test
    public void checkExecutor() {
        Executor executor =  context.getMainExecutor();
        Assertions.assertNotNull(executor);
    }

    @Test
    public void checkMainLooper() {
        Looper looper = context.getMainLooper();
        Assertions.assertNotNull(looper);
    }

    @Test
    public void checkAssetManager() {
        AssetManager assets = context.getAssets();
        Assertions.assertNotNull(assets);
    }
}
