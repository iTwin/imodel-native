/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LineStringPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineStringPlacementStrategyPtr LineStringPlacementStrategy::Create
(
    LineStringPlacementStrategyType strategyType
)
    {
    switch (strategyType)
        {
        case LineStringPlacementStrategyType::Points:
            return LineStringPointsPlacementStrategy::Create();
        case LineStringPlacementStrategyType::MetesAndBounds:
            BeAssert(false && "Not implemented");
            return nullptr;
        default:
            BeAssert(false);
            return nullptr;
        }
    }