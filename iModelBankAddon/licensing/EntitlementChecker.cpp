/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "EntitlementChecker.h"
#include <Licensing/AccessKeyClient.h>
#include "../common/ConversionUtils.h"
#include "../common/macros.h"

using namespace IModelBank;

static Napi::Value checkEntitlement(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENT_STRING(0, licFileName);

    ConversionUtils::LogMessage("imodel-bank-licensing", NativeLogging::SEVERITY::LOG_INFO, Utf8PrintfString("checkEntitlement %s - TBD", licFileName.c_str()).c_str());

    return Napi::Number::New(info.Env(), 0);
}

void EntitlementChecker::Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("checkEntitlement", Napi::Function::New(env, checkEntitlement));
}
