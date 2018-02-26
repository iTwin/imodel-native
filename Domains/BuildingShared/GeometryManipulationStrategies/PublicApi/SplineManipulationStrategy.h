/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/SplineManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
enum class SplinePlacementStrategyType
{
    ControlPoints = 0,
    ThroughPoints
};

struct SplineManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    protected:
        SplineManipulationStrategy() : T_Super() {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override { return false; }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override { return true; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsContinious() const override { return true; }

        virtual SplinePlacementStrategyType _GetType() const = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static SplineManipulationStrategyPtr Create(SplinePlacementStrategyType placementStrategy);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT SplinePlacementStrategyPtr CreatePlacement();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT SplinePlacementStrategyType GetType() const;
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
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;

        void _SetOrder(int order) { m_order = order; }
        int _GetOrder() const { return m_order; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitiveManipulationStrategyPtr _Clone() const override { return new SplineControlPointsManipulationStrategy(*this); };

        virtual SplinePlacementStrategyType _GetType() const override {return SplinePlacementStrategyType::ControlPoints;}
    public:
        static SplineControlPointsManipulationStrategyPtr Create(int order) { return new SplineControlPointsManipulationStrategy(order); }

        void SetOrder(int order) { _SetOrder(order); }
        int GetOrder() const { return _GetOrder(); }
        
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const int default_Order;
    };

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
struct SplineThroughPointsManipulationStrategy : public SplineManipulationStrategy
    {
    DEFINE_T_SUPER(SplineManipulationStrategy)

    private:
        DVec3d m_startTangent = DVec3d::From(1, 0, 0);
        DVec3d m_endTangent = DVec3d::From(1, 0, 0);

        SplineThroughPointsManipulationStrategy() : T_Super() {}

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;

        void _SetStartTangent(DVec3d startTangent) { m_startTangent = startTangent; }
        void _RemoveStartTangent() { m_startTangent.Zero(); m_startTangent.Normalize(); }
        DVec3d _GetStartTangent() const { return m_startTangent; }

        void _SetEndTangent(DVec3d endTangent) { m_endTangent = endTangent; }
        void _RemoveEndTangent() { m_endTangent.Zero(); m_endTangent.Normalize(); }
        DVec3d _GetEndTangent() const { return m_endTangent; }

        virtual SplinePlacementStrategyType _GetType() const override { return SplinePlacementStrategyType::ThroughPoints; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitiveManipulationStrategyPtr _Clone() const override { return new SplineThroughPointsManipulationStrategy(*this); };

    public:
        static SplineThroughPointsManipulationStrategyPtr Create() { return new SplineThroughPointsManipulationStrategy(); }

        void SetStartTangent(DVec3d startTangent);
        void RemoveStartTangent() { _RemoveStartTangent(); }
        DVec3d GetStartTangent() const { return _GetStartTangent(); }

        void SetEndTangent(DVec3d endTangent);
        void RemoveEndTangent() { _RemoveEndTangent(); }
        DVec3d GetEndTangent() const { return _GetEndTangent(); }
    };

END_BUILDING_SHARED_NAMESPACE
