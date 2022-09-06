# Muffin

[![Build Status](https://dev.azure.com/luncliff/personal/_apis/build/status/luncliff.Muffin?branchName=main)](https://dev.azure.com/luncliff/personal/_build/latest?definitionId=34&branchName=main)
[![CircleCI](https://dl.circleci.com/status-badge/img/gh/luncliff/Muffin/tree/main.svg?style=shield)](https://dl.circleci.com/status-badge/redirect/gh/luncliff/Muffin/tree/main)
![Gradle](https://img.shields.io/badge/Gradle-7.2+-02303a)
![Android NDK](https://img.shields.io/badge/NDK-22+-3ecf7d)
![CMake](https://img.shields.io/badge/CMake-3.21+-00529b)

Template for Android NDK module

### References

* [NDK Samples(GitHub)](https://github.com/android/ndk-samples)
* [Android Developer Guide](https://developer.android.com/guide)
  * [Testing](https://developer.android.com/training/testing/unit-testing)
* [Testing with JUnit 5 for Android](https://github.com/mannodermaus/android-junit5)
* https://developer.android.com/studio/projects/gradle-external-native-builds
  * https://developer.android.com/studio/build/native-dependencies?hl=en
  * https://google.github.io/prefab/

#### Android 

* https://developer.android.com/ndk/reference/group/a-hardware-buffer
* https://developer.android.com/ndk/guides/neuralnetworks

#### EGL

* https://github.com/fuyufjh/GraphicBuffer
* https://registry.khronos.org/EGL/extensions/ANDROID/

## How to

### Setup

#### ~~NDK Sanitizers~~

It won't be used anymore.

```bash
cd ${ANDROID_NDK_HOME}/build/tools
    INSTALL_PATH=/tmp/llvm/prebuilt/"$(echo $(uname -s)-$(uname -m) | tr '[:upper:]' '[:lower:]')"
    ./make_standalone_toolchain.py --arch=arm64 --api=27 --stl=libc++ --install-dir=${INSTALL_PATH} --force
cd ${INSTALL_PATH}
    tree -L 2 ./lib64/clang/9.0.0/lib
```

### Build

The build step uses [Gradle 7.2.0+](https://docs.gradle.org/current/userguide/userguide.html). If you don't know how to use it, latest [Android Studio](https://developer.android.com/studio/) can do the work.   

```console
$ git clone https://github.com/luncliff/Muffin
$ cd ./Muffin
$ gradle clean assemble
```

### Test

Connect your device and run the test with Gradle.
Please reference the [test codes](./android/test/).

```console
$ gradle connectedAndroidTest   # Run test
```
