/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ElementManipulationStrategy.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define ELEM_PROPERTY_OVERRIDE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, value_type const& value) override; \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) const override;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ElementManipulationStrategy : public GeometryManipulationStrategy
    {
    DEFINE_T_SUPER(GeometryManipulationStrategy)

    friend struct ElementPlacementStrategy;

    protected:
        // IRessetableDynamic
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetDynamicState(DynamicStateBaseCR state) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DynamicStateBaseCPtr _GetDynamicState() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _RemoveKeyPoint(size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _Clear() override;

        virtual GeometryManipulationStrategyCR _GetGeometryManipulationStrategy() const = 0;
        virtual GeometryManipulationStrategyR _GetGeometryManipulationStrategyForEdit() = 0;

        virtual GeometryPlacementStrategyCPtr _TryGetGeometryPlacementStrategy() const = 0;
        virtual GeometryPlacementStrategyPtr _TryGetGeometryPlacementStrategyForEdit() = 0;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual IGeometryPtr _FinishGeometry() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override;

        ELEM_PROPERTY_OVERRIDE(bool)
        ELEM_PROPERTY_OVERRIDE(int)
        ELEM_PROPERTY_OVERRIDE(double)
        ELEM_PROPERTY_OVERRIDE(DPoint2d)
        ELEM_PROPERTY_OVERRIDE(DVec3d)
        ELEM_PROPERTY_OVERRIDE(DPlane3d)
        ELEM_PROPERTY_OVERRIDE(RotMatrix)
        ELEM_PROPERTY_OVERRIDE(Utf8String)
        ELEM_PROPERTY_OVERRIDE(bvector<double>)
        ELEM_PROPERTY_OVERRIDE(bvector<Utf8String>)
        ELEM_PROPERTY_OVERRIDE(GeometryManipulationStrategyProperty)

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _CopyPropertiesTo(GeometryManipulationStrategyBaseR) const;
    };

END_BUILDING_SHARED_NAMESPACE