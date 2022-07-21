#
# We need proper VCPKG_TARGET_TRIPLET for ANDROID_ABI.
# See https://github.com/microsoft/vcpkg/blob/master/docs/examples/vcpkg_android_example_cmake_script/CMakeLists.txt
#
cmake_minimum_required(VERSION 3.21)
# VCPKG_TARGET_TRIPLET is required
include(${CMAKE_CURRENT_SOURCE_DIR}/scripts/vcpkg_android.cmake)

project(muffin LANGUAGES CXX VERSION 1.3.0)
if(NOT ANDROID)
    message(FATAL_ERROR "requires CMAKE_TOOLCHAIN_FILE=android.toolchain.cmake")
endif()
include(GNUInstallDirs)
include(CheckIncludeFileCXX)

# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(ZLIB REQUIRED)
find_package(Vulkan REQUIRED)
find_package(spdlog CONFIG REQUIRED)
# find_package(OpenSSL REQUIRED)
# find_package(CURL CONFIG REQUIRED)

check_include_file_cxx("vulkan/vulkan.h" found_vulkan)
check_include_file_cxx("GLES3/gl3.h" found_gles3)
check_include_file_cxx("EGL/egl.h" found_egl)
check_include_file_cxx("EGL/eglext_angle.h" found_egl_angle)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_INSTALL_LIBDIR "lib/${ANDROID_ABI}")

message(STATUS "project:")
message(STATUS "  name: ${PROJECT_NAME}")
message(STATUS "  version: ${PROJECT_VERSION}")
message(STATUS "  source: ${PROJECT_SOURCE_DIR}")
message(STATUS "cmake:")
message(STATUS "  version: ${CMAKE_VERSION}")
message(STATUS "  toolchain: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "  find_root_path: ${CMAKE_FIND_ROOT_PATH}") 
message(STATUS "compiler:")
message(STATUS "  id: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  path: ${CMAKE_CXX_COMPILER}")
message(STATUS "android:")
message(STATUS "  platform: ${ANDROID_PLATFORM}")
message(STATUS "  arch_name: ${ANDROID_ARCH_NAME}")
message(STATUS "  abi: ${ANDROID_ABI}")
message(STATUS "  ndk: ${ANDROID_NDK}")
message(STATUS "  stl: ${ANDROID_STL}")

list(APPEND headers src/muffin.hpp)

add_library(muffin SHARED
    ${headers}
    src/muffin.cpp
    src/adapter.cpp
)

# https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-android-with-the-ndk
set_target_properties(muffin
PROPERTIES
    PUBLIC_HEADER "${headers}"
    CXX_STANDARD 20
    ANDROID_API_MIN 26
)

target_include_directories(muffin
PRIVATE
    include
    src
)

target_compile_definitions(muffin
PRIVATE
    AUTHOR_LABEL="luncliff@gmail.com"
)

target_compile_options(muffin
PRIVATE
    -fcoroutines-ts
    -ferror-limit=4
)

target_link_directories(muffin
PRIVATE
    # ${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib
)

target_link_libraries(muffin
PRIVATE
    ${CMAKE_DL_LIBS} ${ANDROID_STL} m android log EGL GLESv3 # camera2ndk mediandk
    Vulkan::Vulkan
    # ZLIB::ZLIB OpenSSL::SSL OpenSSL::Crypto CURL::libcurl
    fmt::fmt-header-only spdlog::spdlog_header_only
)

target_link_options(muffin
PRIVATE
    # ...
)

install(TARGETS         muffin
        EXPORT          muffin-config
        RUNTIME         DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY         DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT      muffin-config
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/muffin
)

include(CMakePackageConfigHelpers)
set(VERSION_FILE_PATH   ${CMAKE_BINARY_DIR}/muffin-config-version.cmake)
write_basic_package_version_file(${VERSION_FILE_PATH}
    VERSION             ${PROJECT_VERSION}
    COMPATIBILITY       SameMajorVersion
)
install(FILES       ${VERSION_FILE_PATH} 
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/muffin
)
