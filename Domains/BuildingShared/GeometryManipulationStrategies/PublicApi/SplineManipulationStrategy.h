/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/SplineManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SplineManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SplineControlPointsManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SplineThroughPointsManipulationStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
struct SplineManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    protected:
        SplineManipulationStrategy() : T_Super() {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override { return !_IsComplete(); };
    };

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
struct SplineControlPointsManipulationStrategy : public SplineManipulationStrategy
    {
    DEFINE_T_SUPER(SplineManipulationStrategy)

    private:
        int m_order;

        SplineControlPointsManipulationStrategy(int order) : T_Super(), m_order(order) {}

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override { BeAssert(false && "Not implemented"); return nullptr; }

        void _SetOrder(int order) { m_order = order; }
        int _GetOrder() const { return m_order; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitiveManipulationStrategyPtr _Clone() const override { return Create(m_order); };
    public:
        static SplineControlPointsManipulationStrategyPtr Create(int order) { return new SplineControlPointsManipulationStrategy(order); }

        void SetOrder(int order) { _SetOrder(order); }
        int GetOrder() const { return _GetOrder(); }
    };

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
struct SplineThroughPointsManipulationStrategy : public SplineManipulationStrategy
    {
    DEFINE_T_SUPER(SplineManipulationStrategy)

    private:
        DVec3d m_startTangent = DVec3d::From(0, 0, 0);
        DVec3d m_endTangent = DVec3d::From(0, 0, 0);

        SplineThroughPointsManipulationStrategy() : T_Super() {}

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override { BeAssert(false && "Not implemented"); return nullptr; }

        void _SetStartTangent(DVec3d startTangent) { m_startTangent = startTangent; }
        void _RemoveStartTangent() { m_startTangent.Zero(); }
        DVec3d _GetStartTangent() const { return m_startTangent; }

        void _SetEndTangent(DVec3d endTangent) { m_endTangent = endTangent; }
        void _RemoveEndTangent() { m_endTangent.Zero(); }
        DVec3d _GetEndTangent() const { return m_endTangent; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitiveManipulationStrategyPtr _Clone() const override { return Create(); };

    public:
        static SplineThroughPointsManipulationStrategyPtr Create() { return new SplineThroughPointsManipulationStrategy(); }

        void SetStartTangent(DVec3d startTangent) { _SetStartTangent(startTangent); }
        void RemoveStartTangent() { _RemoveStartTangent(); }
        DVec3d GetStartTangent() const { _GetStartTangent(); }

        void SetEndTangent(DVec3d endTangent) { _SetEndTangent(endTangent); }
        void RemoveEndTangent() { _RemoveEndTangent(); }
        DVec3d GetEndTangent() const { return _GetEndTangent(); }
    };

END_BUILDING_SHARED_NAMESPACE
