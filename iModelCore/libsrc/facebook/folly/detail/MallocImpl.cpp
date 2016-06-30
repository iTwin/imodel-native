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

#include <folly/detail/Malloc.h>

extern "C" {

void* (*mallocx)(size_t, int) = nullptr;
void* (*rallocx)(void*, size_t, int) = nullptr;
size_t (*xallocx)(void*, size_t, size_t, int) = nullptr;
size_t (*sallocx)(const void*, int) = nullptr;
void (*dallocx)(void*, int) = nullptr;
void (*sdallocx)(void*, size_t, int) = nullptr;
size_t (*nallocx)(size_t, int) = nullptr;
int (*mallctl)(const char*, void*, size_t*, void*, size_t) = nullptr;
int (*mallctlnametomib)(const char*, size_t*, size_t*) = nullptr;
int (*mallctlbymib)(const size_t*, size_t, void*, size_t*, void*, size_t) =
    nullptr;

#if defined (BENTLEY_CHANGE)
#ifdef _MSC_VER
// MSVC doesn't have weak symbols, so do some linker magic
// to emulate them.
const char* mallocxWeak = nullptr;
#pragma comment(linker, "/alternatename:_mallocx=_mallocxWeak")
const char* rallocxWeak = nullptr;
#pragma comment(linker, "/alternatename:rallocx=rallocxWeak")
const char* xallocxWeak = nullptr;
#pragma comment(linker, "/alternatename:xallocx=xallocxWeak")
const char* sallocxWeak = nullptr;
#pragma comment(linker, "/alternatename:sallocx=sallocxWeak")
const char* dallocxWeak = nullptr;
#pragma comment(linker, "/alternatename:dallocx=dallocxWeak")
const char* sdallocxWeak = nullptr;
#pragma comment(linker, "/alternatename:sdallocx=sdallocxWeak")
const char* nallocxWeak = nullptr;
#pragma comment(linker, "/alternatename:nallocx=nallocxWeak")
const char* mallctlWeak = nullptr;
#pragma comment(linker, "/alternatename:mallctl=mallctlWeak")
const char* mallctlnametomibWeak = nullptr;
#pragma comment(linker, "/alternatename:mallctlnametomib=mallctlnametomibWeak")
const char* mallctlbymibWeak = nullptr;
#pragma comment(linker, "/alternatename:mallctlbymib=mallctlbymibWeak")
#elif !FOLLY_HAVE_WEAK_SYMBOLS
void* (*mallocx)(size_t, int) = nullptr;
void* (*rallocx)(void*, size_t, int) = nullptr;
size_t (*xallocx)(void*, size_t, size_t, int) = nullptr;
size_t (*sallocx)(const void*, int) = nullptr;
void (*dallocx)(void*, int) = nullptr;
void (*sdallocx)(void*, size_t, int) = nullptr;
size_t (*nallocx)(size_t, int) = nullptr;
int (*mallctl)(const char*, void*, size_t*, void*, size_t) = nullptr;
int (*mallctlnametomib)(const char*, size_t*, size_t*) = nullptr;
int (*mallctlbymib)(const size_t*, size_t, void*, size_t*, void*, size_t) =
    nullptr;
#endif
#endif

}
