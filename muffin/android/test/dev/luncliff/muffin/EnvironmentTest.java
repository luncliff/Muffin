package dev.luncliff.muffin;

import static org.junit.jupiter.api.DynamicTest.dynamicTest;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.DynamicTest;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestFactory;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

public class EnvironmentTest {
    /**
     * @see "https://github.com/KhronosGroup/EGL-Registry"
     */
    @TestFactory
    Collection<DynamicTest> EGL() {
        ArrayList<DynamicTest> tests = new ArrayList<>();
        tests.add(dynamicTest("KHR", EnvironmentTest::KHR));
        if(Environment.HasEGL("EGL_ANDROID_create_native_client_buffer"))
            tests.add(dynamicTest("ANDROID", EnvironmentTest::ANDROID_Full));
        else
            tests.add(dynamicTest("ANDROID", EnvironmentTest::ANDROID_Lite));
        return tests;
    }

    public static void KHR() {
        Assertions.assertTrue(Environment.HasEGL("EGL_KHR_image_base"));
        Assertions.assertTrue(Environment.HasEGL("EGL_KHR_fence_sync"));
        Assertions.assertTrue(Environment.HasEGL("EGL_KHR_wait_sync"));
    }
    public static void ANDROID_Full() {
        Assertions.assertTrue(Environment.HasEGL("EGL_ANDROID_image_native_buffer"));
        ANDROID_Lite();
    }
    public static void ANDROID_Lite() {
        Assertions.assertTrue(Environment.HasEGL("EGL_ANDROID_presentation_time"));
        Assertions.assertTrue(Environment.HasEGL("EGL_ANDROID_get_native_client_buffer"));
        Assertions.assertTrue(Environment.HasEGL("EGL_ANDROID_get_frame_timestamps"));
    }

}
