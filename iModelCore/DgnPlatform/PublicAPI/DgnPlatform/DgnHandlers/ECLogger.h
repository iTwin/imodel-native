/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ECLogger.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <Logging/bsilog.h>

USING_NAMESPACE_BENTLEY_LOGGING
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! Used for logging in the DgnEC logging category/namespace
struct ECLogger
    {
    static NativeLogging::DgnLogger&     Log ();
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
