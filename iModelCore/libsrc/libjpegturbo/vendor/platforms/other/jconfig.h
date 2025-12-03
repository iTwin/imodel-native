/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//This is a wrapper file that includes that define config for a visual studio compilation. We kept the original extension(.in) to 
// ease future library update.

// For now, all versions are using the same config.
#if defined(_WIN32)
    #include "jconfig.h.in"
#elif defined(ANDROID)
    #include "jconfig.h.in"
#elif defined (__linux)
    #include "jconfig.h.in"
#elif defined (__APPLE__)
    #include "jconfig.h.in"
#else
    #error libjpeg-turbo, platform config not implemented
#endif