/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryPlacementStrategy.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define GMS_PROPERTY_OVERRIDE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, value_type const& value) override; \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) const override;

//=======================================================================================
// A base class for strategies that create new geometry. This can seen as a wrapper
// for GeometryManipulationStrategy that allows manipulations only in the back of 
// the key point collection.
//
// Usually methods calls are forwarded to corresponding GeometryManipulationStrategy methods.
//
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct GeometryPlacementStrategy : public GeometryManipulationStrategyBase
    {
    DEFINE_T_SUPER(GeometryManipulationStrategyBase)

    protected:
        GeometryPlacementStrategy() : T_Super() {}

        // IRessetableDynamic
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetDynamicState(DynamicStateBaseCR state) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DynamicStateBaseCPtr _GetDynamicState() const override;

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
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override;

        GMS_PROPERTY_OVERRIDE(bool)
        GMS_PROPERTY_OVERRIDE(int)
        GMS_PROPERTY_OVERRIDE(double)
        GMS_PROPERTY_OVERRIDE(DPoint2d)
        GMS_PROPERTY_OVERRIDE(DVec3d)
        GMS_PROPERTY_OVERRIDE(DPlane3d)
        GMS_PROPERTY_OVERRIDE(RotMatrix)
        GMS_PROPERTY_OVERRIDE(Utf8String)
        GMS_PROPERTY_OVERRIDE(bvector<double>)
        GMS_PROPERTY_OVERRIDE(bvector<Utf8String>)
        GMS_PROPERTY_OVERRIDE(GeometryManipulationStrategyProperty)

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _CopyPropertiesTo(GeometryManipulationStrategyBaseR) const;

    public:
        //! Get the underlying manipulation strategy.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT GeometryManipulationStrategyCR GetManipulationStrategy() const;

        //! Add key point to the back of the key point collection. Resets dynamic key points before doing that.
        //! @param[in] newKeyPoint The key point to be added.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddKeyPoint(DPoint3dCR newKeyPoint);

        //! Remove key point from the back of the key point collection. Resets dynamic key points before doing that.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void PopKeyPoint();

        //! Add dynamic key point to the back of the key point collection. Resets dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoint The dynamic key point to be added.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);

        //! Add multiple dynamic key points to the back of the key point collection. Resets dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoints Dynamic key points to be added.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);
    };

END_BUILDING_SHARED_NAMESPACE