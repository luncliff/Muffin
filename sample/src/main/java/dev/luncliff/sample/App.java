package dev.luncliff.sample;

import android.app.Application;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

public class App extends Application {
    private final HandlerThread thread0 = new HandlerThread("CameraThread");
    private Handler handler0;

    @Override
    public void onCreate() {
        super.onCreate();
        thread0.start();
        Looper looper = thread0.getLooper();
        if (android.os.Build.VERSION.SDK_INT >= 28)
            handler0 = Handler.createAsync(looper);
        else
            handler0 = new Handler(looper);
    }

    @Override
    public void onTerminate() {
        thread0.quit();
        super.onTerminate();
    }

    Handler getCameraHandler() {
        return handler0;
    }
}
