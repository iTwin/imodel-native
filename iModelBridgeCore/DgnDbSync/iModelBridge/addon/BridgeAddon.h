/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <cstdio>
#include <json/value.h>
#include <Napi/napi.h>
#include <iModelBridge/iModelBridgeFwk.h>

#include "OidcTokenProvider.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_DGN
using namespace Napi;

namespace BridgeNative
{

struct BridgeWorker
    {
    static int RunBridge(Napi::Env& env, const char* json, bool doLogging);
    static void InitAsyncWorker(Napi::Env& env, Napi::Object exports);
    };

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  06/18
//=======================================================================================
struct JsInterop
    {
    static bool IsMainThread();

    static void ThrowJsException(Utf8CP msg);

    static void InitLogging();
    static NativeLogging::ILogger &GetNativeLogger();
    static void LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg);
    static bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev);
    };
} // namespace BridgeNative
