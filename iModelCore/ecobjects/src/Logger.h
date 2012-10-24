/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Logger.h $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_BENTLEY_LOGGING
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ECObjectsLogger
{
private:
    static ILogger* s_log;
    ECObjectsLogger(void) {};

public:
    static ILogger* Log();
};

END_BENTLEY_ECOBJECT_NAMESPACE
