/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once


BEGIN_BUILDING_SHARED_NAMESPACE

struct PointPlacementStrategy : public LineStringPlacementStrategy
    {
    DEFINE_T_SUPER(LineStringPlacementStrategy)

    private:
        PointManipulationStrategyPtr m_manipulationStrategy;

    protected:
        PointPlacementStrategy() : T_Super(), m_manipulationStrategy(PointManipulationStrategy::Create()) {}
        PointPlacementStrategy(PointManipulationStrategyR manipulationStrategy);

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        PointManipulationStrategyCR GetPointManipulationStrategy() const { return *m_manipulationStrategy; }
        PointManipulationStrategyR GetPointManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;

        virtual void _CopyKeyPointsTo(ArcPlacementStrategyR) const override {}
        virtual void _CopyKeyPointsTo(LinePlacementStrategyR) const override {}
        virtual void _CopyKeyPointsTo(LineStringPlacementStrategyR) const override {}
        virtual void _CopyKeyPointsTo(SplineControlPointsPlacementStrategyR) const override {}
        virtual void _CopyKeyPointsTo(SplineThroughPointsPlacementStrategyR) const override {}

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static PointPlacementStrategyPtr Create(PointManipulationStrategyR manipulationStrategy) { return new PointPlacementStrategy(manipulationStrategy); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static PointPlacementStrategyPtr Create() { return new PointPlacementStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE