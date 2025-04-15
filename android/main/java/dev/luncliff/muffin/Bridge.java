package dev.luncliff.muffin;

public class Bridge {
    public static void Load(){
        System.loadLibrary("muffin");
    }
    public static native String getBuildVersion();
    public static native String stringFromJNI();
}
