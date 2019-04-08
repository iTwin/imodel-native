/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ChildCurveVectorPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ChildCurveVectorPlacementStrategy : public CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    private:
        ChildCurveVectorManipulationStrategyPtr m_manipulationStrategy;

    protected:
        ChildCurveVectorPlacementStrategy(ChildCurveVectorManipulationStrategyR manipulationStrategy);

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        ChildCurveVectorManipulationStrategyCR GetChildCurveVectorManipulationStrategy() const { return *m_manipulationStrategy; }
        ChildCurveVectorManipulationStrategyR GetChildCurveVectorManipulationStrategyForEdit() { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_manipulationStrategy; }

    public:
        static ChildCurveVectorPlacementStrategyPtr Create(ChildCurveVectorManipulationStrategyR manipulationStrategy);
    };

END_BUILDING_SHARED_NAMESPACE