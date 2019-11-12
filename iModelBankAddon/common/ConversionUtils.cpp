/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConversionUtils.h"
#include <Bentley/Desktop/FileSystem.h>
#include <BeSQLite/BeSQLite.h>

using namespace IModelBank;
USING_NAMESPACE_BENTLEY_SQLITE

int IModelBank::DisableJsCalls::s_disableJsCalls;

namespace
{
static BeFileName s_addonDllDir;
static intptr_t s_mainThreadId;
static Napi::Env *s_env;
static Napi::ObjectReference s_logger;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value getLogger(Napi::CallbackInfo const &info)
{
    return s_logger.Value();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void setLogger(Napi::CallbackInfo const &info)
{
    s_logger = Napi::ObjectReference::New(info[0].ToObject());
    s_logger.SuppressDestruct();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void throwJsExceptionOnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
{
    Utf8PrintfString formattedMessage("Assertion Failure: %ls (%ls:%d)\n", msg, file, line);
    if (BeThreadUtilities::GetCurrentThreadId() == s_mainThreadId)
        Napi::Error::New(*s_env, formattedMessage.c_str()).ThrowAsJavaScriptException();
    else
        ConversionUtils::LogMessage("iModelBank", NativeLogging::SEVERITY::LOG_ERROR, formattedMessage.c_str());
}

} // namespace

using namespace IModelBank;

Napi::ObjectReference &ConversionUtils::GetJsLogger() { return s_logger; }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConversionUtils::IsJsExecutionDisabled()
{
    return (DisableJsCalls::s_disableJsCalls != 0) || !IsMainThread() || (!s_logger.IsEmpty() && s_logger.Env().IsExceptionPending());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/18
//---------------------------------------------------------------------------------------
void ConversionUtils::Initialize(Napi::Env *env)
{
    s_env = env;

    s_addonDllDir = Desktop::FileSystem::GetLibraryDir();

    s_mainThreadId = BeThreadUtilities::GetCurrentThreadId();

    static std::once_flag s_initFlag;
    std::call_once(s_initFlag, []() {
        BeFileName tempDir;
        Desktop::FileSystem::BeGetTempPath(tempDir);
        BeSQLite::BeSQLiteLib::Initialize(tempDir);
        ConversionUtils::InitLogging();
        BeAssertFunctions::SetBeAssertHandler(throwJsExceptionOnAssert);
#ifdef _WIN32
        static bool s_quietAsserts = true;
        if (s_quietAsserts)
            _set_error_mode(_OUT_TO_STDERR);
        else
            _set_error_mode(_OUT_TO_MSGBOX);
#endif
    });
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConversionUtils::IsMainThread()
{
    return (BeThreadUtilities::GetCurrentThreadId() == s_mainThreadId);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void ConversionUtils::ThrowJsException(Utf8CP msg)
{
    Napi::Error::New(*s_env, msg).ThrowAsJavaScriptException();
}

Napi::Value ConversionUtils::GetValue(Statement &stmt, int i, Napi::Env env)
{
    switch (stmt.GetColumnType(i))
    {
    case DbValueType::IntegerVal:
        return Napi::Number::New(env, stmt.GetValueInt(i));
    case DbValueType::FloatVal:
        return Napi::Number::New(env, stmt.GetValueDouble(i));
    case DbValueType::TextVal:
        return Napi::String::New(env, stmt.GetValueText(i));
    case DbValueType::NullVal:
        return env.Null();
    case DbValueType::BlobVal:
    {
        auto abuf = Napi::ArrayBuffer::New(env, stmt.GetColumnBytes(i));
        memcpy(abuf.Data(), stmt.GetValueBlob(i), stmt.GetColumnBytes(i));
        return abuf;
    }

    default:
        BeAssert(false);
        return env.Undefined();
    }
}

BentleyStatus ConversionUtils::BindValue(Statement &stmt, int i, Napi::Value value)
{
    BentleyStatus status = BSISUCCESS;
    switch (value.Type())
    {
    case napi_undefined:
    case napi_null:
        stmt.BindNull(i);
        break;
    case napi_boolean:
        stmt.BindInt(i, value.ToBoolean().Value());
        break;
    case napi_number:
        stmt.BindDouble(i, value.ToNumber().DoubleValue());
        break;

    case napi_string:
    case napi_symbol:
        stmt.BindText(i, value.ToString().Utf8Value().c_str(), Statement::MakeCopy::Yes);
        break;
    // case napi_object :
    // case napi_function :
    // case napi_external :
    default:
        BeAssert(false);
        status = BSIERROR;
    }
    return status;
}

Napi::PropertyDescriptor ConversionUtils::GetLoggerProperty(Napi::Env env, Napi::Object exports)
{
    return Napi::PropertyDescriptor::Accessor(env, exports, "logger", &getLogger, &setLogger);
}
