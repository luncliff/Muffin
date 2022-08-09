#include <jni.h>
#include <string>

extern "C" {

JNIEXPORT jstring JNICALL Java_dev_luncliff_sample_MainActivity_stringFromJNI(JNIEnv* env, jobject) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

}
