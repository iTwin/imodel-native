/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Logger.h $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>

#if defined (ANDROID) && defined (NDK_BUILD)
    // symbolic links are not used with NDK_BUILD
    #include <bentleylogging.h>
#else
    #include <Logging/bentleylogging.h>
#endif

USING_NAMESPACE_BENTLEY_LOGGING
BEGIN_BENTLEY_EC_NAMESPACE

struct ECObjectsLogger
{
private:
    static ILogger* s_log;
    ECObjectsLogger(void) {};

public:
    static ILogger* Log();
};

END_BENTLEY_EC_NAMESPACE
