/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Logging.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_LOGGING

struct DgnDbServerLogHelper
{
private:
    static NativeLogging::ILogger* logger;
    static void LogInternal(SEVERITY severity, const Utf8String methodName, float timeSpent, Utf8CP messageFormat, va_list args);
public:
    //=======================================================================================
    //@param  severity       The severity level of the log message
    //@param  methodName     The name of the method in which the log message was generated
    //@param  time           The time it took for the method to execute
    //@param  messageFormat  Format for the message to log
    //@param  ...            Arguments for the specified format
    //@bsimethod                                      Vytautas.Strimaitis           08/2016
    //=======================================================================================
    static void Log(SEVERITY severity, const Utf8String methodName, float timeSpent, Utf8CP messageFormat, ...);

    //=======================================================================================
    //@bsimethod                                      Vytautas.Strimaitis           08/2016
    //=======================================================================================
    static void Log(SEVERITY severity, const Utf8String methodName, Utf8CP message, ...);
};
END_BENTLEY_DGNDBSERVER_NAMESPACE