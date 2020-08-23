cmake_minimum_required(VERSION 3.8)
include(ExternalProject)

message(STATUS "using: ${CMAKE_SYSTEM_NAME}")
if(ANDROID)
    if(NOT DEFINED ANDROID_PLATFORM)
        message(FATAL_ERROR "requires ANDROID_PLATFORM: android-24")
    endif()
    if(NOT DEFINED ANDROID_STL)
        message(FATAL_ERROR "requires ANDROID_STL: c++_shared")
    endif()
    if(NOT DEFINED ANDROID_ABI)
        message(FATAL_ERROR "requires ANDROID_ABI: arm64-v8a")
    endif()
    list(APPEND ARGS_TOOLCHAIN  -DANDROID_PLATFORM=${ANDROID_PLATFORM}
                                -DANDROID_STL=${ANDROID_STL}
                                -DANDROID_ABI=${ANDROID_ABI}
    )
    message(STATUS "android: ${ARGS_TOOLCHAIN}")
endif()

if(NOT BUILD_SHARED_LIBS)
    set(ENABLE_SHARED 0)
    set(ENABLE_STATIC 1)
else()
    set(ENABLE_SHARED 1)
    set(ENABLE_STATIC 0)
endif()

ExternalProject_Add(install_libjpeg-turbo
    TMP_DIR             ${CMAKE_BINARY_DIR}/downloads
    DOWNLOAD_DIR        ${PROJECT_SOURCE_DIR}/externals
    SOURCE_DIR          ${PROJECT_SOURCE_DIR}/externals/libjpeg-turbo
    GIT_REPOSITORY      https://github.com/libjpeg-turbo/libjpeg-turbo.git
    GIT_TAG             2.0.5
    GIT_SHALLOW         true
    BUILD_IN_SOURCE     true
    CONFIGURE_COMMAND   cmake . -G "${CMAKE_GENERATOR}"
                                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} ${ARGS_TOOLCHAIN}
                                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                -DENABLE_SHARED=${ENABLE_SHARED}
                                -DENABLE_STATIC=${ENABLE_STATIC}
                                -DENABLE_JAVA=false
    BUILD_COMMAND       cmake --build . 
    BUILD_ALWAYS        true
    INSTALL_COMMAND     cmake --build . --target install --config ${CMAKE_BUILD_TYPE}
)

ExternalProject_Add(install_fmt
    TMP_DIR             ${CMAKE_BINARY_DIR}/downloads
    DOWNLOAD_DIR        ${PROJECT_SOURCE_DIR}/externals
    SOURCE_DIR          ${PROJECT_SOURCE_DIR}/externals/fmt
    GIT_REPOSITORY      https://github.com/fmtlib/fmt.git
    GIT_TAG             6.2.1
    GIT_SHALLOW         true
    BUILD_IN_SOURCE     true
    PATCH_COMMAND       git status
    CONFIGURE_COMMAND   cmake . -G "${CMAKE_GENERATOR}"
                                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} ${ARGS_TOOLCHAIN}
                                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                -DBUILD_SHARED_LIBS=${ENABLE_SHARED}
                                -DFMT_DOC=OFF
                                -DFMT_INSTALL=ON
                                -DFMT_FUZZ=OFF
                                -DFMT_TEST=OFF
                                -DFMT_CUDA_TEST=OFF
    BUILD_COMMAND       cmake --build . 
    BUILD_ALWAYS        true
    INSTALL_COMMAND     cmake --build . --target install --config ${CMAKE_BUILD_TYPE}
)

ExternalProject_Add(install_spdlog
    DEPENDS             install_fmt
    TMP_DIR             ${CMAKE_BINARY_DIR}/downloads
    DOWNLOAD_DIR        ${PROJECT_SOURCE_DIR}/externals
    SOURCE_DIR          ${PROJECT_SOURCE_DIR}/externals/spdlog
    GIT_REPOSITORY      https://github.com/gabime/spdlog.git
    GIT_TAG             v1.6.1
    GIT_SHALLOW         true
    BUILD_IN_SOURCE     true
    PATCH_COMMAND       git status
    CONFIGURE_COMMAND   cmake . -G "${CMAKE_GENERATOR}"
                                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} ${ARGS_TOOLCHAIN}
                                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                -DSPDLOG_BUILD_SHARED=${BUILD_SHARED_LIBS}
                                -DSPDLOG_BUILD_EXAMPLE=OFF
                                -DSPDLOG_BUILD_TESTS=OFF
                                -DSPDLOG_BUILD_BENCH=OFF
                                -DSPDLOG_SANITIZE_ADDRESS=OFF
                                -DSPDLOG_FMT_EXTERNAL=ON
                                -DSPDLOG_FMT_EXTERNAL_HO=OFF
                                -DSPDLOG_NO_EXCEPTIONS=OFF
                                -Dfmt_DIR=${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt
    BUILD_COMMAND       cmake --build . 
    BUILD_ALWAYS        true
    INSTALL_COMMAND     cmake --build . --target install --config ${CMAKE_BUILD_TYPE}
)

ExternalProject_Add(install_ms-gsl
    TMP_DIR             ${CMAKE_BINARY_DIR}/downloads
    DOWNLOAD_DIR        ${PROJECT_SOURCE_DIR}/externals
    SOURCE_DIR          ${PROJECT_SOURCE_DIR}/externals/ms-gsl
    GIT_REPOSITORY      https://github.com/microsoft/GSL.git
    GIT_TAG             v3.1.0
    GIT_SHALLOW         true
    BUILD_IN_SOURCE     true
    PATCH_COMMAND       git status
    CONFIGURE_COMMAND   cmake . -G "${CMAKE_GENERATOR}"
                                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} ${ARGS_TOOLCHAIN}
                                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                -DGSL_TEST=false
    BUILD_COMMAND       cmake --build . 
    BUILD_ALWAYS        true
    INSTALL_COMMAND     cmake --build . --target install --config ${CMAKE_BUILD_TYPE}
)

find_package(ZLIB REQUIRED)
get_target_property(ZLIB_INCLUDE_DIR ZLIB::ZLIB INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(ZLIB_LIBRARY     ZLIB::ZLIB IMPORTED_LOCATION_RELEASE)
add_custom_target(install_zlib
    COMMAND     echo    "zlib:"
    COMMAND     echo    " - inc: ${ZLIB_INCLUDE_DIR}"
    COMMAND     echo    " - lib: ${ZLIB_LIBRARY}"
    WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}/externals
)

ExternalProject_Add(install_libpng
    DEPENDS             install_zlib
                        # download_patch_apng
    TMP_DIR             ${CMAKE_BINARY_DIR}/downloads
    DOWNLOAD_DIR        ${PROJECT_SOURCE_DIR}/externals
    SOURCE_DIR          ${PROJECT_SOURCE_DIR}/externals/libpng
    GIT_REPOSITORY      https://github.com/glennrp/libpng.git
    GIT_TAG             v1.6.37
    GIT_SHALLOW         true
    BUILD_IN_SOURCE     true
    # PATCH_COMMAND       git apply --verbose "${patch_apng}" | true
    CONFIGURE_COMMAND   cmake . -G "${CMAKE_GENERATOR}"
                                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} ${ARGS_TOOLCHAIN}
                                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                -DSKIP_INSTALL_PROGRAMS=ON
                                -DSKIP_INSTALL_EXECUTABLES=ON
                                -DSKIP_INSTALL_FILES=ON
                                -DHAVE_LD_VERSION_SCRIPT=false
                                -DHAVE_SOLARIS_LD_VERSION_SCRIPT=false
                                -DPNG_SHARED=${ENABLE_SHARED}
                                -DPNG_STATIC=${ENABLE_STATIC}
                                -DPNG_TESTS=OFF
                                -DPNG_HARDWARE_OPTIMIZATIONS=OFF
                                -DPNG_BUILD_ZLIB=OFF
                                -DZLIB_INCLUDE_DIR=${ZLIB_INCLUDE_DIR}
                                -DZLIB_LIBRARY=${ZLIB_LIBRARY}
    BUILD_COMMAND       cmake --build . 
    BUILD_ALWAYS        true
    INSTALL_COMMAND     cmake --build . --target install --config ${CMAKE_BUILD_TYPE}
)
