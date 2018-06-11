/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInterop.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelBank.h"
#include <Bentley/Desktop/FileSystem.h>
#include <node-addon-api/napi.h>
#include <imodel-bank-package-version.h>

#define PROPERTY_ATTRIBUTES static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)

namespace IModelBank
{
static BeFileName s_addonDllDir;
static IModelBank::JsInterop::T_AssertHandler s_assertHandler;
static intptr_t s_mainThreadId;
static Napi::Env *s_env;
static Napi::ObjectReference s_logger;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void throwJsExceptionOnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
{
    if (JsInterop::IsMainThread())
        JsInterop::ThrowJsException(Utf8PrintfString("Assertion Failure: %ls (%ls:%d)\n", msg, file, line).c_str());
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value getLogger(Napi::CallbackInfo const &info)
{
    return s_logger.Value();
}

Napi::ObjectReference& jsInterop_getLogger() {return s_logger;}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void setLogger(Napi::CallbackInfo const &info)
{
    s_logger = Napi::ObjectReference::New(info[0].ToObject());
    s_logger.SuppressDestruct();
}

} // namespace IModelBank

using namespace IModelBank;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/18
//---------------------------------------------------------------------------------------
void JsInterop::Initialize(BeFileNameCR addonDllDir, T_AssertHandler assertHandler)
{
    s_addonDllDir = addonDllDir;
    s_assertHandler = assertHandler;

    static std::once_flag s_initFlag;
    std::call_once(s_initFlag, []() {
        BeFileName tempDir;
        Desktop::FileSystem::BeGetTempPath(tempDir);
        BeSQLiteLib::Initialize(tempDir);
        InitLogging();
    });
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsInterop::IsMainThread()
{
    return (BeThreadUtilities::GetCurrentThreadId() == s_mainThreadId);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelBank::JsInterop::ThrowJsException(Utf8CP msg)
{
    Napi::Error::New(*s_env, msg).ThrowAsJavaScriptException();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object iModelBankRegisterModule(Napi::Env env, Napi::Object exports)
{
    s_env = new Napi::Env(env);

    Napi::HandleScope scope(env);

    BeFileName addondir = Desktop::FileSystem::GetLibraryDir();

    s_mainThreadId = BeThreadUtilities::GetCurrentThreadId();

    IModelBank::JsInterop::Initialize(addondir, IModelBank::throwJsExceptionOnAssert);

    exports.DefineProperties(
        {
            Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), PROPERTY_ATTRIBUTES),
            Napi::PropertyDescriptor::Accessor("logger", &IModelBank::getLogger, &IModelBank::setLogger),
        });

    return exports;
}

NODE_API_MODULE(at_bentley_imodel_bank_nodeaddon, iModelBankRegisterModule)
