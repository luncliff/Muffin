package dev.luncliff.muffin;

public class DeviceQuery {
    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("muffin");
    }

    private static DeviceHandle[] devices = null;

    /**
     * Multiple init is safe. Only *first* invocation will take effect
     */
    public static synchronized native void Init();

    /**
     * Get the number of current camera devices
     * 
     * @return 0 if initialization failed or no device confirmed
     */
    public static native int GetDeviceCount();

    /**
     * @param devices SetDeviceData will provide appropriate internal library id
     */
    private static native void SetDeviceData(DeviceHandle[] devices);

    /**
     * @return array of available devices.
     * @see DeviceHandle
     */
    public static synchronized DeviceHandle[] GetDevices() {
        if (devices == null) // allocate java objects
        {
            int count = GetDeviceCount();

            devices = new DeviceHandle[count];
            for (int i = 0; i < count; ++i)
                devices[i] = new DeviceHandle();

            SetDeviceData(devices);
        }
        return devices;
    }
}
