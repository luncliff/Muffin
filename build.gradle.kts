buildscript {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
        // jitpack()
    }
    dependencies {
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:1.7.10")

        // https://developer.android.com/studio/releases/gradle-plugin
        // see https://github.com/android/ndk/issues/1609
        //
        // Linux environment can't find the ninja in android_gradle_build.json
        // If SUPPORT_PREFAB is not defined, Downgrade the AGP version and build without Prefab...
        //
        classpath("com.android.tools.build:gradle:7.2.2")
        // https://github.com/mannodermaus/android-junit5
        classpath("de.mannodermaus.gradle.plugins:android-junit5:1.8.2.1")
    }
}
