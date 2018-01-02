/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcPlacementStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcPlacementStrategy : public CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    protected:
        ArcPlacementStrategy(ArcManipulationStrategyP manipulationStrategy)
            : T_Super(manipulationStrategy) {}
    
        ArcManipulationStrategyR GetArcManipulationStrategyR();
        DPoint3d CalculateVec90KeyPoint(DPoint3dCR endPoint) const;
    };

END_BUILDING_SHARED_NAMESPACE