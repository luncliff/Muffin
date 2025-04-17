# https://learn.microsoft.com/en-us/vcpkg/users/platforms/android
if(NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "VCPKG_ROOT environment variable is required")
endif()

# Help "vcpkg install" command detect NDK compiler/linker toolchains
if(NOT DEFINED ENV{ANDROID_NDK_HOME})
    if(DEFINED ANDROID_NDK)
        set(ENV{ANDROID_NDK_HOME} "${ANDROID_NDK}")
    endif()
endif()

# Custom vcpkg triplet/port settings in arm64-android.cmake, x64-android.cmake, etc.
if(NOT DEFINED ENV{VCPKG_OVERLAY_TRIPLETS})
    set(ENV{VCPKG_OVERLAY_TRIPLETS} "${CMAKE_CURRENT_LIST_DIR}")
endif()

if(NOT DEFINED VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" CACHE FILEPATH "https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration" FORCE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html" FORCE)
    message(STATUS "Using CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
    message(STATUS "Using VCPKG_CHAINLOAD_TOOLCHAIN_FILE: ${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")
endif()

if(NOT DEFINED VCPKG_MANIFEST_DIR)
    set(VCPKG_MANIFEST_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration#vcpkg_manifest_dir" FORCE)
    message(STATUS "Using VCPKG_MANIFEST_DIR: ${VCPKG_MANIFEST_DIR}")
endif()

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(ANDROID_ABI MATCHES "arm64-v8a")
        set(VCPKG_TARGET_TRIPLET "arm64-android" CACHE STRING "" FORCE)
    elseif(ANDROID_ABI MATCHES "armeabi-v7a")
        set(VCPKG_TARGET_TRIPLET "arm-android" CACHE STRING "" FORCE)
    elseif(ANDROID_ABI MATCHES "x86_64")
        set(VCPKG_TARGET_TRIPLET "x64-android" CACHE STRING "" FORCE)
    elseif(ANDROID_ABI MATCHES "x86")
        set(VCPKG_TARGET_TRIPLET "x86-android" CACHE STRING "" FORCE)
    else()
        message(FALTAL_ERROR "Unsupported ANDROID_ABI: ${ANDROID_ABI}")
    endif()
    message(STATUS "Using VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
endif()
