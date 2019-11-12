/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

enum class UsageType
    {
    Production = 0,
    Trial = 1,
    Beta = 2,
    HomeUse = 3,
    PreActivation = 4,
    Evaluation = 5,
    Academic = 6
    };

END_BENTLEY_LICENSING_NAMESPACE
