/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/ElevationStory.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include "Story.h"

BUILDINGSPATIAL_REFCOUNTED_PTR_AND_TYPEDEFS(ElevationStory)

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct ElevationStory : Story
    {
    DGNELEMENT_DECLARE_MEMBERS(BUILDINGSPATIAL_CLASS_ElevationStory, Story);

    protected:
        friend struct ElevationStoryHandler;

    protected:
        explicit ElevationStory(CreateParams const& params) : T_Super(params) {}

    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db);

        BUILDINGSPATIAL_EXPORT static ElevationStoryPtr Create(Dgn::DgnModelCR model);
    };

END_BUILDINGSPATIAL_NAMESPACE