package dev.luncliff.muffin;

public class Environment {
    /**
     * @apiNote The function loads required native libraries.
     *  Users don't have to invoke this manually.
     */
    public static void Init() throws UnsatisfiedLinkError {
        System.loadLibrary("c++_shared");
        System.loadLibrary("muffin");
    }

    /**
     * @apiNote Check if EGL extension is supported by current environment
     * @param extension For example, "EGL_ANDROID_framebuffer_target"
     * @see "https://github.com/KhronosGroup/EGL-Registry/tree/main/extensions"
     * @return True if supported
     */
    public static native boolean HasEGL(String extension);
}
