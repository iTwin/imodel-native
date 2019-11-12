//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

//This is a wrapper file that includes the gdal header for a visual studio compilation. We kept the original extension(.h.vc) to 
// ease future library update.
#if defined (ANDROID) || defined (__APPLE__)
#error Need a proper GDAL cpl_config.h for this platform.
//#   include "cpl_config.h.an"
#elif defined (_WIN32)
#   include "cpl_config.h.vc"
#endif

