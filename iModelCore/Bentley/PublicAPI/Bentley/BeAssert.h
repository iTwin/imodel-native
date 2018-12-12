/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeAssert.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

/** @file */

#include "Bentley.h"

#if !defined (DOCUMENTATION_GENERATOR)
BEGIN_BENTLEY_NAMESPACE
struct BeAssertFunctions
{
enum class AssertType
    {
    Normal    = 0,
    Data      = 1,
    Sigabrt   = 2,
    TypeCount = 3,
    All       = 99
    };
typedef void T_BeAssertHandler (wchar_t const* _Message, wchar_t const*_File, unsigned _Line, AssertType atype);
BENTLEYDLL_EXPORT static void SetBeTestAssertHandler (T_BeAssertHandler*);
BENTLEYDLL_EXPORT static void SetBeAssertHandler (T_BeAssertHandler*);
BENTLEYDLL_EXPORT static void PerformBeAssert(wchar_t const* _Message, wchar_t const*_File, unsigned _Line);
BENTLEYDLL_EXPORT static void PerformBeDataAssert(wchar_t const* _Message, wchar_t const*_File, unsigned _Line);
BENTLEYDLL_EXPORT static void DefaultAssertionFailureHandler (wchar_t const* message, wchar_t const* file, unsigned line);
};
END_BENTLEY_NAMESPACE
#endif // DOCUMENTATION_GENERATOR

#include <assert.h>

#ifdef  NDEBUG

//! Assert that \a _Expression is true
#define BeAssert(_Expression)     ((void)0)
//! Assert that \a _Expression is true, when the test relates to input data.
#define BeDataAssert(_Expression)     ((void)0)
//! Assert that \a _Expression is true
#define BeAssertOnce(_Expression)     ((void)0)
//! Assert that \a _Expression is true, when the test relates to input data.
#define BeDataAssertOnce(_Expression)     ((void)0)

#else
//! Assert that \a _Expression is true
#define BeAssert(_Expression) (void)( (!!(_Expression)) || (BentleyApi::BeAssertFunctions::PerformBeAssert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )
//! Assert that \a _Expression is true, when the test relates to input data.
#define BeDataAssert(_Expression) (void)( (!!(_Expression)) || (BentleyApi::BeAssertFunctions::PerformBeDataAssert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )
//! Assert that \a _Expression is true
#define BeAssertOnce(_Expression)               \
        do {                                    \
            static int st_bAssertedOnce = 0;    \
            if (!st_bAssertedOnce)   \
                {                               \
                (!!(_Expression)) || (BentleyApi::BeAssertFunctions::PerformBeAssert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0);         \
                st_bAssertedOnce = 1;           \
                }                               \
            } while (0)
//! Assert that \a _Expression is true, when the test relates to input data.
#define BeDataAssertOnce(_Expression)           \
        do {                                    \
            static int st_bAssertedOnce = 0;    \
            if (!st_bAssertedOnce)              \
                {                               \
                (!!(_Expression)) || (BentleyApi::BeAssertFunctions::PerformBeDataAssert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0);     \
                st_bAssertedOnce = 1;           \
                }                               \
            } while (0)


#endif  /* NDEBUG */
