/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BimFromDgnDb/BimFromDgnDb.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <functional>

/** @namespace BimFromDgnDb Contains types defined by %Bentley Systems that are used to upgrade < 2.0 iModels to 2.0 iModels. */
#define BIM_FROM_DGNDB_NAMESPACE_NAME BimFromDgnDb
#define BEGIN_BIM_FROM_DGNDB_NAMESPACE namespace BentleyM0200 { namespace Dgn { namespace BIM_FROM_DGNDB_NAMESPACE_NAME {
#define END_BIM_FROM_DGNDB_NAMESPACE   } } }

#if defined (__GNUC__) || defined (__clang__)
    #define EXPORT_ATTRIBUTE        __attribute__((visibility ("default")))
#elif defined (_WIN32) // Windows && WinRT
    #if !defined (CREATE_STATIC_LIBRARIES)
        #define EXPORT_ATTRIBUTE        __declspec(dllexport)
        #define IMPORT_ATTRIBUTE        __declspec(dllimport)
    #else
        #define EXPORT_ATTRIBUTE        __declspec(dllexport)
        #define IMPORT_ATTRIBUTE        __declspec(dllexport)
    #endif
#endif

#ifndef BIM_FROM_DGNDB_EXPORT
#if defined (__BIM_FROM_DGNDB_BUILD__)
#define BIM_FROM_DGNDB_EXPORT EXPORT_ATTRIBUTE
#endif
#endif

#if !defined (BIM_FROM_DGNDB_EXPORT)
#define BIM_FROM_DGNDB_EXPORT  IMPORT_ATTRIBUTE
#endif

enum BimFromDgnDbLoggingSeverity
    {
    LOG_FATAL = 0,      //!< Used for fatal errors that will terminate the application
    LOG_ERROR = (-1),   //!< Used for general errors
    LOG_WARNING = (-2), //!< Used for general warnings
    LOG_INFO = (-3),    //!< Used for general information
    LOG_DEBUG = (-4),   //!< Used for debugging information
    LOG_TRACE = (-5)    //!< Used for tracing function calls
    };

typedef std::function<void(BimFromDgnDbLoggingSeverity severity, const char* message)> T_LogGeneralMessage;
typedef std::function<void(const char* message)> T_LogPerformanceMessage;
typedef std::function<void(const char* json)> T_QueueJson;
typedef std::function<void()> T_Insert;
