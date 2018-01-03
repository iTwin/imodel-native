/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ChildCurveVectorPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ChildCurveVectorPlacementStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ChildCurveVectorPlacementStrategy : public GeometryPlacementStrategy
    {
    DEFINE_T_SUPER(GeometryPlacementStrategy)

    private:
        ChildCurveVectorManipulationStrategyPtr m_manipulationStrategy;

    protected:
        ChildCurveVectorPlacementStrategy(ChildCurveVectorManipulationStrategyP manipulationStrategy);

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyR() override { return *m_manipulationStrategy; }
        ChildCurveVectorManipulationStrategyCR GetChildCurveVectorManipulationStrategy() const { return *m_manipulationStrategy; }
        ChildCurveVectorManipulationStrategyR GetChildCurveVectorManipulationStrategyR() { return *m_manipulationStrategy; }
    };

END_BUILDING_SHARED_NAMESPACE