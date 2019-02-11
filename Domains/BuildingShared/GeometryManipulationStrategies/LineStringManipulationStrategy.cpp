/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LineStringManipulationStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
bvector<ConstructionGeometry> LineStringManipulationStrategy::_FinishConstructionGeometry() const
    {
    return bvector<ConstructionGeometry>();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
template <typename T> void LineStringManipulationStrategy::UpdateKeyPoint
(
    size_t index, 
    T updateFn
)
    {
    bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
    if ((keyPoints.size() > 1) &&
        (0 == index || keyPoints.size() - 1 == index) &&
        (keyPoints.front().AlmostEqual(keyPoints.back())))
        {
        updateFn(0);
        updateFn(keyPoints.size() - 1);
        }
    else
        {
        updateFn(index);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineStringManipulationStrategy::_UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
)
    {
    UpdateKeyPoint(index, [&] (size_t i)
        {
        T_Super::_UpdateDynamicKeyPoint(newDynamicKeyPoint, i);
        });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineStringManipulationStrategy::_ReplaceKeyPoint
(
    DPoint3dCR newKeyPoint, 
    size_t index
)
    {
    UpdateKeyPoint(index, [&] (size_t i)
        {
        T_Super::_ReplaceKeyPoint(newKeyPoint, i);
        });
    }