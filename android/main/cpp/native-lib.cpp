/**
 * @see https://docs.oracle.com/en/java/javase/17/docs/specs/jni/index.html
 * @see https://docs.oracle.com/en/java/javase/17/docs/specs/jni/functions.html
 */
#include <jni.h>
#include <jni_bind_release.h>

#include <string>
#include <string_view>

extern "C" {

/// @brief JNI `jchar` is 16 bit. The matching type in C++ is `std::u16string` and its family
JNIEXPORT jstring JNICALL Java_dev_luncliff_muffin_Bridge_stringFromJNI(JNIEnv *env, jclass) {
    static_assert(std::is_same_v<jchar, uint16_t>);
    std::u16string_view text = u"Hello from C++";
    return env->NewString(reinterpret_cast<const jchar *>(text.data()), static_cast<jsize>(text.length()));
}

/// @brief use the version macro from the CMakeLists.txt
JNIEXPORT jstring JNICALL Java_dev_luncliff_muffin_Bridge_getBuildVersion(JNIEnv *env, jclass) {
#if !defined(VERSION)
#define VERSION "2025.1.0"
#endif
    std::string_view text = VERSION;
    return env->NewStringUTF(VERSION);
}
}
