//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicAPI/Vortex/ExportMacros.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

// Should have come from bentley.h but we do not want to depend on it for now.
//#include <Bentley/Bentley.h>
#if defined (__GNUC__) || defined (__clang__)
    #define EXPORT_ATTRIBUTE        __attribute__((visibility ("default")))
    #define IMPORT_ATTRIBUTE        __attribute__((visibility ("default")))

#elif defined (_WIN32) // Windows && WinRT
    #define EXPORT_ATTRIBUTE        __declspec(dllexport)
    #define IMPORT_ATTRIBUTE        __declspec(dllimport)
#else
    #error unknown compiler
#endif
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C              }

#   if defined (__PointoolsVortexAPI_BUILD__)
#       define VORTEX_EXPORT EXPORT_ATTRIBUTE
#   else
#       define VORTEX_EXPORT IMPORT_ATTRIBUTE
#   endif

#if defined(CREATE_STATIC_LIBRARIES)
    #undef VORTEX_EXPORT
    #define VORTEX_EXPORT
#endif




