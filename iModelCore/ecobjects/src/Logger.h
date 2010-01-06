/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/Logger.h $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects\ECObjects.h>
#include <BsiLogging\bsilog.h>

USING_NAMESPACE_BENTLEY_LOGGING
BEGIN_BENTLEY_EC_NAMESPACE

struct Logger
{
private:
    static Bentley::NativeLogging::ILogger *        s_logger;

public:
    static Bentley::NativeLogging::ILogger * GetLogger(void);
    
private:
    Logger(void) {};
};

END_BENTLEY_EC_NAMESPACE
