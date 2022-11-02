/*---------------------------------------------------------------------------------------------
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|  See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#if defined (ANDROID)
    #define SIZEOF_CURL_OFF_T 8 // copied from config-win32.h, seems reasonable for 64-bit
    #include "vendor/lib/config-bentleyopenssl.h"
    #include "vendor/lib/config-android.h"
#elif defined (__APPLE__)
    #include <TargetConditionals.h>
    #define USE_SECTRANSP 1
    #if !defined (TARGET_OS_IPHONE) || 1 != TARGET_OS_IPHONE
        // MacOS
        #define SIZEOF_CURL_OFF_T 8 // copied from config-win32.h, seems reasonable for 64-bit
        #define SIZEOF_LONG 8 // copied from config-bentleylinux.h, seems reasonable for 64-bit
        
        #include "vendor/lib/config-mac.h"
        
        #undef HAVE_EXTRA_STRICMP_H
        #undef HAVE_EXTRA_STRDUP_H
        #undef HAVE_STRUCT_POLLFD
        #define HAVE_LONGLONG 1
        #undef SEND_TYPE_ARG3
        #define SEND_TYPE_ARG3 size_t
        #define HAVE_SYS_IOCTL_H 1
        #define HAVE_STRDUP 1
        #define ENABLE_IPV6 1
        #define HAVE_GETADDRINFO 1
        #define HAVE_BOOL_T 1
        #define HAVE_STDBOOL_H 1
    #else
        // iOS
        #include "vendor/lib/config-ios.h"
    #endif
#elif defined (__linux)
    #define SIZEOF_CURL_OFF_T 8 // copied from config-win32.h, seems reasonable for 64-bit
    #include "vendor/lib/config-bentleyopenssl.h"
    #include "vendor/lib/config-bentleylinux.h"
#elif defined (BENTLEY_WINRT)
    #include "vendor/lib/config-bentleyopenssl.h"
    #include "vendor/lib/config-bentleywinrt.h"
#elif defined (BENTLEY_WIN32)
    // Ignore certificate revocation check errors that are skipped in most software as well
    #define BENTLEY_SOFT_REVOCATION_CHECKS 1
    #include "vendor/lib/config-bentleywindows.h"
#else
    #include "vendor/lib/config-bentleyopenssl.h"
#endif

// Enable IPV6 support for all platforms
#define ENABLE_IPV6 1
