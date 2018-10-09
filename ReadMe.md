# Muffin
[![Build Status](https://dev.azure.com/luncliff/personal/_apis/build/status/luncliff.Muffin)](https://dev.azure.com/luncliff/personal/_build/latest?definitionId=2) [![Build Status](https://travis-ci.org/luncliff/Muffin.svg?branch=master)](https://travis-ci.org/luncliff/Muffin)

Android Library Project Template. Both for Java/JNI.

  - API level: 24+
  - NDK

### Reference
 - Gradle
 - Android NDK

## How to
### Build
The build step uses [Gradle **4.10.2**](https://gradle.org/). If you don't know how to use it, latest [Android Studio](https://developer.android.com/studio/) can do the work.   
```console
$ git clone https://github.com/luncliff/Muffin
$ cd ./Muffin
$ gradle assemble               # Build: libmuffin.so & Muffin.aar
```

### Test
Connect your device and run the test with Gradle.
Please reference the [test codes](./android/test/muffin/). 
```console
$ gradle connectedAndroidTest   # Run test
```
