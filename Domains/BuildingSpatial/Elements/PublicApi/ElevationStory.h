/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include "Story.h"

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct ElevationStory : Story
    {
    typedef Story T_Super;
    protected:
        explicit ElevationStory(CreateParams const& params) : T_Super(params) {}
    };

END_BUILDINGSPATIAL_NAMESPACE