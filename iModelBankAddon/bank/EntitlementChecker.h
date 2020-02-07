/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <Napi/napi.h>
#include <Bentley/Bentley.h>

namespace IModelBank
{
enum class EntitlementStatus {
    Success = 0,
    LicensingUrlNotSet,
    iModelIdNotSet,
    RequestFailed,
    InvalidResponse,
    Unknown = -1,
};

struct EntitlementChecker
{
    static BentleyApi::Utf8String s_url;
    static BentleyApi::Utf8String s_iModelId;

    static void Run(Napi::Env env);
    static void initialize(Napi::CallbackInfo const& info);
    static EntitlementStatus checkEntitlementWithRetries();
};

}
