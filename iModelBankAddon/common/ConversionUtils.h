/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include "IModelBank.h"
#include <BeSQLite/BeSQLite.h>
#include <Logging/bentleylogging.h>

namespace IModelBank
{
struct ConversionUtils
{
private:
    static void InitLogging();

public:
    static bool IsMainThread();
    static bool IsJsExecutionDisabled();

    static void ThrowJsException(Utf8CP msg);

    static void Initialize(Napi::Env *);

    static NativeLogging::ILogger &GetNativeLogger();
    static void LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg);
    static bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev);

    // Get a SQLite Statement column value as a JS value
    static Napi::Value GetValue(BeSQLite::Statement &stmt, int i, Napi::Env env);

    // Bind a JS value to a SQLite Statement parameter
    static BentleyStatus BindValue(BeSQLite::Statement &stmt, int i, Napi::Value value);

    template <typename STATUSTYPE>
    static Napi::Object CreateErrorObject0(STATUSTYPE errCode, Utf8CP msg, Napi::Env env)
    {
        Napi::Object error = Napi::Object::New(env);
        error.Set(Napi::String::New(env, "status"), Napi::Number::New(env, (int)errCode));
        if (nullptr != msg)
            error.Set(Napi::String::New(env, "message"), Napi::String::New(env, msg));
        return error;
    }

    static void DoDeferredLogging();

    static Napi::ObjectReference &GetJsLogger();
    static Napi::PropertyDescriptor GetLoggerProperty(Napi::Env env, Napi::Object exports);
};
} // namespace IModelBank
