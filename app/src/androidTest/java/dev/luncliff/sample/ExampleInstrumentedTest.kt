package dev.luncliff.sample

import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.ext.junit.runners.AndroidJUnit4
import dev.luncliff.muffin.Bridge

import org.junit.Test
import org.junit.runner.RunWith

import org.junit.Assert.*

/**
 * Instrumented test, which will execute on an Android device.
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
@RunWith(AndroidJUnit4::class)
class ExampleInstrumentedTest {
    @Test
    fun testBridgeBuildVersion() {
        Bridge.Load()
        assertEquals("2025.4.0", Bridge.getBuildVersion())
    }
    @Test
    fun testBridgePackageAssets() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        assertEquals("dev.luncliff.sample", context.packageName)
        context.assets.apply {
            open("mock-file.txt").use {
                assertEquals(13, it.available())
            }
        }
    }
}