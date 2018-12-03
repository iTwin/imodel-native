/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PointManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
PointManipulationStrategyPtr PointManipulationStrategy::Create
(
    DPoint3dCR point
)
    {
    PointManipulationStrategyPtr strategy = Create();
    strategy->AppendKeyPoint(point);
    return strategy;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
void PointManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    if (!m_nonDynamicKeypointSet)
        {
        _ReplaceKeyPoint(newKeyPoint, 0);
        m_nonDynamicKeypointSet = true;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
bool PointManipulationStrategy::_IsComplete() const
    {
    return m_nonDynamicKeypointSet;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
bool PointManipulationStrategy::_CanAcceptMorePoints() const
    {
    return !_IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> PointManipulationStrategy::_GetKeyPoints() const
    {
    return m_keyPoints;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<IGeometryPtr> PointManipulationStrategy::_FinishConstructionGeometry() const
    {
    return{ _FinishGeometry() };
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
void PointManipulationStrategy::_AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint)
    {
    if (!m_nonDynamicKeypointSet)
        _ReplaceKeyPoint(newDynamicKeyPoint, 0);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
void PointManipulationStrategy::_ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index)
    {
    _PopKeyPoint();
    m_keyPoints.push_back(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
bool PointManipulationStrategy::_IsDynamicKeyPointSet() const
    {
    if (!m_nonDynamicKeypointSet && !m_keyPoints.empty())
        return true;
    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
void PointManipulationStrategy::_ResetDynamicKeyPoint()
    {
    if (m_keyPoints.empty())
        return;

    if (_IsDynamicKeyPointSet())
        m_keyPoints.pop_back();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
void PointManipulationStrategy::_PopKeyPoint()
    {
    m_nonDynamicKeypointSet = false;
    if (m_keyPoints.empty())
        return;

    m_keyPoints.pop_back();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
DynamicStateBaseCPtr PointManipulationStrategy::_GetDynamicState() const 
    { 
    return DynamicStateBaseCPtr(BooleanDynamicState::Create(_IsDynamicKeyPointSet()));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr PointManipulationStrategy::_FinishGeometry() const
    {
    if (m_keyPoints.empty())
        return nullptr;

    return IGeometry::Create(ICurvePrimitive::CreatePointString(&m_keyPoints[0], 1));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr PointManipulationStrategy::_FinishPrimitive() const
    {
    bvector<DPoint3d> keyPoints = m_keyPoints;
    if (keyPoints.size() < 1)
        return nullptr;

    return ICurvePrimitive::CreatePointString(keyPoints);
    }