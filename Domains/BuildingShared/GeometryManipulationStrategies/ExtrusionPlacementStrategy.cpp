/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ExtrusionPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

const Utf8CP ExtrusionPlacementStrategy::prop_Height = ExtrusionManipulationStrategy::prop_Height;
const Utf8CP ExtrusionPlacementStrategy::prop_BaseShapeStrategy = ExtrusionManipulationStrategy::prop_BaseShapeStrategy;
const Utf8CP ExtrusionPlacementStrategy::prop_ContinuousBaseShapePrimitiveComplete = ExtrusionManipulationStrategy::prop_ContinuousBaseShapePrimitiveComplete;

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