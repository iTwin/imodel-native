/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/SplinePlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct SplinePlacementStrategy : CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    protected:
        SplinePlacementStrategy() : T_Super() {}
    };

struct SplineControlPointsPlacementStrategy : SplinePlacementStrategy
    {
    DEFINE_T_SUPER(SplinePlacementStrategy)

    private:
        SplineControlPointsManipulationStrategyPtr m_manipulationStrategy;

    protected:
        SplineControlPointsPlacementStrategy(int order) :
            T_Super(), 
            m_manipulationStrategy(SplineControlPointsManipulationStrategy::Create(order)) {}

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyR() override { return *m_manipulationStrategy; }
        SplineControlPointsManipulationStrategyCR GetSplineControlPointsManipulationStrategy() const { return *m_manipulationStrategy; }
        SplineControlPointsManipulationStrategyR GetSplineControlPointsManipulationStrategyR() { return *m_manipulationStrategy; }

        virtual void _SetProperty(Utf8CP key, const int & value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, int & value) const override;

        void _SetOrder(int const & order);
        int _GetOrder() const;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static SplineControlPointsPlacementStrategyPtr Create(int order) { return new SplineControlPointsPlacementStrategy(order); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const Utf8CP prop_Order;
    };

struct SplineThroughPointsPlacementStrategy : SplinePlacementStrategy
    {
    DEFINE_T_SUPER(SplinePlacementStrategy)

    private:
        SplineThroughPointsManipulationStrategyPtr m_manipulationStrategy;

    protected:
        SplineThroughPointsPlacementStrategy() :
            T_Super(),
            m_manipulationStrategy(SplineThroughPointsManipulationStrategy::Create())
            {
            }

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyR() override { return *m_manipulationStrategy; }
        SplineThroughPointsManipulationStrategyCR GetSplineThroughPointsManipulationStrategy() const { return *m_manipulationStrategy; }
        SplineThroughPointsManipulationStrategyR GetSplineThroughPointsManipulationStrategyR() { return *m_manipulationStrategy; }

        virtual void _SetStartTangent(DVec3d tangent);
        virtual void _RemoveStartTangent();
        virtual DVec3d _GetStartTangent() const;

        virtual void _SetEndTangent(DVec3d tangent);
        virtual void _RemoveEndTangent();
        virtual DVec3d _GetEndTangent() const;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static SplineThroughPointsPlacementStrategyPtr Create() { return new SplineThroughPointsPlacementStrategy(); }

        virtual void SetStartTangent(DVec3d tangent) { _SetStartTangent(tangent); }
        virtual void RemoveStartTangent() { _RemoveStartTangent(); }
        virtual DVec3d GetStartTangent() const { return _GetStartTangent(); }

        virtual void SetEndTangent(DVec3d tangent) { _SetEndTangent(tangent); }
        virtual void RemoveEndTangent() { _RemoveEndTangent(); }
        virtual DVec3d GetEndTangent() const { return _GetEndTangent(); }
    };
END_BUILDING_SHARED_NAMESPACE