/**
 * @file    muffin.h
 * @author  github.com/luncliff (luncliff@gmail.com)
 * 
 * @version 1.1
 * @see https://programming.guide/java/list-of-java-exceptions.html
 */
#pragma once
#if !defined(__ANDROID__) || !defined(__ANDROID_API__)
#error "requries __ANDROID__ and __ANDROID_API__"
#endif
static_assert(__cplusplus >= 201703L, "requires C++ 17 or later");
#include <array>
#include <cerrno>
#include <experimental/coroutine>
#include <new>
#include <stdexcept>
#include <string_view>
#include <string>
