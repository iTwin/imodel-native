/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Bentley.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if !defined (__cplusplus)
#error "This file is for C++ compilands only"
#endif

#if defined (__GNUC__) || defined (__clang__)
    //////////////////////////// GNUC /////////////////////////////////////////////
    #include <stddef.h>     // for size_t


    #if defined (__GNUC__) && !defined (__clang__)
        #define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__ * 10)
        #define BENTLEY_CPLUSPLUS 201103L
    #elif defined (__clang__)
        #define CLANG_VERSION (__clang_major__ * 1000 + __clang_minor__ * 10)
        #if (CLANG_VERSION < 4010)
            #error upgrade to XCode version 4.5
        #endif
        #define BENTLEY_CPLUSPLUS 201103L
    #else
        #error unknown compiler
    #endif

    #define _countof(_Array) (sizeof(_Array)/sizeof((_Array)[0]))
    #define _CRT_WIDE(A) L"" A

    extern void _wassert (wchar_t const*, wchar_t const*, int);

    #define _alloca(x)              __builtin_alloca((x))
    #define EXPORT_VTABLE_ATTRIBUTE __attribute__((visibility ("default")))
    #define EXPORT_ATTRIBUTE        __attribute__((visibility ("default")))
    #define IMPORT_ATTRIBUTE        __attribute__((visibility ("default")))
    #define DLLLOCAL_ATTRIBUTE      __attribute__((visibility ("hidden")))
    #define CDECL_ATTRIBUTE
    #define STDCALL_ATTRIBUTE
    #define THROW_SPECIFIER(X)      throw(X)

    #define STD_TR1 std

    #define ENUM_UNDERLYING_TYPE(T)   : T

    typedef void* ULONG_PTR;

    #define UNREACHABLE_CODE(stmt)

    #define DECLARE_KEY_METHOD __attribute__((visibility ("default"))) virtual void DummyKeyMethod() const;

#elif defined (_WIN32) // Windows && WinRT

    //////////////////////////// MSVC /////////////////////////////////////////////

    #include "suppress_warnings.h"

    #if !defined (BENTLEY_WIN32) && !defined (BENTLEY_WINRT)
        #error Windows compile options not specified correctly. Use windowsToolContext.mke.
    #endif

    #if (_MSC_VER <= 1600) // INTPTR_MIN et al are defined in stdint.h. Microsoft started delivering stdint.h as of VS2010
        #define BENTLEY_CPLUSPLUS 199711L
    #else
        #define BENTLEY_CPLUSPLUS 201103L
    #endif

    #define HAVE_TR1
    #define STD_TR1 std::tr1

    #if !defined (CREATE_STATIC_LIBRARIES)
        #define EXPORT_ATTRIBUTE        __declspec(dllexport)
        #define IMPORT_ATTRIBUTE        __declspec(dllimport)
        #define DECLARE_KEY_METHOD      __declspec(dllexport) virtual void DummyKeyMethod() const;
    #else
        #define EXPORT_ATTRIBUTE        __declspec(dllexport)
        #define IMPORT_ATTRIBUTE        __declspec(dllexport)
        #define DECLARE_KEY_METHOD      __declspec(dllexport) virtual void DummyKeyMethod() const;
    #endif
    #define DLLLOCAL_ATTRIBUTE        /* For MSVC, a symbol is not visible outside of the DLL unless it is marked as EXPORT_ATTRIBUTE. */
    #define EXPORT_VTABLE_ATTRIBUTE   /* This is a GCC concept. In MSVC, it appears that the vtable and typeinfo of a class are always exported if the class contains any exported methods. */
    #define CDECL_ATTRIBUTE         __cdecl
    #define STDCALL_ATTRIBUTE       __stdcall
    #define ENUM_UNDERLYING_TYPE(T)   : T
    #define THROW_SPECIFIER(X)

    #if (_MSC_VER < 1600) // INTPTR_MIN et al are defined in stdint.h. Microsoft started delivering stdint.h as of VS2010
        #include <limits.h>
        #if defined(_M_X64)
            #define INTPTR_MIN      _I64_MIN
            #define INTPTR_MAX      _I64_MAX
            #define UINTPTR_MAX     _UI64_MAX
        #else /* !_M_X64 */
            #define INTPTR_MIN      _I32_MIN
            #define INTPTR_MAX      _I32_MAX
            #define UINTPTR_MAX     _UI32_MAX
        #endif
    #endif

    // The following is *NOT* equivalent to uintptr_t
    #if defined(_M_X64)
        typedef unsigned __int64    ULONG_PTR;
    #else /* !_M_X64 */
        typedef __w64 unsigned long  ULONG_PTR;
    #endif // WIN64


#else
    #error unknown compiler
#endif

// Macros to help define "key methods"
#define DEFINE_KEY_METHOD(CLS)   void    CLS::DummyKeyMethod() const {;}

#include "BentleyConfig.h"

// Bentley.r.h includes stdint.h (or does the equivalent).
#include "Bentley.r.h"

// Define standard macros and types for integer limits
#include <limits.h>

// We define these alias macros to clarify what symbol to use for a given fixed-size type. For example, if you use Int32, use INT32_MIN/MAX.
#ifndef INT32_MAX
#define INT32_MAX   INT_MAX
#endif
#ifndef UINT32_MAX
#define UINT32_MAX  UINT_MAX
#endif
#ifndef INT64_MAX
#define INT64_MAX   LLONG_MAX
#endif
#ifndef UINT64_MAX
#define UINT64_MAX  ULLONG_MAX
#endif
#ifndef INT32_MIN
#define INT32_MIN   INT_MIN
#endif
#ifndef INT64_MIN
#define INT64_MIN   LLONG_MIN
#endif

/** @namespace Bentley Contains types defined by %Bentley Systems. */
#if defined (DOCUMENTATION_GENERATOR)
    // Hold the namespace name constant from an SDK documentation perspective
    #define BENTLEY_NAMESPACE_NAME BentleyApi
#else
    // The actual version-specific namespace name from a code/type perspective
    #define BENTLEY_NAMESPACE_NAME BentleyG06
#endif

#define BEGIN_BENTLEY_NAMESPACE namespace BENTLEY_NAMESPACE_NAME {
#define END_BENTLEY_NAMESPACE   }
#define USING_NAMESPACE_BENTLEY using namespace BENTLEY_NAMESPACE_NAME;

// create the Bentley namespace
BEGIN_BENTLEY_NAMESPACE
END_BENTLEY_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)
    // namespace alias that always refers to the current version.
    namespace BentleyApi = BENTLEY_NAMESPACE_NAME;
#endif

// use unnamed namespace instead of static in C++ source files.
#define BEGIN_UNNAMED_NAMESPACE namespace {
#define END_UNNAMED_NAMESPACE   }

// Define standard pointer types (P, CP, R, CR) in the current namespace for the specified struct
#define DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) \
    struct _structname_; \
    typedef _structname_* _structname_##P, &_structname_##R; \
    typedef _structname_ const* _structname_##CP; \
    typedef _structname_ const& _structname_##CR; 

// These macros should only be used for classes in the Bentley namespace (consider using DEFINE_POINTER_SUFFIX_TYPEDEFS instead)
#define ADD_BENTLEY_NAMESPACE_TYPEDEFS1(_namespace_,_sourceName_,_name_,structclass) \
    namespace BENTLEY_NAMESPACE_NAME {\
    typedef structclass _namespace_ :: _sourceName_*          _name_##P, &_name_##R;  \
    typedef structclass _namespace_ :: _sourceName_ const*    _name_##CP; \
    typedef structclass _namespace_ :: _sourceName_ const&    _name_##CR; }

#define ADD_BENTLEY_NAMESPACE_TYPEDEFS(_namespace_,_name_) ADD_BENTLEY_NAMESPACE_TYPEDEFS1(_namespace_,_name_,_name_,struct)
#define BENTLEY_NAMESPACE_TYPEDEF(t,tP)    namespace BENTLEY_NAMESPACE_NAME {struct t; typedef struct BENTLEY_NAMESPACE_NAME::t*   tP;}
#define BENTLEY_NAMESPACE_TYPEDEFS(_name_) BEGIN_BENTLEY_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_NAMESPACE
#define BENTLEY_REF_COUNTED_PTR(_sname_) namespace BENTLEY_NAMESPACE_NAME {struct _sname_; typedef RefCountedPtr<_sname_> _sname_##Ptr;}

#if !defined (NULL)
    #if (BENTLEY_CPLUSPLUS <= 199711L) || defined(_MANAGED)
        #define NULL    0
    #else
        // nullptr is part C++11
        #define NULL    nullptr
    #endif
#endif

#if !defined (NO_BENTLEY_PUBLIC)
#define Public
#endif

#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C              }
#define ALLOW_NULL_OUTPUT(var,out) _t_##var, &var(out?*out:_t_##var)
#define DEFINE_T_SUPER(B) private: typedef B T_Super; public:

// this can be used to quiet compiler warnings for variables only used in asserts
#define UNUSED_VARIABLE(x) (void)(x)
#define FREE_AND_CLEAR(ptr)     {if(ptr){free(ptr);ptr=nullptr;}}
#define DELETE_AND_CLEAR(ptr)   {if(ptr){delete (ptr);ptr=nullptr;}}
#define RELEASE_AND_CLEAR(ptr)  {if(ptr){(ptr)->Release();ptr=nullptr;}}

/*---------------------------------------------------------------------------------
 NOTE: This is here because the Windows header file WinGDI.h defines ERROR to 0, which is a disaster for
 Bentley APIs where 0 means SUCCESS.

 If you want to use Bentley header files and you need to use anything from wingdi.h, you MUST include
 that file BEFORE Bentley.h  (and of course be aware that Bentley.h will undef ERROR and you'll
 get the enum ERROR=0x8000 below).

 If you really need types from WinGDI.h, usually the easiest thing to do is to put:
\code
   #include <wtypes.h>
\endcode
 as the first line in your source file. */

#if !defined (_WINGDI_)
    #define NOGDI       // Do NOT let WinGDI.h define ERROR to 0 !!!!
#endif
/*---------------------------------------------------------------------------------*/

#ifdef  TRUE
    #undef TRUE
#endif
#ifdef  FALSE
    #undef FALSE
#endif
#ifdef  ERROR
    #undef ERROR
#endif
#ifdef  SUCCESS
    #undef SUCCESS
#endif

BEGIN_BENTLEY_NAMESPACE
enum BentleyTrueFalse
    {
    TRUE  = 1,
    FALSE = 0
    };

enum BentleyStatus
    {
    SUCCESS     = 0,
    BSISUCCESS  = 0,
    ERROR       = 0x8000,
    BSIERROR    = 0x8000,
    };

enum class BentleyCharEncoding
    {
    Locale = 0,
    Utf8   = 1
    };

typedef int                 StatusInt;      // zero always means SUCCESS
typedef wchar_t             WChar;          // Note: do not use this type to declare, read, or write a stored string. Strings must be stored as UTF-16 or UTF-8.
typedef wchar_t const*      WCharCP;
typedef wchar_t*            WCharP;
typedef char const*         CharCP;
typedef char*               CharP;
typedef Utf8Char*           Utf8P;
typedef Utf8Char const*     Utf8CP;
typedef Utf16Char*          Utf16P;
typedef Utf16Char const*    Utf16CP;

typedef void*               UserDataP;
typedef void const*         UserDataCP;
typedef void*               CallbackArgP;

END_BENTLEY_NAMESPACE

BENTLEY_NAMESPACE_TYPEDEFS (WString)
BENTLEY_NAMESPACE_TYPEDEFS (AString)
BENTLEY_NAMESPACE_TYPEDEFS (Utf8String)
BENTLEY_NAMESPACE_TYPEDEFS (BeFileName)

#if !defined (NO_USING_NAMESPACE_BENTLEY)
    USING_NAMESPACE_BENTLEY
#endif

#if defined (_MSC_VER)
    #if (_MSC_VER <= 1600)
        #error MSC 11 or newer required 
    #endif
#endif

#if !defined (FOR_EACH)
    #define FOR_EACH(VAR,COL) for (VAR : COL)
#endif                        

//__PUBLISH_SECTION_END__
#undef BENTLEYDLL_EXPORT
#ifdef __BENTLEYDLL_BUILD__
    #define BENTLEYDLL_EXPORT EXPORT_ATTRIBUTE
#else
//__PUBLISH_SECTION_START__
    #define BENTLEYDLL_EXPORT IMPORT_ATTRIBUTE
//__PUBLISH_SECTION_END__
#endif
