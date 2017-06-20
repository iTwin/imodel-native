/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BimTeleporter/BimTeleporter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <functional>

//__PUBLISH_SECTION_START__

/** @namespace BimTeleporter Contains types defined by %Bentley Systems that are used to synchronize between DgnDb and foreign data formats. */
#define BIM_TELEPORTER_NAMESPACE_NAME BimTeleporter
#define BEGIN_BIM_TELEPORTER_NAMESPACE namespace BentleyB0200 { namespace Dgn { namespace BIM_TELEPORTER_NAMESPACE_NAME {
#define END_BIM_TELEPORTER_NAMESPACE   } } }

//__PUBLISH_SECTION_END__

#ifndef BIM_TELEPORTER_EXPORT
#if defined (__BIM_TELEPORTER_BUILD__)
#define BIM_TELEPORTER_EXPORT EXPORT_ATTRIBUTE
#endif
#endif

//__PUBLISH_SECTION_START__

#if !defined (BIM_TELEPORTER_EXPORT)
#define BIM_TELEPORTER_EXPORT  IMPORT_ATTRIBUTE
#endif

#define BIM_TELEPORTER_TYPEDEFS(_structname_) \
    BEGIN_BIM_TELEPORTER_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BIM_TELEPORTER_NAMESPACE

#define BIM_TELEPORTER_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BIM_TELEPORTER_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BIM_TELEPORTER_NAMESPACE 

enum TeleporterLoggingSeverity
    {
    LOG_FATAL = 0,      //!< Used for fatal errors that will terminate the application
    LOG_ERROR = (-1),   //!< Used for general errors
    LOG_WARNING = (-2), //!< Used for general warnings
    LOG_INFO = (-3),    //!< Used for general information
    LOG_DEBUG = (-4),   //!< Used for debugging information
    LOG_TRACE = (-5)    //!< Used for tracing function calls
    };

typedef std::function<void(TeleporterLoggingSeverity severity, const char* message)> T_LogGeneralMessage;
typedef std::function<void(const char* message)> T_LogPerformanceMessage;
typedef std::function<void(const char* json)> T_QueueJson;
