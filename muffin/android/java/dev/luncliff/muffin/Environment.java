package dev.luncliff.muffin;

public class Environment {
    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("muffin");
    }

    public static native boolean HasEGLExtension(String name);
}
