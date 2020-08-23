package muffin;

import android.content.Context;

import androidx.test.core.app.ApplicationProvider;

import org.junit.Before;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import java.util.concurrent.Executor;

public class NativeLoadTest {
    @Before
    public void stl(){
        System.loadLibrary("c++_shared");
    }
    @Test
    public void png() {
        System.loadLibrary("png");
    }
    @Test
    public void turbojpeg() {
        System.loadLibrary("jpeg");
        System.loadLibrary("turbojpeg");
    }
    @Test
    public void spdlog() {
        System.loadLibrary("fmt");
        System.loadLibrary("spdlog");
    }
    @Test
    public void muffin() {
        System.loadLibrary("muffin");
    }
}
