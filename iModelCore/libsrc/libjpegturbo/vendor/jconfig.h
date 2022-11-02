/*--------------------------------------------------------------------------------------+
|
|     $Source: vendor/jconfig.h $
|    $RCSfile: HIERasterReference.cpp,v $
|   $Revision: 1.207 $
|       $Date: 2011/01/18 15:06:27 $
|     $Author: Marc.Bedard $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//This is a wrapper file that includes that define config for a visual studio compilation. We kept the original extension(.in) to 
// ease future library update.

// For now, all versions are using the same config.
#if defined(_WIN32)
    #include "win/jconfig.h.in"
#elif defined(ANDROID)
    #include "win/jconfig.h.in"
#elif defined (__linux)
    #include "win/jconfig.h.in"
#elif defined (__APPLE__)
    #include "win/jconfig.h.in"
#else
    #error libjpeg-turbo, platform config not implemented
#endif