/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/Building.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <SpatialComposition/Elements/SpatialCompositionElementsApi.h>

BUILDINGSPATIAL_REFCOUNTED_PTR_AND_TYPEDEFS(Building)

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct Building : SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume
    {
    DGNELEMENT_DECLARE_MEMBERS(BUILDINGSPATIAL_CLASS_Building, SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume);

    protected:
        friend struct BuildingHandler;

    protected:
        explicit Building(CreateParams const& params) : T_Super(params) {}
    
    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db);
        
        BUILDINGSPATIAL_EXPORT static BuildingPtr Create (Dgn::DgnModelCR model);
    };

END_BUILDINGSPATIAL_NAMESPACE
