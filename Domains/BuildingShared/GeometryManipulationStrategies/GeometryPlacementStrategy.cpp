/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryPlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>
#include "PublicApi/GeometryManipulationStrategyBase.h"
#include "PublicApi/GeometryManipulationStrategy.h"
#include "PublicApi/GeometryPlacementStrategy.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
GeometryPlacementStrategy::GeometryPlacementStrategy
(
    GeometryManipulationStrategyP manipulationStrategy
)
    : T_Super()
    , m_manipulationStrategy(manipulationStrategy)
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    m_manipulationStrategy->AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> const& GeometryPlacementStrategy::_GetKeyPoints() const
    {
    return m_manipulationStrategy->GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::PopKeyPoint()
    {
    m_manipulationStrategy->PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryPlacementStrategy::_IsDynamicKeyPointSet() const
    {
    return m_manipulationStrategy->IsDynamicKeyPointSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_SetDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index,
    GeometryManipulationStrategyBase::DynamicKeyPointType type
)
    {
    m_manipulationStrategy->SetDynamicKeyPoint(newDynamicKeyPoint, index, type);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_ResetDynamicKeyPoint()
    {
    m_manipulationStrategy->ResetDynamicKeyPoint();
    }