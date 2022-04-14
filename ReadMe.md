# Muffin

[![Build Status](https://dev.azure.com/luncliff/personal/_apis/build/status/luncliff.Muffin?branchName=master)](https://dev.azure.com/luncliff/personal/_build/latest?definitionId=34&branchName=master)
[![CircleCI](https://circleci.com/gh/luncliff/Muffin/tree/master.svg?style=shield)](https://circleci.com/gh/luncliff/Muffin/tree/master)

Template for Android NDK module

### References

* [NDK Samples(GitHub)](https://github.com/android/ndk-samples)
* [Android Developer Guide](https://developer.android.com/guide)
  * [Testing](https://developer.android.com/training/testing/unit-testing)
* [Testing with JUnit 5 for Android](https://github.com/mannodermaus/android-junit5)
* https://developer.android.com/studio/projects/gradle-external-native-builds
  * https://developer.android.com/studio/build/native-dependencies?hl=en
  * https://google.github.io/prefab/

## How to

### Setup

#### NDK Sanitizers

```bash
cd ${ANDROID_NDK_HOME}/build/tools
    INSTALL_PATH=/tmp/llvm/prebuilt/"$(echo $(uname -s)-$(uname -m) | tr '[:upper:]' '[:lower:]')"
    ./make_standalone_toolchain.py --arch=arm64 --api=27 --stl=libc++ --install-dir=${INSTALL_PATH} --force
cd ${INSTALL_PATH}
    tree -L 2 ./lib64/clang/9.0.0/lib
```

### Build

The build step uses [Gradle 6.7.1+](https://gradle.org/). If you don't know how to use it, latest [Android Studio](https://developer.android.com/studio/) can do the work.   

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
