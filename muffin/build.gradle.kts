import org.gradle.api.tasks.testing.logging.TestExceptionFormat
import org.gradle.api.tasks.testing.logging.TestLogEvent
import java.net.URI

// https://developer.android.com/studio/build/native-dependencies?hl=en
plugins {
    id("com.android.library") // version "7.2.2"
    id("org.jetbrains.kotlin.android")
    id("kotlin-android")
    id("maven-publish")
}
repositories {
    google()
    mavenCentral()
    // sonatypeSnapshots()
}

/**
 * @brief Set of variables for the build configuration
 */
object BuildParams {
    const val projectName = "muffin"
    const val githubUrl = "https://github.com/luncliff/muffin"
    val vcpkgToolchainFile = System.getenv("VCPKG_ROOT") + "/scripts/buildsystems/vcpkg.cmake"
    val ndkToolchainFile = System.getenv("ANDROID_NDK_HOME") + "/build/cmake/android.toolchain.cmake"
}

android {
    compileSdk = 32
    // buildToolsVersion = "30.0.2"
    ndkVersion = "24.0.8215888"

    buildTypes {
        getByName("debug") {
            isJniDebuggable = true
            isTestCoverageEnabled = true
            isMinifyEnabled = false
        }
        getByName("release") {
            isJniDebuggable = true
            isMinifyEnabled = true
        }
    }
    buildFeatures {
        buildConfig = true
        prefab = true
        // prefabPublishing = true
    }
    // prefab {
    //     muffin.headers "src"
    // }

    defaultConfig {
        minSdk = 26
        targetSdk = 32
        ndk {
            abiFilters.clear()
            abiFilters.add("arm64-v8a")
            abiFilters.add("armeabi-v7a")
            abiFilters.add("x86_64")
        }
        buildConfigField("String", "VERSION_NAME", "\"1.3.0\"")
        externalNativeBuild {
            cmake {
                cppFlags += "-fno-rtti"
                arguments += "-DANDROID_STL=c++_shared"
                arguments += "-DANDROID_ARM_NEON=ON"
                arguments += ("-DCMAKE_TOOLCHAIN_FILE:FILEPATH=" + BuildParams.vcpkgToolchainFile)
                arguments += ("-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE:FILEPATH=" + BuildParams.ndkToolchainFile)
                targets(BuildParams.projectName)
            }
        }
        // https://github.com/mannodermaus/android-junit5
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        testInstrumentationRunnerArguments["runnerBuilder"] = "de.mannodermaus.junit5.AndroidJUnit5Builder"
    }
    externalNativeBuild {
        cmake {
            version = "3.21.3+"
            path = file("CMakeLists.txt")
        }
    }

    sourceSets {
        getByName("main"){
            java.srcDir("android/java")
            manifest.srcFile("android/AndroidManifest.xml")
            assets.srcDir("android/assets")
        }
        getByName("androidTest"){
            java.srcDir("android/test")
            assets.srcDir("android/testAssets")
        }
    }

    packagingOptions {
        jniLibs {
            // use binaries in the other AAR
            excludes += "lib/**/libtensorflowlite_jni.so"
        }
        resources {
            excludes += "META-INF/LICENSE*" // JUnit 5 will bundle in files
        }
    }

    testOptions {
        unitTests.isReturnDefaultValues = true
    }
}

tasks.withType<Test> {
  failFast = true
  testLogging {
    events = setOf(TestLogEvent.PASSED, TestLogEvent.SKIPPED, TestLogEvent.FAILED)
    exceptionFormat = TestExceptionFormat.FULL
    events = setOf(
        TestLogEvent.PASSED,
        TestLogEvent.SKIPPED,
        TestLogEvent.FAILED,
        TestLogEvent.STANDARD_ERROR
    )
    showStandardStreams = true
    showStackTraces = true
    showCauses = true
    showExceptions = true
  }
}

dependencies {
    // https://developers.google.com/ar/develop/java/quickstart
    // api("com.google.ar:core:1.32.0")
    // api("com.google.guava:guava:31.1-jre")
    api("org.jetbrains.kotlin:kotlin-stdlib:1.7.10")

    // https://search.maven.org/artifact/org.tensorflow/tensorflow-lite
    // https://www.tensorflow.org/lite/performance/gpu_advanced
    // api("org.tensorflow:tensorflow-lite:2.9.0")
    // implementation("org.tensorflow:tensorflow-lite-gpu:2.9.0")
    androidTestImplementation("org.tensorflow:tensorflow-lite:2.9.0")

    api("androidx.core:core-ktx:1.8.0")
    api("androidx.camera:camera-core:1.1.0")
    api("androidx.camera:camera-camera2:1.1.0")
    api("androidx.camera:camera-lifecycle:1.1.0")

    // https://developer.android.com/training/testing/set-up-project#android-test-dependencies
    androidTestImplementation("androidx.test:core:1.4.0")
    androidTestImplementation("androidx.test:rules:1.4.0")
    androidTestImplementation("androidx.test:runner:1.4.0")
    androidTestImplementation("androidx.test.ext:junit:1.1.3")

    androidTestImplementation("org.junit.jupiter:junit-jupiter-api:5.7.1")
    androidTestImplementation("org.junit.jupiter:junit-jupiter-params:5.7.1")
    androidTestRuntimeOnly("org.junit.jupiter:junit-jupiter-engine:5.7.1")

    androidTestImplementation("de.mannodermaus.junit5:android-test-core:1.2.2")
    androidTestRuntimeOnly("de.mannodermaus.junit5:android-test-runner:1.2.2")
}


afterEvaluate {
    publishing {
        repositories {
            maven {
                url = URI("https://.../repository/nightly")
                credentials {
                    username = System.getenv("USERNAME")
                    password = System.getenv("PASSWORD")
                }
            }
        }
        publications {
            create<MavenPublication>("debug") {
                from(components.getByName("debug"))
                groupId = "org.dev.luncliff"
                artifactId = "sample"
                version = "1.0-SNAPSHOT"
            }
            create<MavenPublication>("release") {
                from(components.getByName("release"))
                groupId = "org.dev.luncliff"
                artifactId = "sample-debug"
                version = "1.0-SNAPSHOT"
            }
        }
    }
}
