#
#  Author
#      luncliff@gmail.com
#
cmake_minimum_required(VERSION 3.10)
project(muffin LANGUAGES CXX VERSION 1.1)

if(NOT ANDROID)
    message(FATAL_ERROR "requires CMAKE_TOOLCHAIN_FILE=android.toolchain.cmake")
endif()

# make .so by default
if(NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS true) 
endif()

# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
message(STATUS "${PROJECT_NAME}:")
message(STATUS " - version: ${PROJECT_VERSION}")
message(STATUS " - project_dir: ${PROJECT_SOURCE_DIR}")
message(STATUS " - cmake:")
message(STATUS "   - version:   ${CMAKE_VERSION}")
message(STATUS "   - toolchain: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS " - compiler")
message(STATUS "   - id: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "   - version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "   - path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Android:")
message(STATUS "  - platform:  ${ANDROID_PLATFORM}")  #
message(STATUS "  - arch_name: ${ANDROID_ARCH_NAME}") # 
message(STATUS "  - abi: ${ANDROID_ABI}") # arm64-v8a 
message(STATUS "  - ndk: ${ANDROID_NDK}") # Path/to/NDK
message(STATUS "  - stl: ${ANDROID_STL}") # c++_shared

add_library(muffin
    include/muffin.h
    src/libmain.cpp
)

#
# NDK configuration
#

# https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-android-with-the-ndk
set_target_properties(muffin
PROPERTIES
    ANDROID_API_MIN 28
)

target_include_directories(muffin
PRIVATE
    ${ANDROID_NDK}/sysroot/usr/include/ 
    ${ANDROID_NDK}/sources/android/support/include/
    ${ANDROID_NDK}/platforms/${ANDROID_PLATFORM}/arch-${ANDROID_ARCH_NAME}/usr/include/ 
)

target_link_libraries(muffin
PRIVATE
    ${CMAKE_DL_LIBS} ${ANDROID_STL} m 
    android log camera2ndk mediandk
    vulkan
)

# path for <asm/types.h>
if(ANDROID_ABI STREQUAL "arm64-v8a")
    target_include_directories(muffin
    PRIVATE
        ${ANDROID_NDK}/sysroot/usr/include/aarch64-linux-android
    )
elseif(ANDROID_ABI MATCHES "arm")
    target_include_directories(muffin
    PRIVATE
        ${ANDROID_NDK}/sysroot/usr/include/arm-linux-androideabi
    )
elseif(ANDROID_ABI MATCHES "x86") # x86, x86_64
    target_include_directories(muffin
    PRIVATE
        ${ANDROID_NDK}/sysroot/usr/include/x86_64-linux-android
    )
endif()

if(ANDROID_STL STREQUAL "c++_shared")
    target_include_directories(muffin
    PRIVATE
        ${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/include/ 
    )
    # target_link_directories(muffin
    # PRIVATE
    #     ${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/ANDROID_ABI/
    # )
    target_compile_options(muffin
    PRIVATE
        -stdlib=libc++ -std=c++2a
        -fcoroutines-ts
    )

elseif(ANDROID_STL STREQUAL "gnustl_shared")
    message(WARNING "recommend ANDROID_STL=c++_shared")
    target_include_directories(muffin
    PRIVATE
        ${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/include/ 
    )
    # target_link_directories(muffin
    # PRIVATE
    #     ${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/ANDROID_ABI/
    # )

else()
    message(FATAL_ERROR "requires ANDROID_STL 'c++_shared' or 'gnustl_shared'")
endif()

# target_link_directories(muffin
# PRIVATE
#     ${ANDROID_NDK}/platforms/${ANDROID_PLATFORM}/arch-${ANDROID_ARCH_NAME}/usr/lib/    
# PRIVATE
#     android/jniLibs/${ANDROID_ABI}
# )


#
# Module configuration
#

target_sources(muffin
PRIVATE
    android/jni/adapter.h
    android/jni/adapter.cpp
)

target_include_directories(muffin
PRIVATE
    include    
)

target_link_libraries(muffin
PRIVATE
    ${CMAKE_DL_LIBS} ${ANDROID_STL} m
    android log camera2ndk mediandk
    GLESv2 vulkan
)

target_compile_definitions(muffin
PRIVATE
    AUTHOR_LABEL="luncliff@gmail.com"
)

target_compile_options(muffin
PRIVATE
    -fvisibility=default -ferror-limit=4
    -std=c++2a -stdlib=libc++
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(muffin
    PRIVATE
        -g -O0
    )
else()
    target_compile_options(muffin
    PRIVATE
        -O3
    )
endif()
