/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ISolidPrimitivePtr ExtrusionPlacementStrategy::FinishExtrusion
(
    bool closedBaseShape,
    bool capped
) const
    {
    ExtrusionManipulationStrategyCP manipulationStrategy = dynamic_cast<ExtrusionManipulationStrategyCP>(&_GetManipulationStrategy());
    if (nullptr == manipulationStrategy)
        {
        BeAssert(false);
        return nullptr;
        }

    return manipulationStrategy->FinishExtrusion(closedBaseShape, capped);
    }