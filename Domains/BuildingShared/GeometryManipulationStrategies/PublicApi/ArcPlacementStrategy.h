/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcPlacementStrategy : public CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    private:
        ArcManipulationStrategyPtr m_manipulationStrategy;

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcPlacementStrategy(ArcManipulationStrategyP manipulationStrategy);
    
        DPoint3d CalculateVec90KeyPoint(DPoint3dCR endPoint) const;

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyR() override { return *m_manipulationStrategy; }
        ArcManipulationStrategyCR GetArcManipulationStrategy() const { return *m_manipulationStrategy; }
        ArcManipulationStrategyR GetArcManipulationStrategyR() { return *m_manipulationStrategy; }

    public:
        BE_PROP_NAME(Normal)
    };

END_BUILDING_SHARED_NAMESPACE