#include <muffin.hpp>

#include <fmt/format.h>
#define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>

#include <cstdio>
#include <jpeglib.h>
#include <png.h>
#include <turbojpeg.h>

#define LIB_PROLOGUE __attribute__((constructor))
#define LIB_EPILOGUE __attribute__((destructor))
