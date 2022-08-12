package dev.luncliff.muffin;

import org.junit.Before;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import java.time.Clock;
import java.time.Instant;
import java.time.LocalTime;

public class NativeTimerTest {
    @Before
    public void loadLibrary() {
        System.loadLibrary("muffin");
    }

    native int countWithInterval(int duration, int interval);

    @Test
    public void testRepeat1s() {
        LocalTime start = java.time.LocalTime.now();
        int count = countWithInterval(2000, 900);
        Assertions.assertEquals(2, count);
        LocalTime end = java.time.LocalTime.now();
        int elapsed = end.getSecond() - start.getSecond();
        Assertions.assertTrue(elapsed >= 2);
    }

    @Test
    public void testRepeat3s() {
        LocalTime start = java.time.LocalTime.now();
        int count = countWithInterval(3000, 400);
        Assertions.assertEquals(7, count);
        LocalTime end = java.time.LocalTime.now();
        int elapsed = end.getSecond() - start.getSecond();
        Assertions.assertTrue(elapsed >= 3);
    }
}
