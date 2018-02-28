/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define GMS_PROPERTY_OVERRIDE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, value_type const& value) override; \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) const override;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct GeometryPlacementStrategy : public GeometryManipulationStrategyBase
    {
    DEFINE_T_SUPER(GeometryManipulationStrategyBase)

    protected:
        GeometryPlacementStrategy() : T_Super() {}

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const = 0;
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() = 0;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<DPoint3d> _GetKeyPoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsDynamicKeyPointSet() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual IGeometryPtr _FinishGeometry() const override;

        GMS_PROPERTY_OVERRIDE(bool)
        GMS_PROPERTY_OVERRIDE(int)
        GMS_PROPERTY_OVERRIDE(double)
        GMS_PROPERTY_OVERRIDE(DVec3d)
        GMS_PROPERTY_OVERRIDE(DPlane3d)
        GMS_PROPERTY_OVERRIDE(Dgn::DgnElementId)
        GMS_PROPERTY_OVERRIDE(Dgn::DgnElementCP)
        GMS_PROPERTY_OVERRIDE(Utf8String)
        GMS_PROPERTY_OVERRIDE(bvector<double>)
        GMS_PROPERTY_OVERRIDE(bvector<Utf8String>)
        GMS_PROPERTY_OVERRIDE(GeometryManipulationStrategyProperty)

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT GeometryManipulationStrategyCR GetManipulationStrategy() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddKeyPoint(DPoint3dCR newKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void PopKeyPoint();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);
    };

END_BUILDING_SHARED_NAMESPACE