/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <Napi/napi.h>

namespace IModelBank
{
struct EntitlementChecker
{
    static void Run(Napi::Env);
};

}
