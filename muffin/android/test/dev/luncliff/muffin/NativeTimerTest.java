package dev.luncliff.muffin;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;

import java.time.LocalTime;

public class NativeTimerTest {
    @BeforeAll
    public static void setupAll() {
        Environment.Init();
    }

    native int countWithInterval(int duration, int interval);

    @Test
    public void testRepeat1s() {
        LocalTime start = java.time.LocalTime.now();
        int count = countWithInterval(2000, 900);
        Assertions.assertEquals(2 + 1, count);
        LocalTime end = java.time.LocalTime.now();
        int elapsed = end.getSecond() - start.getSecond();
        Assertions.assertTrue(elapsed >= 2);
    }

    @Test
    public void testRepeat3s() {
        LocalTime start = java.time.LocalTime.now();
        int count = countWithInterval(3000, 400);
        Assertions.assertEquals(7 + 1, count);
        LocalTime end = java.time.LocalTime.now();
        int elapsed = end.getSecond() - start.getSecond();
        Assertions.assertTrue(elapsed >= 3);
    }
}
