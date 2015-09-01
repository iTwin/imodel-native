//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HTypes.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
/**  htypes.h
**
**====================================================================*/
#pragma once

#include "HmrMacro.h"
#include "ImagePPErrors.r.h"
#include <float.h>

// Math constants
#ifndef PI
#   define PI (3.141592653589793238462643)
#endif

// The following is only applicable in C++
#if defined (ANDROID) || defined (__APPLE__)

#elif defined (_WIN32)
#   include <limits>
#endif

BEGIN_IMAGEPP_NAMESPACE

#ifdef _M_X64
typedef int64_t    HSINTX;
typedef uint64_t   HUINTX;
#   define HSINTX_MIN      _I64_MIN
#   define HSINTX_MAX      _I64_MAX
#   define HUINTX_MIN      0
#   define HUINTX_MAX      UINT64_MAX
#else
typedef int32_t   HSINTX;
typedef uint32_t  HUINTX;
#    define HSINTX_MIN      INT_MIN
#    define HSINTX_MAX      INT_MAX
#    define HUINTX_MIN      0
#    define HUINTX_MAX      UINT_MAX
#endif

// Type used by Persistent object an other object to implement GetClassID
typedef int32_t   HCLASS_ID;

END_IMAGEPP_NAMESPACE