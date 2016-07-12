/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BentleyConfig.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// BENTLEYCONFIG_NO_JAVASCRIPT is defined by the various platform-specific ToolContext.mke files

#ifndef BENTLEYCONFIG_NO_JAVASCRIPT
    #define BENTLEYCONFIG_JAVASCRIPT
#endif

//---------------------------------------------------------------------------------------
//  operating system configuration
//---------------------------------------------------------------------------------------
#if defined (__APPLE__)

    #define BENTLEYCONFIG_OS_APPLE
    #define BENTLEYCONFIG_OS_UNIX

    #include <TargetConditionals.h>
    #if defined (TARGET_OS_IPHONE) || defined (TARGET_IPHONE_SIMULATOR)
        #define BENTLEYCONFIG_OS_APPLE_IOS
        #define BENTLEYCONFIG_GRAPHICS_OPENGLES
        #define BENTLEYCONFIG_GRAPHICS_OPENGL
    #else
        #define BENTLEYCONFIG_OS_APPLE_MACOS
        #define BENTLEYCONFIG_GRAPHICS_OPENGL
        #define BENTLEYCONFIG_VIRTUAL_MEMORY   //  Assume the OS swapping is better than anything we would do
        #define BENTLEYCONFIG_OPENCASCADE
    #endif

    #define BENTLEYCONFIG_DISPLAY_APPLE


#elif defined (ANDROID)

    #define BENTLEYCONFIG_OS_ANDROID
    #define BENTLEYCONFIG_OS_UNIX
    #define BENTLEYCONFIG_GRAPHICS_OPENGLES
    #define BENTLEYCONFIG_GRAPHICS_OPENGL
    #define BENTLEYCONFIG_DISPLAY_ANDROID
    // *** WIP_ANDROID_CLANG -- still working on etting OpenCASCADE to build using clang
    // #define BENTLEYCONFIG_OPENCASCADE

#elif defined (_WIN32)

    // BENTLEYCONFIG_OS_WINDOWS means all flavors of Windows (x64, x86, WinRTx86, WinRTx64)
    #define BENTLEYCONFIG_OS_WINDOWS
    #define BENTLEYCONFIG_GRAPHICS_DIRECTX
    #define BENTLEYCONFIG_GRAPHICS_HAVE_BACKING_STORE

    //  BENTLEYCONFIG_SUPPORTS_SYSTEM_MOUSE is defined if the system supports a real mouse
    //  and DgnView is known to support the mouse properly on the system.
    #define BENTLEYCONFIG_SUPPORTS_SYSTEM_MOUSE

    #define BENTLEYCONFIG_VIRTUAL_MEMORY 
    
    #if defined (BENTLEY_WINRT)
        // BENTLEYCONFIG_OS_WINRT will only be set for WinRTx86 and WinRTx64 (Store/Metro apps)
        #define BENTLEYCONFIG_OS_WINRT
        #define BENTLEYCONFIG_DISPLAY_METRO
    #else
        #define BENTLEYCONFIG_GRAPHICS_SUPPORT_QV_THREAD
        #define BENTLEYCONFIG_SUPPORT_PRELOADING_DISK_CACHE
//        #define BENTLEYCONFIG_PARASOLIDS
        #define BENTLEYCONFIG_OPENCASCADE
        #define BENTLEYCONFIG_DISPLAY_WIN32
    #endif

#elif defined (__linux)

    #define BENTLEYCONFIG_OS_UNIX
    #define BENTLEYCONFIG_GRAPHICS_OPENGL
    #define BENTLEYCONFIG_DISPLAY_WX
    #define BENTLEYCONFIG_VIRTUAL_MEMORY

#elif defined (__EMSCRIPTEN__)

#else
    #error Unexpected OS configuration
#endif

#if defined (_M_X64) || defined(__LP64__)
    #define BENTLEYCONFIG_64BIT_HARDWARE
#endif

#ifdef NO_STD_REGEX
    #define BENTLEYCONFIG_CPP_MISSING_STD_REGEX
#endif

#ifdef BENTLEY_CPP_MISSING_WCHAR_SUPPORT
    #define BENTLEY_CONFIG_CPP_MISSING_WCHAR_SUPPORT
#endif
