/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ChildCurveVectorPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ChildCurveVectorPlacementStrategy::ChildCurveVectorPlacementStrategy
(
    ChildCurveVectorManipulationStrategyR manipulationStrategy
)
    : T_Super()
    , m_manipulationStrategy(&manipulationStrategy)
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ChildCurveVectorPlacementStrategyPtr ChildCurveVectorPlacementStrategy::Create
(
    ChildCurveVectorManipulationStrategyR manipulationStrategy
)
    {
    return new ChildCurveVectorPlacementStrategy(manipulationStrategy);
    }