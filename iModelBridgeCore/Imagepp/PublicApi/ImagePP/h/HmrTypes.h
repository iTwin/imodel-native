//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HmrTypes.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//  This header file defines all HMR defined types
#pragma once

#include <stdio.h>
#include <string.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#include <stdlib.h>

#include <limits.h>

// The following is only applicable in C++
#if defined (__cplusplus)
#   if defined (ANDROID) || defined (__APPLE__)

#   elif defined (_WIN32)
#       include <limits>
#   endif
#endif
#include <float.h>

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

/*
** Type used by Persistent object an other object to implement
** GetClassID
*/
typedef int32_t   HCLASS_ID;

