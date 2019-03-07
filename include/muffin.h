//
//  Author
//      luncliff@gmail.com
//
#pragma once

#define _INTERFACE_     __attribute__((visibility("default")))
#define _C_INTERFACE_   extern "C" __attribute__((visibility("default")))
#define _HIDDEN_        __attribute__((visibility("hidden")))

#ifndef MUFFIN_INCLUDE_H
#define MUFFIN_INCLUDE_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/android_sink.h>

#include <jni.h>
#include <android/hardware_buffer.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadata.h>
#include <camera/NdkCameraMetadataTags.h>
#include <camera/NdkCaptureRequest.h>

// - Note
//      Group of java native type variables
// - Reference
//      https://programming.guide/java/list-of-java-exceptions.html
struct _HIDDEN_ java_type_set_t final
{
    jclass runtime_exception{};
    jclass illegal_argument_exception{};
    jclass illegal_state_exception{};
    jclass unsupported_operation_exception{};
    jclass index_out_of_bounds_exception{};
};

#endif // MUFFIN_INCLUDE_H
