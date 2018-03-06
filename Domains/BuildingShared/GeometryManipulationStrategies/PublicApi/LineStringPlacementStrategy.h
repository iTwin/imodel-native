/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/LineStringPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

enum class LineStringPlacementStrategyType
    {
    Points = 0,
    MetesAndBounds
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct LineStringPlacementStrategy : public CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    private:
        LineStringManipulationStrategyPtr m_manipulationStrategy;

    protected:
        LineStringPlacementStrategy() : T_Super(), m_manipulationStrategy(LineStringManipulationStrategy::Create()) {}

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        LineStringManipulationStrategyCR GetLineStringManipulationStrategy() const { return *m_manipulationStrategy; }
        LineStringManipulationStrategyR GetLineStringManipulationStrategyForEdit() { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_manipulationStrategy; }

    public:
        static LineStringPlacementStrategyPtr Create(LineStringPlacementStrategyType strategyType);
        static LineStringPlacementStrategyPtr Create(LineStringPlacementStrategyType strategyType, LineStringManipulationStrategyR manipulationStrategy);
    };

END_BUILDING_SHARED_NAMESPACE