#pragma once
#if !defined(__ANDROID__) || !defined(__ANDROID_API__)
#error "requries __ANDROID__ and __ANDROID_API__"
#endif
static_assert(__cplusplus >= 201703L, "requires C++ 17 or later");

#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <android/api-level.h>
#include <android/hardware_buffer.h>
#include <sys/epoll.h>

#include <cerrno>
#include <experimental/coroutine>
#include <system_error>

#include "egl_context.hpp"
#include "egl_surface.hpp"

class native_loader_t final {
    void* handle;

   public:
    native_loader_t() noexcept = default;
    explicit native_loader_t(const char* name) noexcept(false);
    ~native_loader_t() noexcept;

    void load(const char* name) noexcept(false);
    void* get_proc_address(const char* proc) const noexcept;
};

/**
 * @brief RAII wrapping for epoll file descriptor
 * @ingroup Linux
 */
class epoll_owner_t final {
    int64_t epfd;

   public:
    /**
     * @brief create a fd with `epoll`. Throw if the function fails.
     * @see kqeueue
     * @throw system_error
     */
    epoll_owner_t() noexcept(false);
    /**
     * @brief close the current epoll file descriptor
     */
    ~epoll_owner_t() noexcept;
    epoll_owner_t(const epoll_owner_t&) = delete;
    epoll_owner_t(epoll_owner_t&&) = delete;
    epoll_owner_t& operator=(const epoll_owner_t&) = delete;
    epoll_owner_t& operator=(epoll_owner_t&&) = delete;

   public:
    /**
     * @brief bind the fd to epoll
     * @param fd
     * @param req
     * @see epoll_ctl
     * @throw system_error
     */
    void try_add(uint64_t fd, epoll_event& req) noexcept(false);

    /**
     * @brief unbind the fd to epoll
     * @param fd
     * @see epoll_ctl
     */
    void remove(uint64_t fd);

    /**
     * @brief fetch all event_ts for the given kqeueue descriptor
     * @param wait_ms millisecond to wait
     * @param list
     * @return ptrdiff_t
     * @see epoll_wait
     * @throw system_error
     *
     * Timeout is not an error for this function
     */
    [[nodiscard]] ptrdiff_t wait(uint32_t wait_ms, epoll_event* ptr, int count) noexcept(false);

   public:
    /**
     * @brief return temporary awaitable object for given event_t
     * @param req input for `change` operation
     * @see change
     *
     * There is no guarantee of reusage of returned awaiter object
     * When it is awaited, and `req.udata` is null(0),
     * the value is set to `coroutine_handle<void>`
     *
     * ```cpp
     * auto edge_in_async(epoll_owner_t& ep, int64_t fd) -> frame_t {
     *     epoll_event_t req{};
     *     req.event_ts = EPOLLET | EPOLLIN | EPOLLONESHOT;
     *     req.data.ptr = nullptr;
     *     co_await ep.submit(fd, req);
     * }
     * ```
     */
    [[nodiscard]] auto submit(int64_t fd, epoll_event& req) noexcept {
        class awaiter_t final : public std::experimental::suspend_always {
            epoll_owner_t& ep;
            int64_t fd;
            epoll_event& req;

           public:
            constexpr awaiter_t(epoll_owner_t& _ep, int64_t _fd, epoll_event& _req) : ep{_ep}, fd{_fd}, req{_req} {}

           public:
            void await_suspend(std::experimental::coroutine_handle<void> coro) noexcept(false) {
                if (req.data.ptr == nullptr) req.data.ptr = coro.address();
                return ep.try_add(fd, req);
            }
        };
        return awaiter_t{*this, fd, req};
    }
};

/**
 * @brief RAII + stateful `event_tfd`
 * @see https://github.com/grpc/grpc/blob/master/src/core/lib/iomgr/is_epollexclusive_available.cc
 * @ingroup Linux
 * 
 * If the object is signaled(`set`), 
 * the bound `epoll_owner_t` will yield suspended coroutine through `epoll_event_t`'s user data.
 * 
 * Its object can be `co_await`ed multiple times
 */
class event_t final {
    uint64_t state;

   public:
    event_t() noexcept(false);
    ~event_t() noexcept;
    event_t(const event_t&) = delete;
    event_t(event_t&&) = delete;
    event_t& operator=(const event_t&) = delete;
    event_t& operator=(event_t&&) = delete;

    uint64_t fd() const noexcept;
    bool is_set() const noexcept;
    void set() noexcept(false);
    void reset() noexcept(false);
};

/**
 * @see https://man7.org/linux/man-pages/man2/timerfd_create.2.html
 */
class repeat_timer_t final {
    int handle;

   public:
    explicit repeat_timer_t() noexcept(false);
    ~repeat_timer_t() noexcept;
    repeat_timer_t(const repeat_timer_t&) = delete;
    repeat_timer_t(repeat_timer_t&&) = delete;
    repeat_timer_t& operator=(const repeat_timer_t&) = delete;
    repeat_timer_t& operator=(repeat_timer_t&&) = delete;

    void start(const timespec& interval) noexcept(false);
    void stop() noexcept(false);

    int fd() const noexcept;
};
