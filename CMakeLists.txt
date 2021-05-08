#
#  Author
#      luncliff@gmail.com
#
cmake_minimum_required(VERSION 3.13)

# https://github.com/microsoft/vcpkg/blob/master/docs/examples/vcpkg_android_example_cmake_script/CMakeLists.txt
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
include(vcpkg_android)

project(muffin LANGUAGES CXX VERSION 1.1.3)
if(NOT ANDROID)
    message(FATAL_ERROR "requires CMAKE_TOOLCHAIN_FILE=android.toolchain.cmake")
endif()

# make .so by default
if(NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS true) 
endif()

find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include(CheckIncludeFileCXX)
check_include_file_cxx("vulkan/vulkan.h" found_vulkan)

# https://developer.android.com/studio/build/native-dependencies?hl=en
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
message(STATUS "${PROJECT_NAME}:")
message(STATUS " - version: ${PROJECT_VERSION}")
message(STATUS " - project_dir: ${PROJECT_SOURCE_DIR}")
message(STATUS "cmake:")
message(STATUS " - version:   ${CMAKE_VERSION}")
message(STATUS " - toolchain: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS " - find_root_path: ${CMAKE_FIND_ROOT_PATH}") 
message(STATUS "compiler:")
message(STATUS " - id: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " - version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " - path: ${CMAKE_CXX_COMPILER}")
message(STATUS "android:")
message(STATUS "  - platform:  ${ANDROID_PLATFORM}")  #
message(STATUS "  - arch_name: ${ANDROID_ARCH_NAME}") # 
message(STATUS "  - abi: ${ANDROID_ABI}") # arm64-v8a 
message(STATUS "  - ndk: ${ANDROID_NDK}") # Path/to/NDK
message(STATUS "  - stl: ${ANDROID_STL}") # c++_shared

add_library(muffin
    include/muffin.hpp
    src/libmain.cpp
    src/adapter.h
    src/adapter.cpp
)

# https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-android-with-the-ndk
set_target_properties(muffin
PROPERTIES
    PUBLIC_HEADER   include/muffin.hpp
    ANDROID_API_MIN 28
)

if(ANDROID_STL STREQUAL "c++_shared")
    target_compile_options(muffin
    PUBLIC
        -stdlib=libc++ -std=c++2a
        -fcoroutines-ts
    )

elseif(ANDROID_STL STREQUAL "gnustl_shared")
    message(WARNING "recommend ANDROID_STL=c++_shared")
else()
    message(FATAL_ERROR "requires ANDROID_STL 'c++_shared' or 'gnustl_shared'")
endif()

target_include_directories(muffin
PRIVATE
    include jniLibs/include
)

target_link_directories(muffin
PRIVATE
    jniLibs/${ANDROID_ABI}
)

target_link_libraries(muffin
PRIVATE
    ${CMAKE_DL_LIBS} ${ANDROID_STL} m android log 
    EGL GLESv3 # camera2ndk mediandk vulkan
    Microsoft.GSL::GSL spdlog::spdlog
)

target_compile_definitions(muffin
PRIVATE
    AUTHOR_LABEL="luncliff@gmail.com"
    SPDLOG_NO_TLS
)

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    target_compile_options(muffin
    PRIVATE
        -ferror-limit=4
    )
endif()

# https://developer.android.com/ndk/guides/asan
if(false)
    # https://github.com/google/sanitizers/wiki/AddressSanitizer
    # https://source.android.com/devices/tech/debug/sanitizers
    if(ANDROID_PLATFORM_LEVEL GREATER 26 AND ANDROID_ARCH_NAME MATCHES "arm")
        message(STATUS "using sanitizers: ${ANDROID_PLATFORM} ${ANDROID_ABI} ${ANDROID_ARCH_NAME}")
        target_compile_options(muffin
        INTERFACE
            -fno-omit-frame-pointer
            -fsanitize=address      # https://clang.llvm.org/docs/AddressSanitizer.html#limitations
            -fsanitize=undefined    # https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html#available-checks
        )
        if(ANDROID_ABI STREQUAL armeabi-v7a)
            set(san_arch arm)
        elseif(ANDROID_ABI STREQUAL arm64-v8a)
            set(san_arch aarch64)
        elseif(ANDROID_ABI STREQUAL x86)
            set(san_arch i686)
        elseif(ANDROID_ABI STREQUAL x86_64)
            set(san_arch x86_64)
        endif()
        # replaces `target_link_directories` for CMake 3.13+
        set(san_pattern "clang_rt.*-${san_arch}-android")
        file(GLOB sanitizers 
            ${internal_dir}/libs/${ANDROID_ABI}/lib${san_pattern}.so)
        target_link_libraries(muffin
        INTERFACE
            ${sanitizers}   # clang_rt.asan-${san_arch}-android
                            # clang_rt.ubsan_minimal-${san_arch}-android
        )
    endif()
endif()

install(TARGETS         muffin
        EXPORT          ${PROJECT_NAME}-config
        RUNTIME         DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)
install(EXPORT      ${PROJECT_NAME}-config
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
set(VERSION_FILE_PATH   ${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake)
write_basic_package_version_file(${VERSION_FILE_PATH}
    VERSION             ${PROJECT_VERSION}
    COMPATIBILITY       SameMajorVersion
)
install(FILES           ${VERSION_FILE_PATH} 
        DESTINATION     ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
)
