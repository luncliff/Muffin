plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
}

// todo https://docs.gradle.org/current/userguide/build_environment.html
ext {
    version = "2025.4.0" // see MUFFIN_VERSION
}

android {
    namespace = "dev.luncliff.muffin"
    compileSdk = 35
    buildToolsVersion = "35.0.1"
    ndkVersion = libs.versions.ndkVersion.get()

    defaultConfig {
        minSdk = 24
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++20"
                cppFlags += "-fno-rtti"
                arguments += ("-DMUFFIN_VERSION:STRING=${project.version}")
                arguments += "-DANDROID_STL=c++_shared"
            }
        }
        ndk {
            abiFilters.clear()
            abiFilters.add("arm64-v8a")
            abiFilters.add("x86_64")
        }
        sourceSets {
            getByName("main").setRoot("main")
            getByName("main") {
                assets.srcDir("assets")
            }
            getByName("androidTest").setRoot("androidTest")
            getByName("androidTest") {
                assets.srcDir("androidTestAssets")
            }
        }
    }

    buildTypes {
        debug {
            isJniDebuggable = true
        }
        release {
            isMinifyEnabled = false
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions {
        jvmTarget = "11"
    }
    packaging {
        // ... jniLibs
        resources {
            excludes += "META-INF/LICENSE*" // JUnit 5 will bundle in files
        }
    }
    externalNativeBuild {
        cmake {
            path = file("CMakeLists.txt")
            version = "3.21.0+"
        }
    }
    buildFeatures {
        viewBinding = true
    }
}

dependencies {
    // see libs.versions.toml
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}