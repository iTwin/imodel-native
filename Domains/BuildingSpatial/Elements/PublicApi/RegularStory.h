/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/RegularStory.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include "Story.h"

BUILDINGSPATIAL_REFCOUNTED_PTR_AND_TYPEDEFS(RegularStory)

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct RegularStory : Story
    {
    DGNELEMENT_DECLARE_MEMBERS(BUILDINGSPATIAL_CLASS_RegularStory, Story);

    protected:
        friend struct RegularStoryHandler;

    protected:
        explicit RegularStory(CreateParams const& params) : T_Super(params) {}

        virtual BUILDINGSPATIAL_EXPORT Dgn::Render::GeometryParams _CreateGeometryParameters() override;
        virtual BUILDINGSPATIAL_EXPORT Dgn::DgnSubCategoryId _GetLabelSubCategoryId() const;
    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db);

        BUILDINGSPATIAL_EXPORT static RegularStoryPtr Create(Dgn::DgnModelCR model);

        //! Updates RegularStory's shape with given base curve vector
        //! @param[in] curveVector             new shape curve vector
        //! @param[in] updatePlacementOrigin   true if origin of this RegularStory should be updated
        //! @return                            true if there were no errors while updating RegularStory geometry
        virtual BUILDINGSPATIAL_EXPORT bool SetFootprintShape(CurveVectorCPtr curveVector, bool updatePlacementOrigin = true) final;
    };

END_BUILDINGSPATIAL_NAMESPACE