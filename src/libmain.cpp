// ---------------------------------------------------------------------------
//
//  Author
//      luncliff@gmail.com
//
// ---------------------------------------------------------------------------
#include <muffin.h>

#define LIB_PROLOGUE __attribute__((constructor))
#define LIB_EPILOGUE __attribute__((destructor))

using logger_ptr_t = std::shared_ptr<spdlog::logger>;
logger_ptr_t logger{};

// On dll is attached...
LIB_PROLOGUE void OnAttach(void *) noexcept(false)
{
    static constexpr auto tag_muffin = "muffin";

    // log will print thread id and message
    spdlog::set_pattern("[thread %t] %v");
    logger = spdlog::android_logger_st("android", tag_muffin);

    logger->debug("{} {}", tag_muffin, "loaded");
    return;
}

LIB_EPILOGUE void OnDetach(void *) noexcept
{
    // On dll is detached...
    return;
}
