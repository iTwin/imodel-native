/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ElementPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define EPS_PROPERTY_OVERRIDE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, value_type const& value) override; \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) const override;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ElementPlacementStrategy : public GeometryPlacementStrategy
    {
    DEFINE_T_SUPER(GeometryPlacementStrategy)

    protected:
        ElementPlacementStrategy() : T_Super() {}

        virtual ElementManipulationStrategyCR _GetElementManipulationStrategy() const = 0;
        virtual ElementManipulationStrategyR _GetElementManipulationStrategyForEdit() = 0;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT GeometryPlacementStrategyCPtr TryGetGeometryPlacementStrategy() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT GeometryPlacementStrategyPtr TryGetGeometryPlacementStrategyForEdit();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<DPoint3d> _GetKeyPoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsDynamicKeyPointSet() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual Dgn::DgnElementPtr _FinishElement();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model);

        EPS_PROPERTY_OVERRIDE(bool)
        EPS_PROPERTY_OVERRIDE(int)
        EPS_PROPERTY_OVERRIDE(double)
        EPS_PROPERTY_OVERRIDE(DVec3d)
        EPS_PROPERTY_OVERRIDE(DPlane3d)
        EPS_PROPERTY_OVERRIDE(RotMatrix)
        EPS_PROPERTY_OVERRIDE(Dgn::DgnElementId)
        EPS_PROPERTY_OVERRIDE(Dgn::DgnElementCP)
        EPS_PROPERTY_OVERRIDE(Dgn::ColorDef)
        EPS_PROPERTY_OVERRIDE(Utf8String)
        EPS_PROPERTY_OVERRIDE(bvector<double>)
        EPS_PROPERTY_OVERRIDE(bvector<Utf8String>)
        EPS_PROPERTY_OVERRIDE(GeometryManipulationStrategyProperty)

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement(Dgn::DgnModelR model);
    };

END_BUILDING_SHARED_NAMESPACE