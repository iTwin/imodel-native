/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct SplinePlacementStrategy : CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    protected:
        SplinePlacementStrategy() : T_Super() {}
        virtual SplinePlacementStrategyType _GetType() const = 0;
    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT SplinePlacementStrategyType GetType() const;
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
        SplineControlPointsPlacementStrategy(SplineControlPointsManipulationStrategyR);


        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        SplineControlPointsManipulationStrategyCR GetSplineControlPointsManipulationStrategy() const { return *m_manipulationStrategy; }
        SplineControlPointsManipulationStrategyR GetSplineControlPointsManipulationStrategyForEdit() { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_manipulationStrategy; }

        virtual SplinePlacementStrategyType _GetType() const override { return SplinePlacementStrategyType::ControlPoints; }

        void _SetOrder(int const & order);
        int _GetOrder() const;
    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static SplineControlPointsPlacementStrategyPtr Create(int order) { return new SplineControlPointsPlacementStrategy(order); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static SplineControlPointsPlacementStrategyPtr Create(SplineControlPointsManipulationStrategyR manipulationStrategy) { return new SplineControlPointsPlacementStrategy(manipulationStrategy); }

        static constexpr Utf8CP prop_Order() { return SplineControlPointsManipulationStrategy::prop_Order(); }
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
        SplineThroughPointsPlacementStrategy(SplineThroughPointsManipulationStrategyR);

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        SplineThroughPointsManipulationStrategyCR GetSplineThroughPointsManipulationStrategy() const { return *m_manipulationStrategy; }
        SplineThroughPointsManipulationStrategyR GetSplineThroughPointsManipulationStrategyForEdit() { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_manipulationStrategy; }

        virtual SplinePlacementStrategyType _GetType() const override { return SplinePlacementStrategyType::ThroughPoints; }

        virtual void _SetStartTangent(DVec3d tangent);
        virtual void _RemoveStartTangent();
        virtual DVec3d _GetStartTangent() const;

        virtual void _SetEndTangent(DVec3d tangent);
        virtual void _RemoveEndTangent();
        virtual DVec3d _GetEndTangent() const;

    public:
        static constexpr Utf8CP prop_StartTangent() { return SplineThroughPointsManipulationStrategy::prop_StartTangent(); }
        static constexpr Utf8CP prop_EndTangent() { return SplineThroughPointsManipulationStrategy::prop_EndTangent(); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static SplineThroughPointsPlacementStrategyPtr Create() { return new SplineThroughPointsPlacementStrategy(); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static SplineThroughPointsPlacementStrategyPtr Create(SplineThroughPointsManipulationStrategyR manipulationStrategy) { return new SplineThroughPointsPlacementStrategy(manipulationStrategy); }

        virtual void SetStartTangent(DVec3d tangent) { _SetStartTangent(tangent); }
        virtual void RemoveStartTangent() { _RemoveStartTangent(); }
        virtual DVec3d GetStartTangent() const { return _GetStartTangent(); }

        virtual void SetEndTangent(DVec3d tangent) { _SetEndTangent(tangent); }
        virtual void RemoveEndTangent() { _RemoveEndTangent(); }
        virtual DVec3d GetEndTangent() const { return _GetEndTangent(); }
    };
END_BUILDING_SHARED_NAMESPACE