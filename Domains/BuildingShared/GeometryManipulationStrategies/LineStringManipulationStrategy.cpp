/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LineStringManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineStringManipulationStrategyPtr LineStringManipulationStrategy::Create
(
    ICurvePrimitiveCR primitive
)
    {
    BeAssert(primitive.GetCurvePrimitiveType() == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString);
    if (primitive.GetCurvePrimitiveType() != ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
        return nullptr;

    bvector<DPoint3d> const* keyPoints = primitive.GetLineStringCP();
    if (nullptr == keyPoints)
        return nullptr;

    LineStringManipulationStrategyPtr strategy = Create();
    strategy->AppendKeyPoints(*keyPoints);
    return strategy;
    }

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
    return LineStringPlacementStrategy::Create(LineStringPlacementStrategyType::Points, *this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineStringPlacementStrategyPtr LineStringManipulationStrategy::_CreateLineStringPlacementStrategy
(
    LineStringPlacementStrategyType strategyType
)
    {
    return LineStringPlacementStrategy::Create(strategyType, *this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<IGeometryPtr> LineStringManipulationStrategy::_FinishConstructionGeometry() const
    {
    return bvector<IGeometryPtr>();
    }