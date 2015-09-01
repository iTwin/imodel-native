//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ext/gdal/port/cpl_config.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

//This is a wrapper file that includes the gdal header for a visual studio compilation. We kept the original extension(.h.vc) to 
// ease future library update.
#if defined (ANDROID) || defined (__APPLE__)
#   include "cpl_config.h.an"
#elif defined (_WIN32)
#   include "cpl_config.h.vc"
#endif

