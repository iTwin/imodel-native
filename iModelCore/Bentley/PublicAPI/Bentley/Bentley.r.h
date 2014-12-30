/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Bentley.r.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*--------------------------------------------------------------------------------------
* This file contains ONLY typedefs and #defines specified by the Bentley Programming Style Guide
* and are used by .cpp, .r, and .mt files.
+--------------------------------------------------------------------------------------*/

//__PUBLISH_SECTION_START__

#if defined (mdl_resource_compiler) || defined (mdl_type_resource_generator)

    #define BEGIN_BENTLEY_NAMESPACE
    #define END_BENTLEY_NAMESPACE
    #define ALIGNMENT_ATTRIBUTE(B)
    #define ENUM_UNDERLYING_TYPE(T)

#endif // (mdl_resource_compiler) || defined (mdl_type_resource_generator)

#if !defined (NO_BENTLEY_STDINT_TYPEDEDFS)

    //  Use standard integer types
    #if defined (mdl_resource_compiler) || defined (mdl_type_resource_generator)
        // We must define the types for use by the resource compiler
    #elif defined (_WIN32) // Windows && WinRT
        #if (_MSC_VER >= 1600)
            //  Microsoft started delivering stdint.h as of VS2010
            #define HAVE_STDINT
            #if (_MSC_VER >= 1600)
                //  Without this definition of INT64_MAX etc. generate an error if intsafe.h was previously included.
                //  See http://connect.microsoft.com/VisualStudio/feedback/details/621653/including-stdint-after-intsafe-generates-warnings#
                #undef INT8_MIN
                #undef INT16_MIN
                #undef INT32_MIN
                #undef INT8_MAX
                #undef INT16_MAX
                #undef INT32_MAX
                #undef UINT8_MAX
                #undef UINT16_MAX
                #undef UINT32_MAX
                #undef INT64_MIN
                #undef INT64_MAX
                #undef UINT64_MAX
            #endif
        #endif
        // WIP_ALIGN - in x86 build, we can't pass an aligned type by value to a function -- causes error C2718: 'const T_Adouble': actual parameter with __declspec(align('8')) won't be aligned
        //#if !defined (_MANAGED)
        //    #define ALIGNMENT_ATTRIBUTE(B)  __declspec(align(B))
        //#else
            #define ALIGNMENT_ATTRIBUTE(B)
        //#endif
    #elif defined (__clang__) || defined (__GNUC__)
        #define ALIGNMENT_ATTRIBUTE(B)  __attribute__((aligned(B)))
        //  GCC always delivers stdint.h.
        #define HAVE_STDINT
    #else
        #error unknown compiler
    #endif

    #ifdef HAVE_STDINT
        #include <stdint.h>

        #if defined (ANDROID)
            // This define is only required on Android/GCC to get platform-independent printf macros such as PRI64d.
            #define __STDC_FORMAT_MACROS
        #endif
        #include <inttypes.h>
    #else
        typedef signed char         int8_t;
        typedef unsigned char       uint8_t;
        typedef short               int16_t;
        typedef unsigned short      uint16_t;
        typedef int                 int32_t;
        typedef unsigned int        uint32_t;
        typedef long long           int64_t;
        typedef unsigned long long  uint64_t;
    #endif

#endif

#if !defined (NO_BENTLEY_BASICTYPES)
// We no longer provide the following typedefs. Some conflict on various platforms, rendering them useless. Use the new C++11 intrinsic types in their place.
// Because we have generally mapped our typedefs to the C++11 intrinsics anyway, interop should still be trivial.
//
//  Int8    > int8_t
//  UInt8   > uint8_t
//  Int16   > int16_t
//  UInt16  > uint16_t
//  Int32   > int32_t
//  UInt32  > uint32_t
//  Int64   > int64_t
//  UInt64  > uint64_t
//
// We have removed these typedefs in favor of using their underlying types directly.
// Note that in many cases you should use the explicit-sized C++11 types, especially when persisting data and using any string formatting functions.
//
//  UShort  > unsigned short
//  ULong   > unsigned long
//  Short   > short
//  UInt    > unsigned int
//  UChar   > unsigned char
//
// We have kept this typedef for readability reasons.
typedef unsigned char Byte;
//
// We have remapped 'byte' to 'Byte' to make sure people realize it is not an intrinsic type.
//  byte    > Byte
#endif

// NB: Do not use WChar/wchar_t to declare stored data.
//     Store strings as UTF-16 or UTF-8.
typedef uint16_t            Utf16Char;
typedef char                Utf8Char;

#if defined (__cplusplus)
#define ENUM_IS_FLAGS(ENUMTYPE) \
inline ENUMTYPE operator| (ENUMTYPE a, ENUMTYPE b) { return static_cast<ENUMTYPE>(static_cast<int>(a) | static_cast<int>(b)); } \
inline ENUMTYPE operator& (ENUMTYPE a, ENUMTYPE b) { return static_cast<ENUMTYPE>(static_cast<int>(a) & static_cast<int>(b)); } \
inline ENUMTYPE operator~ (ENUMTYPE a) { return static_cast<ENUMTYPE>(~(static_cast<int>(a))); }
#else
#define ENUM_IS_FLAGS (ENUMTYPE)
#endif
typedef ALIGNMENT_ATTRIBUTE(8) double   T_Adouble;
typedef ALIGNMENT_ATTRIBUTE(8) uint64_t T_AUInt64;
typedef ALIGNMENT_ATTRIBUTE(8)  int64_t T_AInt64;
