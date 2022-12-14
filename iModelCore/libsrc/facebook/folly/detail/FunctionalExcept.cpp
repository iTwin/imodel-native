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

#include <folly/Portability.h>

// If FOLLY_HAVE_BITS_FUNCTEXCEPT_H is set, this file compiles to
// nothing.

#if !FOLLY_HAVE_BITS_FUNCTEXCEPT_H

#include <folly/detail/FunctionalExcept.h>

#include <stdexcept>

FOLLY_NAMESPACE_STD_BEGIN

// WIP_XCODE9 and NDK r18
#if !(defined (__clang__) && defined (__APPLE__) && ((__clang_major__ * 1000 + __clang_minor__ * 10) >= 9000)) && !(defined (__clang__) && defined (ANDROID) && ((__clang_major__ * 1000 + __clang_minor__ * 10) >= 7000))

void __throw_length_error(const char* msg) {
  throw std::length_error(msg);
}

void __throw_logic_error(const char* msg) {
  throw std::logic_error(msg);
}

void __throw_out_of_range(const char* msg) {
  throw std::out_of_range(msg);
}

#endif

#if defined(_MSC_VER)
void __throw_bad_alloc() {
  throw std::bad_alloc();
}
#endif

FOLLY_NAMESPACE_STD_END

#endif
