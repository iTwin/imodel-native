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

#include <string.h>
#include <stdlib.h>

#include <folly/portability/Config.h>

#if !FOLLY_HAVE_MEMRCHR
#ifdef _WIN32
// MSVC CRT and clang-cl on Windows don't provide memrchr; supply an inline fallback.
inline void* memrchr(const void* s, int c, size_t n) {
    if (n == 0) return nullptr;
    const unsigned char* p = static_cast<const unsigned char*>(s) + n;
    while (n--) {
        if (*--p == static_cast<unsigned char>(c)) return const_cast<void*>(static_cast<const void*>(p));
    }
    return nullptr;
}
#else
extern "C" void* memrchr(const void* s, int c, size_t n);
#endif
#endif

#if defined(_WIN32) || defined(__FreeBSD__)
extern "C" char* strndup(const char* a, size_t len);
#endif

#ifdef _WIN32
extern "C" {
void bzero(void* s, size_t n);
int strcasecmp(const char* a, const char* b);
int strncasecmp(const char* a, const char* b, size_t c);
char* strtok_r(char* str, char const* delim, char** ctx);
}
#endif
