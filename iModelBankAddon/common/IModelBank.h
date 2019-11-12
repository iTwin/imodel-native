/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <Bentley/Bentley.h>
#define NODE_ADDON_API_DISABLE_DEPRECATED
#include <Napi/napi.h>

namespace IModelBank
{
struct DisableJsCalls
    {
    static int s_disableJsCalls;
    DisableJsCalls() {++s_disableJsCalls;}
    ~DisableJsCalls() {--s_disableJsCalls;}
    };
}
