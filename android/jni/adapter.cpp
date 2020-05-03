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
}

/**
 * @brief Find exception class information (type info)
 * @return java/lang/RuntimeException
 */
jclass get_runtime_exception(JNIEnv* env) {
    return env->FindClass("java/lang/RuntimeException");
}

class compass_t final {
    ALooper* const looper;
    ASensorManager* const manager;
    ASensorList sensors = nullptr;
    int count = 0;

  public:
    explicit compass_t(const char* _id,
                       ALooper* _looper = ALooper_prepare(
                           ALOOPER_PREPARE_ALLOW_NON_CALLBACKS)) noexcept(false)
        : looper{_looper}, manager{ASensorManager_getInstanceForPackage(_id)} {
        // increase reference if the looper is provided
        if (ALooper_forThread() != looper)
            ALooper_acquire(looper);
        // sensors[0]...
        count = ASensorManager_getSensorList(manager, &sensors);
        __android_log_print(ANDROID_LOG_INFO, "[muffin]", "sensor count: %u",
                            get_count());
    }
    ~compass_t() noexcept {
        ALooper_release(looper);
    }

    uint32_t get_count() const noexcept {
        return static_cast<uint32_t>(count);
    }
};

void get_field(JNIEnv* env,                    //
               const char* t, jobject _object, //
               const char* f, jlong& ref) {
    const jclass _type = env->FindClass(t);
    const jfieldID _field = env->GetFieldID(_type, f, "J"); // long
    ref = env->GetLongField(_object, _field);
}

void set_field(JNIEnv* env,                    //
               const char* t, jobject _object, //
               const char* f, jlong value) {
    const jclass _type = env->FindClass(t);
    const jfieldID _field = env->GetFieldID(_type, f, "J"); // long
    env->SetLongField(_object, _field, value);
}

static_assert(sizeof(void*) <= sizeof(jlong),
              "`jlong` must be able to contain `void*` pointer");

extern "C" {

jlong Java_muffin_Compass_create(JNIEnv* env, jclass type, jstring _id) {
    char name[60]{};
    env->GetStringUTFRegion(_id, 0, env->GetStringLength(_id), name);

    auto* impl = new compass_t{name};
    return reinterpret_cast<jlong>(impl);
}

void Java_muffin_Compass_close(JNIEnv* env, jobject _object) {
    jlong value = 0;
    get_field(env, "muffin/Compass", _object, "impl", value);
    auto* impl = reinterpret_cast<compass_t*>(value);
    delete impl;
}

jstring Java_muffin_Compass_getName(JNIEnv* env, jobject _object) {
    jlong value = 0;
    get_field(env, "muffin/Compass", _object, "impl", value);
    char buf[20]{};
    snprintf(buf, 20, "%lx", static_cast<uint64_t>(value));
    return env->NewStringUTF(buf);
}

} // extern "C"
