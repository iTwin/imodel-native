/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BentleyConfig.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// BENTLEYCONFIG_JAVASCRIPT is defined by the various platform-specific ToolContext.mke files

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
    #endif

#elif defined (ANDROID)

    #define BENTLEYCONFIG_OS_ANDROID
    #define BENTLEYCONFIG_OS_UNIX
    #define BENTLEYCONFIG_GRAPHICS_OPENGLES
    #define BENTLEYCONFIG_GRAPHICS_OPENGL

#elif defined (_WIN32)

    // BENTLEYCONFIG_OS_WINDOWS means all flavors of Windows (x64, x86, WinRTx86, WinRTx64)
    #define BENTLEYCONFIG_OS_WINDOWS
    #define BENTLEYCONFIG_GRAPHICS_DIRECTX
    #define BENTLEYCONFIG_GRAPHICS_HAVE_BACKING_STORE

    //  BENTLEYCONFIG_SUPPORTS_SYSTEM_MOUSE is defined if the system supports a real mouse
    //  and DgnView is known to support the mouse properly on the system.
    #define BENTLEYCONFIG_SUPPORTS_SYSTEM_MOUSE

    #if defined (BENTLEY_WINRT)
        // BENTLEYCONFIG_OS_WINRT will only be set for WinRTx86 and WinRTx64 (Store/Metro apps)
        #define BENTLEYCONFIG_OS_WINRT
    #else
        #define BENTLEYCONFIG_GRAPHICS_SUPPORT_QV_THREAD
        #define BENTLEYCONFIG_SUPPORT_PRELOADING_DISK_CACHE
        #define BENTLEYCONFIG_PARASOLIDS
    #endif

#elif defined (__linux)

    #define BENTLEYCONFIG_OS_UNIX
    #define BENTLEYCONFIG_GRAPHICS_OPENGL

#else
    #error Unexpected OS configuration
#endif


#ifdef NO_STD_REGEX
    #define BENTLEYCONFIG_CPP_MISSING_STD_REGEX
#endif

#ifdef BENTLEY_CPP_MISSING_WCHAR_SUPPORT
    #define BENTLEY_CONFIG_CPP_MISSING_WCHAR_SUPPORT
#endif
