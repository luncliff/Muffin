/**
 * @file    adapter.cpp
 * @author  github.com/luncliff (luncliff@gmail.com)
 */
#include "adapter.h"

extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
    constexpr auto version = JNI_VERSION_1_6;
    JNIEnv* env{};
    jint result = -1;
    if (vm->GetEnv((void**)&env, version) != JNI_OK) {
        return result;
    }
    return version;
    // jclass
    // // Find exception class information (type info)
    // java.runtime_exception = env->FindClass("java/lang/RuntimeException");
    // java.illegal_argument_exception =
    //     env->FindClass("java/lang/IllegalArgumentException");
    // java.illegal_state_exception =
    //     env->FindClass("java/lang/IllegalStateException");
    // java.unsupported_operation_exception =
    //     env->FindClass("java/lang/UnsupportedOperationException");
    // java.index_out_of_bounds_exception =
    //     env->FindClass("java/lang/IndexOutOfBoundsException");
}
