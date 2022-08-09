package dev.luncliff.sample;

import android.app.Application;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Log;

import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import dev.luncliff.muffin.DeviceManager;

public class MuffinApp extends Application {
    private final HandlerThread thread0 = new HandlerThread("muffin-thread-0");
    private Handler handler0;

    private void setupHandler(){
        thread0.start();
        Looper looper = thread0.getLooper();
        if (android.os.Build.VERSION.SDK_INT >= 28)
            handler0 = Handler.createAsync(looper);
        else
            handler0 = new Handler(looper);
    }
    Handler getBackgroundHandler() {
        return handler0;
    }

    private final ExecutorService background0 = Executors.newFixedThreadPool(2);
    Executor getBackgroundExecutor(){
        return background0;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        setupHandler();
        DeviceManager.Init();
        Log.i("App", String.format("Camera Count: %d", DeviceManager.GetDeviceCount()));
    }

    @Override
    public void onTerminate() {
        thread0.quit();
        background0.shutdown();
        super.onTerminate();
    }

}
