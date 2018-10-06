package muffin;

import android.Manifest;
import android.content.Context;
import android.content.res.AssetManager;
import android.support.test.InstrumentationRegistry;
import android.support.test.rule.GrantPermissionRule;
import android.support.test.runner.AndroidJUnit4;

import junit.framework.Assert;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static org.junit.Assert.assertNotNull;

/**
 * @author luncliff@gmail.com
 */
@RunWith(AndroidJUnit4.class)
public class TestBackbone {
    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("muffin");
    }

    @Rule
    public GrantPermissionRule useCamera =
            GrantPermissionRule.grant(Manifest.permission.CAMERA);
    @Rule
    public GrantPermissionRule writeStorage =
            GrantPermissionRule.grant(Manifest.permission.WRITE_EXTERNAL_STORAGE);
    @Rule
    public GrantPermissionRule readStorage =
            GrantPermissionRule.grant(Manifest.permission.READ_EXTERNAL_STORAGE);

    protected ExecutorService executor;
    protected Context context;
    protected AssetManager assets;

    @Before
    public void CreateExecutorService() {
        executor = Executors.newFixedThreadPool(2);
        Assert.assertNotNull(executor);
    }

    @Before
    public void GetContext() {
        context = InstrumentationRegistry.getTargetContext();
        assertNotNull(context);
        assets = context.getAssets();
    }

    @Test
    public void ReadyForTest() {
        assertNotNull(assets);
    }

    @After
    public void CloseAssets() {
        assets.close();
    }
}
