# Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

cmake_minimum_required(VERSION 3.22.1)

## Project definition
project(godot_meta_toolkit LANGUAGES CXX)

## Godot OpenXR Vendors submodule dependencies
include(${PROJECT_SOURCE_DIR}/common.cmake)

## Platform SDK
set(PLATFORM_SDK_ROOT_DIR "${PROJECT_SOURCE_DIR}/../thirdparty/ovr_platform_sdk")
set(PLATFORM_SDK_HEADERS_DIR "${PLATFORM_SDK_ROOT_DIR}/Include" CACHE STRING "")
set(PLATFORM_SDK_LIB_PATH "${PLATFORM_SDK_ROOT_DIR}/Android/libs/${ANDROID_ABI}/libovrplatformloader.so")
add_library(ovrplatformloader
        SHARED
        IMPORTED GLOBAL
        )
set_target_properties(ovrplatformloader PROPERTIES IMPORTED_LOCATION ${PLATFORM_SDK_LIB_PATH})

# Project headers
set(TOOLKIT_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/main/cpp/include)
set(TOOLKIT_GENERATED_HEADERS_DIR ${PROJECT_SOURCE_DIR}/gen/include)

# Setup the project sources
file(GLOB_RECURSE ANDROID_SOURCES ${PROJECT_SOURCE_DIR}/src/main/cpp/*.c**)
file(GLOB_RECURSE ANDROID_HEADERS ${PROJECT_SOURCE_DIR}/src/main/cpp/*.h**)
file(GLOB_RECURSE GENERATED_SOURCES ${PROJECT_SOURCE_DIR}/gen/*.c**)
file(GLOB_RECURSE GENERATED_HEADERS ${PROJECT_SOURCE_DIR}/gen/*.h**)

add_library(${PROJECT_NAME}
        SHARED
        ${ANDROID_SOURCES}
        ${ANDROID_HEADERS}
        ${GENERATED_SOURCES}
        ${GENERATED_HEADERS}
        )

target_include_directories(${PROJECT_NAME}
        SYSTEM PUBLIC
        ${GODOT_CPP_INCLUDE_DIRECTORIES}
        ${PLATFORM_SDK_HEADERS_DIR}
        ${TOOLKIT_HEADERS_DIR}
        ${TOOLKIT_GENERATED_HEADERS_DIR}
        )

target_link_libraries(${PROJECT_NAME}
        android
        log
        ${GODOT-CPP}
        ovrplatformloader
        )

# Add the compile flags
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS ${GODOT_COMPILE_FLAGS})
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS ${GODOT_LINKER_FLAGS})
