/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LineStringManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr LineStringManipulationStrategy::_FinishPrimitive() const
    {
    bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
    if (keyPoints.size() < 2)
        return nullptr;

    return ICurvePrimitive::CreateLineString(keyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitiveManipulationStrategyPtr LineStringManipulationStrategy::_Clone() const
    {
    return Create();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineStringManipulationStrategy::_IsComplete() const
    {
    return GetKeyPoints().size() >= 2;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr LineStringManipulationStrategy::_CreateDefaultPlacementStrategy()
    {
    return LineStringPlacementStrategy::Create(LineStringPlacementStrategyType::Points);
    }