/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ExtrusionPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ExtrusionPlacementStrategy::ExtrusionPlacementStrategy()
    : T_Super(*CreateDefaultExtrusionManipulationStrategy()) 
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ExtrusionManipulationStrategyPtr ExtrusionPlacementStrategy::CreateDefaultExtrusionManipulationStrategy()
    {
    CurveVectorManipulationStrategyPtr baseShapeManipulationStrategy = CurveVectorManipulationStrategy::Create();
    baseShapeManipulationStrategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::LineString);
    baseShapeManipulationStrategy->ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType::Points);
    return ExtrusionManipulationStrategy::Create(*baseShapeManipulationStrategy);
    }