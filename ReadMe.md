# Muffin

[![Build Status](https://dev.azure.com/luncliff/personal/_apis/build/status/luncliff.Muffin)](https://dev.azure.com/luncliff/personal/_build/latest?definitionId=2) [![Build Status](https://travis-ci.org/luncliff/Muffin.svg?branch=master)](https://travis-ci.org/luncliff/Muffin)

Template for Android NDK module

* API level: 28+

### References

* [Android Developer Guide](https://developer.android.com/guide)
  * [Testing](https://developer.android.com/training/testing/unit-testing)
* [Testing with JUnit 5 for Android](https://github.com/mannodermaus/android-junit5)

## How to

### Build

The build step uses [Gradle 5.x](https://gradle.org/). If you don't know how to use it, latest [Android Studio](https://developer.android.com/studio/) can do the work.   

```console
$ git clone https://github.com/luncliff/Muffin
$ cd ./Muffin
$ gradle assemble
```

### Test

Connect your device and run the test with Gradle.
Please reference the [test codes](./android/test/).

```console
$ gradle connectedAndroidTest   # Run test
```
