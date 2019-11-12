/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

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
            return LineStringMetesAndBoundsPlacementStrategy::Create();
        default:
            BeAssert(false);
            return nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineStringPlacementStrategyPtr LineStringPlacementStrategy::Create
(
    LineStringPlacementStrategyType strategyType, 
    LineStringManipulationStrategyR manipulationStrategy
)
    {
    LineStringPlacementStrategyPtr placementStrategy = Create(strategyType);
    if (placementStrategy.IsNull())
        return nullptr;

    placementStrategy->m_manipulationStrategy = &manipulationStrategy;
    return placementStrategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void LineStringPlacementStrategy::_CopyKeyPointsTo
(
    LinePlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.empty())
        return;

    other.AddKeyPoint(keyPoints.front());
    }