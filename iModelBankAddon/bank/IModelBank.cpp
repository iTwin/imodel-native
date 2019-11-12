/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
#include <delayimp.h>

// On Windows, we delay load node.lib. It declares all of its exports to be from "node.exe".
// We Register a delay-load hook to redirect any node.exe references to be from the loading executable.
// This way the addon will still work even if the host executable is not named node.exe.
static FARPROC delayLoadNotify(unsigned dliNotify, PDelayLoadInfo pdli) {return (dliNotePreLoadLibrary != dliNotify || 0 != _stricmp(pdli->szDll, "node.exe")) ? nullptr : (FARPROC)::GetModuleHandle(nullptr);}
decltype(__pfnDliNotifyHook2) __pfnDliNotifyHook2 = delayLoadNotify;
#endif

#include "../common/ConversionUtils.h"
#include "../common/NativeSQLiteDb.h"
#include "../common/NativeSQLiteStatement.h"
#include <imodel-bank-package-version.h>
#include "EntitlementChecker.h"

using namespace IModelBank;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object registerModule(Napi::Env env, Napi::Object exports)
{
    ConversionUtils::Initialize(new Napi::Env(env));

    Napi::HandleScope scope(env);

    NativeSQLiteDb::Init(env, exports);
    NativeSQLiteStatement::Init(env, exports);
    EntitlementChecker::Run(env);

    exports.Set("doDeferredLogging", Napi::Function::New(env, [](const Napi::CallbackInfo &) { ConversionUtils::DoDeferredLogging(); }));

    exports.DefineProperties(
        {
        Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), PROPERTY_ATTRIBUTES),
        ConversionUtils::GetLoggerProperty(env, exports)
        });
    return exports;
}

NODE_API_MODULE(imodel_bank, registerModule)
