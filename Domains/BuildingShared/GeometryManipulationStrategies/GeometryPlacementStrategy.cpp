/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    m_manipulationStrategy->_ResetDynamicKeyPoint();
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
    m_manipulationStrategy->_ResetDynamicKeyPoint();
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
void GeometryPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    m_manipulationStrategy->AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_AddDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    m_manipulationStrategy->AppendDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    m_manipulationStrategy->_ResetDynamicKeyPoint();
    _AddDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::AddDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    m_manipulationStrategy->_ResetDynamicKeyPoint();
    _AddDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_ResetDynamicKeyPoint()
    {
    m_manipulationStrategy->ResetDynamicKeyPoint();
    }