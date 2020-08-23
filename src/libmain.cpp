#include <muffin.h>

#include <fmt/format.h>
#define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>

#define LIB_PROLOGUE __attribute__((constructor))
#define LIB_EPILOGUE __attribute__((destructor))
