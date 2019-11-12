/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../common/IModelBank.h"

namespace IModelBank
{
struct EntitlementChecker
{
    static void Init(Napi::Env env, Napi::Object exports);
};
} // namespace IModelBank
