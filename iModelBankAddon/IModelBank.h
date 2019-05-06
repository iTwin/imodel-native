/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <BeSQLite/BeSQLite.h>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

namespace IModelBank
{
//=======================================================================================
// @bsistruct                                   Sam.Wilson                  06/18
//=======================================================================================
struct JsInterop
{
    static bool IsMainThread();
    static bool IsJsExecutionDisabled();

    static void ThrowJsException(Utf8CP msg);

    static void InitLogging();
    static NativeLogging::ILogger &GetNativeLogger();
    static void LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg);
    static bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev);
};

} // namespace IModelBank
