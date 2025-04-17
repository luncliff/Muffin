package dev.luncliff.muffin

import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.ext.junit.runners.AndroidJUnit4

import org.junit.Test
import org.junit.runner.RunWith

import org.junit.Assert.*
import org.junit.Before

@RunWith(AndroidJUnit4::class)
class BridgeTest {
    @Before
    fun setup(){
        try {
            Bridge.Load()
        } catch (ex: UnsatisfiedLinkError){
            fail(ex.message)
        }
    }

    @Test
    fun testStringFromCpp() {
        assertEquals("Hello from C++", Bridge.stringFromJNI())
    }

    @Test
    fun testBuildVersion() {
        assertEquals("2025.4.0", Bridge.getBuildVersion())
    }

    @Test
    fun testPackageName() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        assertEquals("dev.luncliff.muffin.test", context.packageName)
    }

    @Test
    fun testPackageAssets() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        context.assets.apply {
            open("mock-file.txt").use {
                assertEquals(13, it.available())
            }
            open("test-mock-file.txt").use {
                assertEquals(18, it.available())
            }
        }
    }
}