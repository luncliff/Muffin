cmake_minimum_required(VERSION 3.21)
if(NOT DEFINED MUFFIN_VERSION)
    set(MUFFIN_VERSION 2025.3.0)
endif()
project(muffin LANGUAGES CXX VERSION ${MUFFIN_VERSION})

include(GNUInstallDirs)

find_library(LOG_LIB NAMES log REQUIRED)
find_library(ANDROID_LIB NAMES android REQUIRED)

add_library(${CMAKE_PROJECT_NAME} SHARED)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    main/cpp/native-lib.cpp
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    -DVERSION="${PROJECT_VERSION}"
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    ${LOG_LIB}
    ${ANDROID_LIB}
)
