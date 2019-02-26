#pragma once

#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <SpatialComposition/Elements/SpatialCompositionElementsApi.h>

BUILDINGSPATIAL_REFCOUNTED_PTR_AND_TYPEDEFS(Building)

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct Building : SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume
    {
    DGNELEMENT_DECLARE_MEMBERS(BUILDINGSPATIAL_CLASS_Building, SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume);
    typedef SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume T_Super;
    public:
        explicit Building(Dgn::GeometricElement3d::CreateParams const& params) : SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume(params) {}

        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db);
        
        BUILDINGSPATIAL_EXPORT static BuildingPtr Create (Dgn::DgnModelCR model);
    };

END_BUILDINGSPATIAL_NAMESPACE
