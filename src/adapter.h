/**
 * @file    adapter.h
 * @author  github.com/luncliff (luncliff@gmail.com)
 */
#pragma once
#include <muffin.hpp>

#include <android/hardware_buffer.h>
#include <android/log.h>
#include <android/api-level.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/sensor.h>

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadata.h>
#include <camera/NdkCameraMetadataTags.h>
#include <camera/NdkCaptureRequest.h>

#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>

