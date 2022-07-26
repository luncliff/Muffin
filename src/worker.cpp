#include <jni.h>
#include <spdlog/spdlog.h>

#include <experimental/coroutine>

void store_runtime_exception(JNIEnv *env, const char *message) noexcept;

//jobject make_runnable(JNIEnv *env, std::experimental::coroutine_handle<> task) noexcept {
//    jclass _type = env->FindClass("muffin/NativeRunnable");
//    jobject _task = env->AllocObject(_type);
//    set_field(env, _type, _task, "handle",
//              reinterpret_cast<jlong>(task.address()));
//    return _task;
//}

class jni_executor_t final {
    JNIEnv *env;
    jobject executor;
    jclass type;
    jmethodID method;

   public:
    jni_executor_t(JNIEnv *env, jobject executor) noexcept(false)
        : env{env}, executor{executor}, type{env->FindClass("java/util/concurrent/Executor")}, method{} {
        if (type == nullptr) {
            auto msg = fmt::format("No Java class: {}", "java/util/concurrent/Executor");
            spdlog::error("{:s}", msg);
            throw std::runtime_error{msg};
        }
        method = env->GetMethodID(type, "execute", "(Ljava/lang/Runnable;)V");
        if (method == nullptr) {
            auto msg = fmt::format("No Java method: {} {}", "java/util/concurrent/Executor", "execute");
            spdlog::error("{:s}", msg);
            throw std::runtime_error{msg};
        }
    }

    void execute(jobject task) noexcept { env->CallVoidMethod(executor, method, task); }
};

JNIEXPORT
uint32_t schedule(JNIEnv *env, jobject executor, jobject task) {
    try {
        jni_executor_t ex{env, executor};
        ex.execute(task);
        return 0;
    } catch (const std::runtime_error &ex) {
        spdlog::error("{}", ex.what());
        return 1;
    }
}