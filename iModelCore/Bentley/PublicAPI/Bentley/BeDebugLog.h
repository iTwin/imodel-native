/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeDebugLog.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

/** @file */

#include "Bentley.h"

BEGIN_BENTLEY_NAMESPACE
//! Functions to write to the platform-specific log.
struct BeDebugLogFunctions
{
//! Write \a message to the platform-specific log.
//! @remarks On Android, output goes to logcat. On iOS, output goes to syslog. On WinRT, OutputDebugString is called.
//! @param message      The message to write.
//! @param fileName     Additional information to log: the name of the file in which the logging event occurred.
//! @param lineNumber   Additional information to log: the line number in the file in which the logging event occurred.
BENTLEYDLL_EXPORT static void PerformBeDebugLog (Utf8CP message, Utf8CP fileName, unsigned lineNumber);
};
END_BENTLEY_NAMESPACE

#if defined (NDEBUG)
    //! compiled out for non-debug builds
    #define BeDebugLog(message)
#else
    //! Write \a message to the platform-specific log.
    //! @see BentleyApi::BeDebugLogFunctions::PerformBeDebugLog for arguments
    #define BeDebugLog(message) (BeDebugLogFunctions::PerformBeDebugLog (message, __FILE__, __LINE__))
#endif
