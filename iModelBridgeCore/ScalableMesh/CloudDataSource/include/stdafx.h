#pragma once

#define NOMINMAX

#ifdef _WIN32
    #include <Windows.h>
#endif

#include <curl/curl.h>
#include <openssl/crypto.h>

#include <algorithm>

#ifndef NDEBUG

#ifdef _WIN32
    #include <windows.h>
    const DWORD MS_VC_EXCEPTION = 0x406D1388;

    #pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
        {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
        } THREADNAME_INFO;
    #pragma pack(pop)

void SetThreadName(DWORD dwThreadID, char const* threadName);
#endif

#endif
