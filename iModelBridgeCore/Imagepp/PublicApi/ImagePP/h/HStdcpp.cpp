//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HStdcpp.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// hstdcpp.cpp : source file that includes just the standard includes
// Pre-compiled header
// hstdcpp.obj will contain the pre-compiled type information
#include <ImagePP/h/hstdcpp.h>

#if defined (ANDROID) || defined (__APPLE__)

#elif defined (_WIN32)
    // VC8 support
    // link warning LNK4221
    // obj file must by have a public member
    uint32_t HStdcppDummy=0;
#else
#   error Unknown compiler 
#endif






