/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/PointManipulationStrategy.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Vytautas.Kaniusonis            10/2018
//=======================================================================================
struct PointManipulationStrategy : public LineStringManipulationStrategy
    {
    DEFINE_T_SUPER(LineStringManipulationStrategy)

    private:
        PointManipulationStrategy() : T_Super() {}
        bvector<DPoint3d> m_keyPoints;
        bool m_nonDynamicKeypointSet = false;

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _OnKeyPointsChanged() override {};
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _RemoveKeyPoint(size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _Clear() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<DPoint3d> _GetKeyPoints() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsDynamicKeyPointSet() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DynamicStateBaseCPtr _GetDynamicState() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetDynamicState(DynamicStateBaseCR state) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual IGeometryPtr _FinishGeometry() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;

        virtual ICurvePrimitive::CurvePrimitiveType _GetResultCurvePrimitiveType() const override { return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString; }

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static PointManipulationStrategyPtr Create() { return new PointManipulationStrategy(); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static PointManipulationStrategyPtr Create(DPoint3dCR point);
    };

END_BUILDING_SHARED_NAMESPACE
