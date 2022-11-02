// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#pragma once

/// If defined, indicates that Json use exception to report invalid type manipulation
/// instead of C assert macro.
# define JSON_USE_EXCEPTION 1

# if defined(JSON_DLL_BUILD)
#  define JSON_API __declspec(dllexport)
# elif defined(JSON_DLL)
#  define JSON_API __declspec(dllimport)
# else
#  define JSON_API
# endif

// If JSON_NO_INT64 is defined, then Json only support C++ "int" type for integer
// Storages, and 64 bits integer support is disabled.
// #define JSON_NO_INT64 1

#if defined(_MSC_VER)  &&  _MSC_VER <= 1200 // MSVC 6
// Microsoft Visual Studio 6 only support conversion from __int64 to double
// (no conversion from unsigned __int64).
#define JSON_USE_INT64_DOUBLE_CONVERSION 1
#endif // if defined(_MSC_VER)  &&  _MSC_VER < 1200 // MSVC 6

#if defined(_MSC_VER)  &&  _MSC_VER >= 1500 // MSVC 2008
/// Indicates that the following function is deprecated.
# define JSONCPP_DEPRECATED(message) __declspec(deprecated(message))
#endif

#if !defined(JSONCPP_DEPRECATED)
# define JSONCPP_DEPRECATED(message)
#endif // if !defined(JSONCPP_DEPRECATED)

#if !defined(NO_BENTLEY_CHANGES)
   #include <stdint.h>
#endif

#include <Bentley/Bentley.h>

BEGIN_BENTLEY_NAMESPACE
namespace Json {
   typedef int Int;
   typedef unsigned int UInt;
   typedef int64_t Int64;
   typedef uint64_t UInt64;
   typedef Int64 LargestInt;
   typedef UInt64 LargestUInt;
#  define JSON_HAS_INT64

// BeJsonCpp change
#if defined (__APPLE__) && !defined (__LP64__)
    // see MacTypes.h for conflicting definition of UInt32
    typedef unsigned long UInt32;
#else
    typedef UInt UInt32;
#endif

} // end namespace Json

END_BENTLEY_NAMESPACE

