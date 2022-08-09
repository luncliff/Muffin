cmake_minimum_required(VERSION 3.18.1)
project(sample LANGUAGES CXX)

find_library(NDK_LOG_LIBPATH NAMES log REQUIRED)

add_library(sample SHARED
    native-lib.cpp
)

set_target_properties(sample
PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
)

target_link_libraries(sample
PRIVATE
    ${ANDROID_STL}
    ${NDK_LOG_LIBPATH}
)
