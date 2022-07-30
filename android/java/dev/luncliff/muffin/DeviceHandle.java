package dev.luncliff.muffin;

import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.os.Handler;
import android.view.Surface;

import androidx.annotation.NonNull;

/**
 * In short, this is combination of
 * {@link android.hardware.camera2.CameraDevice} and
 * {@link android.hardware.camera2.CameraCaptureSession}
 *
 * @todo capture request with configuration
 * @todo multiple surface request
 */
public class DeviceHandle {
    private long ptr = 0;

    /**
     * @return {@link CameraCharacteristics#LENS_FACING_FRONT } ||
     *         {@link CameraCharacteristics#LENS_FACING_BACK } ||
     *         {@link CameraCharacteristics#LENS_FACING_EXTERNAL }
     */
    public native int facing();

    /**
     * Open a camera device
     * 
     * @see android.hardware.camera2.CameraManager#openCamera(String,
     *      CameraDevice.StateCallback, Handler)
     */
    private native void open();

    /**
     * Close both internal session and device handle so that other device can be
     * opened This method is named to support {@link AutoCloseable} in future update
     *
     * @see CameraCaptureSession#close()
     * @see DeviceHandle#close()
     */
    public native void close();

    /**
     * User of the Camera 2 API must provide valid Surface.
     *
     * @see "https://developer.android.com/reference/android/view/Surface"
     * @param surface output surface for Camera 2 API
     */
    public void repeat(Surface surface) {
        open();
        startRepeat(surface);
    }

    /**
     * Open a camera and create a session with repeating capture request
     * 
     * @param surface camera will provide image to given surface
     */
    private native void startRepeat(Surface surface);

    /**
     * Stop the repeating request
     * 
     * @see DeviceHandle#startRepeat(Surface)
     */
    public native void stopRepeat();

    /**
     * User of the Camera 2 API must provide valid Surface.
     *
     * @see "https://developer.android.com/reference/android/view/Surface"
     * @param surface output surface for Camera 2 API
     */
    public void capture(Surface surface) {
        open();
        startCapture(surface);
    }

    /**
     * Open a camera and create a session with capture request
     * 
     * @param surface camera will provide image to given surface
     */
    private native void startCapture(Surface surface);

    /**
     * Stop(abort) the capture request
     * 
     * @see DeviceHandle#startCapture(Surface)
     */
    public native void stopCapture();

    @NonNull
    @Override
    public String toString() {
        return String.format("DeviceHandle{%x}", ptr);
    }
}