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
        tests.add(dynamicTest("KHR", EnvironmentTest::KHR_image_base));
        if(Environment.HasEGLExtension("EGL_ANDROID_create_native_client_buffer"))
            tests.add(dynamicTest("ANDROID", EnvironmentTest::ANDROID_create_native_client_buffer));
        return tests;
    }

    public static void KHR_image_base() {
        Assertions.assertTrue(Environment.HasEGLExtension("EGL_KHR_image_base"));
    }
    public static void ANDROID_create_native_client_buffer() {
        Assertions.assertTrue(Environment.HasEGLExtension("EGL_ANDROID_image_native_buffer"));
        Assertions.assertTrue(Environment.HasEGLExtension("EGL_KHR_image_base"));
    }
}
