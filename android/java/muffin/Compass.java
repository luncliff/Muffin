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
    private static native void destroy(long impl);
    @Override
    public void close() throws RuntimeException{
        pause(impl);
        destroy(impl);
    }

    private static native int update(long impl);
    public void update() throws RuntimeException {
        update(impl); // @todo throw if error code
    }
    private static native int resume(long impl);
    public void resume() throws RuntimeException {
        resume(impl); // @todo throw if error code
    }
    private static native int pause(long impl);
    public void pause() throws RuntimeException {
        pause(impl); // @todo throw if error code
    }

    public native String getName();
}
