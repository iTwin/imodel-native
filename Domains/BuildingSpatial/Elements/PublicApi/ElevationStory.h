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

        virtual BUILDINGSPATIAL_EXPORT Dgn::Render::GeometryParams _CreateGeometryParameters() override;
        virtual BUILDINGSPATIAL_EXPORT Dgn::DgnSubCategoryId _GetLabelSubCategoryId() const;
    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db);

        BUILDINGSPATIAL_EXPORT static ElevationStoryPtr Create(Dgn::DgnModelCR model);

        //! Updates ElevationStory's shape with given base curve vector
        //! @param[in] curveVector             new shape curve vector
        //! @param[in] updatePlacementOrigin   true if origin of this ElevationStory should be updated
        //! @return                            true if there were no errors while updating ElevationStory geometry
        virtual BUILDINGSPATIAL_EXPORT bool SetFootprintShape(CurveVectorCPtr curveVector, bool updatePlacementOrigin = true) final;
    };

END_BUILDINGSPATIAL_NAMESPACE