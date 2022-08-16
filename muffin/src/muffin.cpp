#include "muffin.hpp"

#include <android/api-level.h>
#include <android/dlext.h>
#include <android/native_window_jni.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <spdlog/sinks/android_sink.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <time.h>  // todo: acquire resolution https://man7.org/linux/man-pages/man2/clock_getres.2.html
#include <unistd.h>

#include <chrono>

extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
    constexpr auto version = JNI_VERSION_1_6;
    JNIEnv* env{};
    jint result = -1;
    if (vm->GetEnv((void**)&env, version) != JNI_OK) return result;

    auto stream = spdlog::android_logger_st("android", "muffin");
    stream->set_pattern("%v");                // Logcat will report time, thread, and level.
    stream->set_level(spdlog::level::debug);  // just print messages
    spdlog::set_default_logger(stream);
    spdlog::info("Device API Level: {}", android_get_device_api_level());
    return version;
}

void get_field(JNIEnv* env,  //
               jclass _type, jobject target, const char* field_name, jlong& ref) noexcept {
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    ref = env->GetLongField(target, _field);
}

void set_field(JNIEnv* env,  //
               jclass _type, jobject target, const char* field_name, jlong value) noexcept {
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    env->SetLongField(target, _field, value);
}

/**
 * @brief Find exception class information (type info)
 * @see java/lang/RuntimeException
 */
void store_runtime_exception(JNIEnv* env, const char* message) noexcept {
    const char* name = "java/lang/RuntimeException";
    jclass t = env->FindClass(name);
    if (t == nullptr) return spdlog::error("{:s}: {:s}", "No Java class", name);
    env->ThrowNew(t, message);
}

static_assert(sizeof(void*) <= sizeof(jlong), "`jlong` must be able to contain `void*` pointer");

native_loader_t::native_loader_t(const char* libname) noexcept(false) : native_loader_t{} { load(libname); }

native_loader_t::~native_loader_t() noexcept {
    if (handle) dlclose(handle);
}

void native_loader_t::load(const char* libname) noexcept(false) {
    android_dlextinfo info{};
    info.flags = ANDROID_DLEXT_FORCE_LOAD;
    handle = android_dlopen_ext(libname, RTLD_NOW, &info);
    if (handle == nullptr) throw std::system_error{errno, std::system_category(), dlerror()};
}

void* native_loader_t::get_proc_address(const char* proc) const noexcept {
    spdlog::trace("{:s}: {:s}", "dlsym", proc);
    return dlsym(handle, proc);
}

epoll_owner_t::epoll_owner_t() noexcept(false) : epfd{epoll_create1(EPOLL_CLOEXEC)} {
    if (epfd < 0) throw std::system_error{errno, std::system_category(), "epoll_create1"};
}
epoll_owner_t::~epoll_owner_t() noexcept { close(epfd); }

void epoll_owner_t::try_add(uint64_t fd, epoll_event& req) noexcept(false) {
    int op = EPOLL_CTL_ADD, ec = 0;
TRY_OP:
    ec = epoll_ctl(epfd, op, fd, &req);
    if (ec == 0) return;
    if (errno == EEXIST) {
        op = EPOLL_CTL_MOD;  // already exists. try with modification
        goto TRY_OP;
    }
    throw std::system_error{errno, std::system_category(), "epoll_ctl(EPOLL_CTL_ADD|EPOLL_CTL_MODE)"};
}

void epoll_owner_t::remove(uint64_t fd) {
    epoll_event req{};  // just prevent non-null input
    const auto ec = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &req);
    if (ec != 0) throw std::system_error{errno, std::system_category(), "epoll_ctl(EPOLL_CTL_DEL)"};
}
ptrdiff_t epoll_owner_t::wait(uint32_t wait_ms, epoll_event* ptr, int count) noexcept(false) {
    count = epoll_wait(epfd, ptr, count, wait_ms);
    if (count == -1) throw std::system_error{errno, std::system_category(), "epoll_wait"};
    return static_cast<ptrdiff_t>(count);
}

//
//  We are going to combine file descriptor and state bit
//
//  On x86 system,
//    this won't matter since `int` is 32 bit.
//    we can safely use msb for state indicator.
//
//  On x64 system,
//    this might be a hazardous since the value of `eventfd` can be corrupted.
//    **Normally** descriptor in Linux system grows from 3, so it is highly
//    possible to reach system limitation before the value consumes all 63 bit.
//
constexpr uint64_t emask = 1ULL << 63;

//  the msb(most significant bit) will be ...
//   1 if the fd is signaled,
//   0 on the other case
bool is_signaled(uint64_t state) noexcept {
    return emask & state;  // msb is 1?
}

int64_t get_eventfd(uint64_t state) noexcept { return static_cast<int64_t>(~emask & state); }

void notify_event(int64_t efd) noexcept(false) {
    // signal the eventfd...
    //  the message can be any value
    //  since the purpose of it is to trigger the epoll
    //  we won't care about the internal counter of the eventfd
    if (write(efd, &efd, sizeof(efd)) == -1) throw std::system_error{errno, std::system_category(), "write"};
}

void consume_event(int64_t efd) noexcept(false) {
    if (read(efd, &efd, sizeof(efd)) == -1) throw std::system_error{errno, std::system_category(), "read"};
}

event_file_t::event_file_t() noexcept(false) : state{} {
    const auto fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd == -1) throw std::system_error{errno, std::system_category(), "eventfd"};

    this->state = fd;  // start with unsignaled state
}
event_file_t::~event_file_t() noexcept {
    // if already closed, fd == 0
    if (auto fd = get_eventfd(state)) close(fd);
}

uint64_t event_file_t::fd() const noexcept { return get_eventfd(state); }

bool event_file_t::is_set() const noexcept { return is_signaled(state); }

void event_file_t::set() noexcept(false) {
    // already signaled. nothing to do...
    if (is_signaled(state))
        // !!! under the race condition, this check is not safe !!!
        return;

    auto fd = get_eventfd(state);
    notify_event(fd);                           // if it didn't throwed
    state = emask | static_cast<uint64_t>(fd);  //  it's signaled state from now
}

void event_file_t::reset() noexcept(false) {
    const auto fd = get_eventfd(state);
    // if already signaled. nothing to do...
    if (is_signaled(state)) consume_event(fd);
    // make unsignaled state
    this->state = static_cast<uint64_t>(fd);
}

repeat_timer_t::repeat_timer_t() noexcept(false) : handle{timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC)} {
    if (handle == -1)
        throw std::system_error{errno, std::system_category(),
                                "timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC)"};
}

repeat_timer_t::~repeat_timer_t() noexcept { close(handle); }

void repeat_timer_t::start(const timespec& interval) noexcept(false) {
    itimerspec spec{}; 
    clock_gettime(CLOCK_REALTIME, &spec.it_value);  // from current time point,
    spec.it_interval = interval;                    //  repeat with the interval
    if (timerfd_settime(handle, TFD_TIMER_ABSTIME, &spec, nullptr) == -1)
        throw std::system_error{errno, std::system_category(), "timerfd_settime(TFD_TIMER_ABSTIME)"};
}

void repeat_timer_t::stop() noexcept(false) {
    itimerspec spec{};
    // using zero for `spec.it_value` will stop the timer
    if (timerfd_settime(handle, TFD_TIMER_ABSTIME, &spec, nullptr) == -1)
        throw std::system_error{errno, std::system_category(), "timerfd_settime(TFD_TIMER_ABSTIME)"};
}

int repeat_timer_t::fd() const noexcept { return handle; }

/**
 * @brief Bind the given `event`(`eventfd`) to `epoll_owner_t`(Epoll)
 * 
 * @param ep  epoll_owner_t
 * @param efd event
 * @see event
 * @return awaitable struct for the binding
 * @ingroup Linux
 */
auto wait_in(epoll_owner_t& ep, event_file_t& efd) {
    class awaiter_t : epoll_event {
        epoll_owner_t& ep;
        event_file_t& efd;

       public:
        /**
         * @brief Prepares one-time registration
         */
        awaiter_t(epoll_owner_t& _ep, event_file_t& _efd) noexcept : epoll_event{}, ep{_ep}, efd{_efd} {
            this->events = EPOLLET | EPOLLIN | EPOLLONESHOT;
        }

        [[nodiscard]] bool await_ready() const noexcept { return efd.is_set(); }
        /**
         * @brief Wait for `write` to given `eventfd`
         */
        void await_suspend(std::experimental::coroutine_handle<void> coro) noexcept(false) {
            this->data.ptr = coro.address();
            return ep.try_add(efd.fd(), *this);
        }
        /**
         * @brief Reset the given event object when resumed
         */
        void await_resume() noexcept { return efd.reset(); }
    };
    return awaiter_t{ep, efd};
}

extern "C" {
JNIEXPORT jint Java_dev_luncliff_muffin_NativeTimerTest_countWithInterval(  //
    JNIEnv* env, jobject, jint d, jint i) {
    using namespace std::chrono;
    try {
        epoll_owner_t ep{};
        epoll_event events[1]{};
        events[0].events = EPOLLIN;
        events[0].data.ptr = nullptr;

        repeat_timer_t timer{};
        ep.try_add(timer.fd(), events[0]);

        timespec interval{};
        interval.tv_sec = i / 1'000; // ms -> sec
        interval.tv_nsec = i * 1'000'000 - (interval.tv_sec * 1'000'000'000);
        timer.start(interval);

        std::this_thread::sleep_for(milliseconds{d});

        uint64_t count = ep.wait(0, events, 1);
        if (auto rsz = read(timer.fd(), &count, sizeof(uint64_t)); rsz == -1)
            spdlog::error("{}: {} {}", __func__, "read", errno);

        timer.stop();
        return static_cast<jint>(count);
    } catch (const std::exception& ex) {
        spdlog::error("{}: {}", __func__, ex.what());
        return 0;
    }
}
}  // extern "C"
