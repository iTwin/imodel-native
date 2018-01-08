/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurveVectorManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

enum class DefaultNewGeometryType
    {
    Default = 0,
    Line,
    Arc
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct CurveVectorManipulationStrategy : public GeometryManipulationStrategy
    {
    DEFINE_T_SUPER(GeometryManipulationStrategy)

    private:
        bvector<CurvePrimitiveManipulationStrategyPtr> m_primitiveStrategies;
        DefaultNewGeometryType m_defaultNewGeometryType;

        CurvePrimitivePlacementStrategyPtr GetStrategyForAppend();
        bool IsLastStrategyReadyForPop() const;

    protected:
        CurveVectorManipulationStrategy() : T_Super(), m_defaultNewGeometryType(DefaultNewGeometryType::Line) {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurveVectorPtr _Finish() const;

        virtual bvector<DPoint3d> _GetKeyPoints() const override;

        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override { return true; }

        virtual bool _IsDynamicKeyPointSet() const override;
        virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override { BeAssert(false && "Not implemented"); }
        virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _ResetDynamicKeyPoint() override;

        virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _PopKeyPoint() override;
        virtual void _RemoveKeyPoint(size_t index) override { BeAssert(false && "Not implemented"); }

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorPtr Finish() const;

        static CurveVectorManipulationStrategyPtr Create() { return new CurveVectorManipulationStrategy(); }

        void ChangeDefaultNewGeometryType(DefaultNewGeometryType newGeometryType);
    };

END_BUILDING_SHARED_NAMESPACE
