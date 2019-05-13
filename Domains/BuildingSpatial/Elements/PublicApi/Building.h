/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
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
    
        virtual BUILDINGSPATIAL_EXPORT Dgn::Render::GeometryParams _CreateGeometryParameters() override;
    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db);
        
        BUILDINGSPATIAL_EXPORT static BuildingPtr Create (Dgn::DgnModelCR model);

        //! Updates building's shape with given curve vector
        //! @param[in] curveVector             new shape curve vector
        //! @param[in] updatePlacementOrigin   true if origin of this building should be updated
        //! @return                            true if there were no errors while updating building shape
        virtual BUILDINGSPATIAL_EXPORT bool SetFootprintShape(CurveVectorCPtr curveVector, bool updatePlacementOrigin = true) final;
    };

END_BUILDINGSPATIAL_NAMESPACE
