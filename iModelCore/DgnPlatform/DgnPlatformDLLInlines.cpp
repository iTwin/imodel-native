/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnPlatformDLLInlines.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if !defined (NDEBUG) || defined (__GNUC__) || defined (__clang__) || defined(BENTLEY_WINRT)
    /*----------------------------------------------------------------------------------+
    |
    | !defined (NDEBUG) means we are producing a DEBUG build.  Don't want to inline in
    |   this case for debugging purposes.
    |
    | For GCC and clang, inline methods are not exported.  Always produce a 
    |   non-inlined implementation here for external (to DgnCore) callers to use.  
    |   Internal callers will get their implementation by way of "DgnPlatformInternal.h".
    |
    +----------------------------------------------------------------------------------*/
    #define __DGNCORE_DONT_INLINE__
    #include "DgnPlatformInternal.h"
    #include "DgnCore/DgnCoreDLLInlines.h"
    #undef  __DGNCORE_DONT_INLINE__
#endif
