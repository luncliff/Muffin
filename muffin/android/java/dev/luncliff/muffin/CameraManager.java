package dev.luncliff.muffin;

public class CameraManager {
    static {
        Environment.Init();
    }

    private static CameraHandle[] devices = null;

    /**
     * @apiNote Multiple init is safe. Only *first* invocation will take effect
     */
    public static native void Init();

    /**
     * Get the number of current camera devices
     * 
     * @return 0 if initialization failed or no device confirmed
     */
    public static native int GetDeviceCount();

    /**
     * @param devices SetDeviceData will provide appropriate internal library id
     */
    private static native void SetDeviceData(CameraHandle[] devices);

    /**
     * @return array of available devices.
     * @see CameraHandle
     */
    public static synchronized CameraHandle[] GetDevices() {
        if (devices == null) // allocate java objects
        {
            int count = GetDeviceCount();
            devices = new CameraHandle[count];
            for (int i = 0; i < count; ++i)
                devices[i] = new CameraHandle();
            SetDeviceData(devices);
        }
        return devices;
    }
}
