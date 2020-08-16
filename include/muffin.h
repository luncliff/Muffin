/**
 * @file    muffin.h
 * @author  github.com/luncliff (luncliff@gmail.com)
 * 
 * @version 1.1
 * @see https://programming.guide/java/list-of-java-exceptions.html
 */
#pragma once
#ifndef MUFFIN_INCLUDE_H
#define MUFFIN_INCLUDE_H
#if !defined(__ANDROID__) || !defined(__ANDROID_API__)
#error "requries __ANDROID__ and __ANDROID_API__"
#endif
#include <new>
#include <stdexcept>
#include <string_view>

#include <experimental/coroutine>
#endif
