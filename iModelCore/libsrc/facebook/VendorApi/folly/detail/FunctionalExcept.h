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

#include <folly/Portability.h>

#if !FOLLY_HAVE_BITS_FUNCTEXCEPT_H

FOLLY_NAMESPACE_STD_BEGIN

// WIP newer clangs
#if !(defined(__clang__) && defined(__APPLE__) && ((__clang_major__ * 1000 + __clang_minor__ * 10) >= 9000)) && !(defined(__clang__) && defined(__linux__) && ((__clang_major__ * 1000 + __clang_minor__ * 10) >= 3080))

[[noreturn]] BE_FOLLY_EXPORT void __throw_length_error(const char* msg);
[[noreturn]] BE_FOLLY_EXPORT void __throw_logic_error(const char* msg);
[[noreturn]] BE_FOLLY_EXPORT void __throw_out_of_range(const char* msg);

#endif

#ifdef _MSC_VER
[[noreturn]] BE_FOLLY_EXPORT void __throw_bad_alloc();
#endif

FOLLY_NAMESPACE_STD_END

#else
#error This file should never be included if FOLLY_HAVE_BITS_FUNCTEXCEPT_H is set
#endif
