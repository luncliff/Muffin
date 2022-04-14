cmake_minimum_required(VERSION 3.21)

# https://github.com/microsoft/vcpkg/blob/master/docs/examples/vcpkg_android_example_cmake_script/CMakeLists.txt
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
include(vcpkg_android)

project(muffin LANGUAGES CXX VERSION 1.2.0)
if(NOT ANDROID)
    message(FATAL_ERROR "requires CMAKE_TOOLCHAIN_FILE=android.toolchain.cmake")
endif()
include(GNUInstallDirs)

find_package(ZLIB REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include(CheckIncludeFileCXX)
check_include_file_cxx("vulkan/vulkan.h" found_vulkan)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

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
message(STATUS "  - platform: ${ANDROID_PLATFORM}")
message(STATUS "  - arch_name: ${ANDROID_ARCH_NAME}")
message(STATUS "  - abi: ${ANDROID_ABI}")
message(STATUS "  - ndk: ${ANDROID_NDK}")
message(STATUS "  - stl: ${ANDROID_STL}")

add_library(muffin
    include/muffin.hpp
    src/muffin.cpp
    src/adapter.cpp
)

# https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-android-with-the-ndk
set_target_properties(muffin
PROPERTIES
    PUBLIC_HEADER   include/muffin.hpp
    CXX_STANDARD 20
    # ANDROID_API_MIN 28
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
    jniLibs/${ANDROID_ABI}
)

target_link_libraries(muffin
PRIVATE
    ${CMAKE_DL_LIBS} ${ANDROID_STL} m android log 
    EGL GLESv3 # camera2ndk mediandk vulkan
    Vulkan::Vulkan
    Microsoft.GSL::GSL spdlog::spdlog
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
