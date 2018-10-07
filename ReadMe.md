# Muffin
[![Build Status](https://travis-ci.org/luncliff/Muffin.svg?branch=master)](https://travis-ci.org/luncliff/Muffin)

Android Library Project Template. Both for Java/JNI.

  - API level: 24+
  - NDK

### Reference
 - Gradle
 - Android NDK

## How to
### Build
For **Windows** environment, latest [Android Studio](https://developer.android.com/studio/) is recommended.   
For **Linux/MacOS**, [Gradle 4.4+](https://gradle.org/) will be enough.   

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
