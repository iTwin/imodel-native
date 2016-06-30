/*
 * Copyright 2016 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

// BENTLEY modifications to the Facebook Folly library.

#ifdef __BE_FOLLY_DLL__
    #define BE_FOLLY_EXPORT EXPORT_ATTRIBUTE
#else
    #define BE_FOLLY_EXPORT IMPORT_ATTRIBUTE
#endif

#include <cstdint>
#include <thread>
#include <Bentley/RefCounted.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeThreadLocalStorage.h>

#define DCHECK_GT(a,b)
#define DCHECK_EQ(a,b)
#define CHECK_GE(a, b)
#define CHECK_LT(a,b)
#define DCHECK(a)
#define CHECK(a)

#if defined (_MSC_VER)
    #pragma warning (disable : 4100)  //unreferenced formal parameter
    #pragma warning (disable : 4101)  //unreferenced local variable
    #pragma warning (disable : 4127)  //conditional expression is constant
    #pragma warning (disable : 4146)  //unary minus operator applied to unsigned type, result still unsigned
    #pragma warning (disable : 4189)  //local variable is initialized but not referenced
    #pragma warning (disable : 4244)  //'+=': conversion from 'const int32_t' to 'unsigned char', possible loss of data
    #pragma warning (disable : 4267)  //argument': conversion from 'size_t' to 'DWORD', possible loss of data
    #pragma warning (disable : 4305)  //'*=': truncation from 'int' to 'unsigned char'
    #pragma warning (disable : 4312)  //'<function-style-cast>': conversion from 'DWORD' to 'pid_t' of greater size
    #pragma warning (disable : 4316)  //object allocated on the heap may not be aligned 128
    #pragma warning (disable : 4324)  //structure was padded due to alignment specifier
    #pragma warning (disable : 4800)  //forcing value to bool
#endif

#define FOLLY_NO_CONFIG 
#define FOLLY_HAVE_STD__IS_TRIVIALLY_COPYABLE 1 

namespace folly {

using pthread_key_t = void*;
using pthread_t = std::thread::id;

inline pthread_t pthread_self() {return std::this_thread::get_id();}
inline void pthread_key_delete(pthread_key_t key) {BentleyApi::BeThreadLocalStorage::Delete(key);}
inline int pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) {*key = BentleyApi::BeThreadLocalStorage::Create(); return 0;}
inline void* pthread_getspecific(pthread_key_t key) {return BentleyApi::BeThreadLocalStorage::GetValue(key);}
inline int pthread_setspecific(pthread_key_t key, void* val) {BentleyApi::BeThreadLocalStorage::SetValue(key, val); return 0;}
inline int sched_yield() {std::this_thread::yield(); return 0;}

class NonCopyable{
  NonCopyable(NonCopyable const&) = delete;
  NonCopyable& operator=(NonCopyable const&) = delete;
protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
};
}

#ifndef FOLLY_NO_CONFIG
#include <folly/folly-config.h>
#endif

#ifdef FOLLY_PLATFORM_CONFIG
#include FOLLY_PLATFORM_CONFIG
#endif

#if FOLLY_HAVE_FEATURES_H
#include <features.h>
#endif

#ifdef __ANDROID__
#include <android/api-level.h>
#endif

#ifdef __APPLE__
#include <Availability.h>
#endif
