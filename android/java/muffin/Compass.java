package muffin;

import android.content.Context;
import android.hardware.SensorManager;
import android.os.Looper;
import android.util.Log;

/**
 * @author luncliff@gmail.com
 */
public class Compass implements AutoCloseable {
    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("muffin");
    }

    private long impl;

    public Compass(Context context) throws RuntimeException {
        impl = create(context.getPackageName());
    }

    private static native long create(String id) throws RuntimeException;

    @Override
    public native void close() throws RuntimeException;

    public native String getName();
}
