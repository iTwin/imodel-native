/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct SolidPrimitiveManipulationStrategy : public GeometryManipulationStrategy
    {
    DEFINE_T_SUPER(GeometryManipulationStrategy)

    protected:
        SolidPrimitiveManipulationStrategy();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<DPoint3d> _GetKeyPoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsDynamicKeyPointSet() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override;

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

        virtual GeometryManipulationStrategyCR _GetBaseShapeManipulationStrategy() const = 0;
        virtual GeometryManipulationStrategyR _GetBaseShapeManipulationStrategyForEdit() = 0;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsBaseComplete() const;
        virtual void _SetBaseComplete(bool value) {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, bool const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, bool& value) const override;
        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;

        virtual ISolidPrimitivePtr _FinishSolidPrimitive() const = 0;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual IGeometryPtr _FinishGeometry() const override;

    public:
        static constexpr Utf8CP prop_BaseComplete() { return "BaseComplete"; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool IsBaseComplete() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetBaseComplete(bool value);

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ISolidPrimitivePtr FinishSolidPrimitive() const;
    };

END_BUILDING_SHARED_NAMESPACE