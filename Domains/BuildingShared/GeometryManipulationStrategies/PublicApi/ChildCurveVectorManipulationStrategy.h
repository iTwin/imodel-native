/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ChildCurveVectorManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ChildCurveVectorManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    private:
        CurveVector::BoundaryType m_boundaryType;
        CurveVectorManipulationStrategyPtr m_cvManipulationStrategy;

        ChildCurveVectorManipulationStrategy();

    protected:
        virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;

        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override;

        virtual bool _IsContinious() const override { return false; }

        CurvePrimitiveManipulationStrategyPtr _Clone() const override;

        virtual bvector<DPoint3d> _GetKeyPoints() const override;

        virtual bool _IsDynamicKeyPointSet() const override;
        virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
        virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) override;
        virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        virtual void _ResetDynamicKeyPoint() override;

        virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints) override;
        virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        virtual void _PopKeyPoint() override;
        virtual void _RemoveKeyPoint(size_t index) override;
        virtual void _Clear() override;

        // IRessetableDynamic
        virtual void _SetDynamicState(DynamicStateBaseCR state) override;
        virtual DynamicStateBaseCPtr _GetDynamicState() const override;

    public:
        static ChildCurveVectorManipulationStrategyPtr Create() { return new ChildCurveVectorManipulationStrategy(); }
        static ChildCurveVectorManipulationStrategyPtr Create(CurveVectorCR cv);
    };

END_BUILDING_SHARED_NAMESPACE