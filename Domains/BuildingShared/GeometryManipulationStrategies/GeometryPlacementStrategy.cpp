/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryPlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

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
void GeometryPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    m_manipulationStrategy->AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _AddKeyPoint(newKeyPoint);
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
void GeometryPlacementStrategy::_PopKeyPoint()
    {
    m_manipulationStrategy->PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::PopKeyPoint()
    {
    _PopKeyPoint();
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
void GeometryPlacementStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    m_manipulationStrategy->AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    m_manipulationStrategy->AppendDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_InsertDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    m_manipulationStrategy->InsertDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_InsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    m_manipulationStrategy->InsertDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    m_manipulationStrategy->UpdateDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_UpdateDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    m_manipulationStrategy->UpdateDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_ResetDynamicKeyPoint()
    {
    m_manipulationStrategy->ResetDynamicKeyPoint();
    }