// ---------------------------------------------------------------------------
//
//  Author
//      luncliff@gmail.com
//
// ---------------------------------------------------------------------------
#include "adapter.h"

extern std::shared_ptr<spdlog::logger> logger;

java_type_set_t java{};

_C_INTERFACE_ jint JNI_OnLoad(JavaVM* vm, void*)
{
    assert(vm != nullptr);

    JNIEnv* env{};
    vm->GetEnv(reinterpret_cast<void**>(std::addressof(env)), JNI_VERSION_1_6);
    assert(env != nullptr);

    // Find exception class information (type info)
    java.runtime_exception = env->FindClass(
        "java/lang/RuntimeException");
    java.illegal_argument_exception = env->FindClass(
        "java/lang/IllegalArgumentException");
    java.illegal_state_exception = env->FindClass(
        "java/lang/IllegalStateException");
    java.unsupported_operation_exception = env->FindClass(
        "java/lang/UnsupportedOperationException");
    java.index_out_of_bounds_exception = env->FindClass(
        "java/lang/IndexOutOfBoundsException");

    assert(java.runtime_exception != nullptr);
    assert(java.illegal_argument_exception != nullptr);
    assert(java.illegal_state_exception != nullptr);
    assert(java.unsupported_operation_exception != nullptr);
    assert(java.index_out_of_bounds_exception != nullptr);

    return JNI_VERSION_1_6;
}
