/**
 * @file    adapter.h
 * @author  github.com/luncliff (luncliff@gmail.com)
 */
#pragma once
#include <android/api-level.h>
#include <android/hardware_buffer.h>
#include <android/native_window_jni.h>
#include <android/sensor.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include <muffin.hpp>

#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>
