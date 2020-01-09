/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <Bentley/Bentley.h>

namespace IModelBank
{
    Utf8String calculateHash(Utf8StringCR activityId, Utf8StringCR iModelId, Utf8StringCR time);
}
