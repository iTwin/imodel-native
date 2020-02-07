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
    Utf8String calculateHash(Utf8String activityId, Utf8String iModelId, Utf8String time);
}
