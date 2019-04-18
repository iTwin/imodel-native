/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/Space.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <SpatialComposition/Elements/SpatialCompositionElementsApi.h>

BUILDINGSPATIAL_REFCOUNTED_PTR_AND_TYPEDEFS(Space)

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct Space : SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume
    {
    DGNELEMENT_DECLARE_MEMBERS(BUILDINGSPATIAL_CLASS_Space, SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume);

    protected:
        friend struct SpaceHandler;

    protected:
        explicit Space(CreateParams const& params) : T_Super(params) {}

    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db);
        
        BUILDINGSPATIAL_EXPORT static SpacePtr Create (Dgn::DgnModelCR model);
    };

END_BUILDINGSPATIAL_NAMESPACE
