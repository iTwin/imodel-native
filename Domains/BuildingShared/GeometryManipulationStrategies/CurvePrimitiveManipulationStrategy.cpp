/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_SetDynamicState
(
    DynamicStateBaseCR state
)
    {
    BooleanDynamicStateCP booleanState = dynamic_cast<BooleanDynamicStateCP>(&state);
    if (nullptr == booleanState)
        return;

    m_dynamicKeyPointSet = booleanState->GetState();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
DynamicStateBaseCPtr CurvePrimitiveManipulationStrategy::_GetDynamicState() const
    {
    return DynamicStateBaseCPtr(BooleanDynamicState::Create(m_dynamicKeyPointSet));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr CurvePrimitiveManipulationStrategy::FinishPrimitive() const
    {
    return _FinishPrimitive();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    m_keyPoints.push_back(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_AppendKeyPoints
(
    bvector<DPoint3d> const& newKeyPoints
)
    {
    for (DPoint3dCR keyPoint : newKeyPoints)
        _AppendKeyPoint(keyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_InsertKeyPoint
(
    DPoint3dCR newKeyPoint,
    size_t index
)
    {
    if (m_keyPoints.size() >= index)
        {
        if (m_keyPoints.size() > index)
            m_keyPoints.insert(&m_keyPoints[index], newKeyPoint);
        else
            _AppendKeyPoint(newKeyPoint);
        }
    else
        BeAssert(false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_ReplaceKeyPoint
(
    DPoint3dCR newKeyPoint,
    size_t index
)
    {
    if (m_keyPoints.size() <= index)
        {
        BeAssert(false);
        return;
        }

    m_keyPoints[index] = newKeyPoint;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_PopKeyPoint()
    {
    if (m_keyPoints.empty())
        return;

    m_keyPoints.pop_back();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_RemoveKeyPoint
(
    size_t index
)
    {
    if (m_keyPoints.size() <= index)
        {
        BeAssert(false);
        return;
        }

    m_keyPoints.erase(&m_keyPoints[index]);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_Clear()
    {
    m_keyPoints.clear();
    m_keyPointsWithDynamicKeyPoint.clear();
    m_dynamicKeyPointSet = false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    m_keyPointsWithDynamicKeyPoint.push_back(newDynamicKeyPoint);
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    for (DPoint3dCR newDynamicKeyPoint : newDynamicKeyPoints)
        m_keyPointsWithDynamicKeyPoint.push_back(newDynamicKeyPoint);
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_InsertDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    if (m_keyPointsWithDynamicKeyPoint.size() < index)
        {
        BeAssert(false);
        return;
        }

    if (m_keyPointsWithDynamicKeyPoint.size() == index)
        {
        _AppendDynamicKeyPoint(newDynamicKeyPoint);
        return;
        }

    m_keyPointsWithDynamicKeyPoint.insert(&m_keyPointsWithDynamicKeyPoint[index], newDynamicKeyPoint);
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_InsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    if (m_keyPointsWithDynamicKeyPoint.size() < index)
        {
        BeAssert(false);
        return;
        }

    if (m_keyPointsWithDynamicKeyPoint.size() == index)
        {
        _AppendDynamicKeyPoints(newDynamicKeyPoints);
        return;
        }

    for (size_t i = index; i - index < newDynamicKeyPoints.size(); ++i)
        {
        if (m_keyPointsWithDynamicKeyPoint.size() == i)
            m_keyPointsWithDynamicKeyPoint.push_back(newDynamicKeyPoints[i - index]);
        else
            m_keyPointsWithDynamicKeyPoint.insert(&m_keyPointsWithDynamicKeyPoint[i], newDynamicKeyPoints[i - index]);
        }
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_UpsertDynamicKeyPoint
(
    DPoint3d newDynamicKeyPoint,
    size_t index
)
    {
    if (m_keyPointsWithDynamicKeyPoint.size() == index)
        _AppendDynamicKeyPoint(newDynamicKeyPoint);
    else
        _UpdateDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_UpsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    if (index > m_keyPointsWithDynamicKeyPoint.size())
        {
        BeAssert(false);
        return;
        }

    if (newDynamicKeyPoints.size() == 1)
        {
        _UpsertDynamicKeyPoint(newDynamicKeyPoints.front(), index);
        return;
        }

    for (size_t i = index; i - index < newDynamicKeyPoints.size(); ++i)
        {
        if (m_keyPointsWithDynamicKeyPoint.size() == i)
            m_keyPointsWithDynamicKeyPoint.push_back(newDynamicKeyPoints[i - index]); // append
        else
            m_keyPointsWithDynamicKeyPoint[i] = newDynamicKeyPoints[i - index]; // update
        }
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    if (m_keyPointsWithDynamicKeyPoint.size() <= index)
        {
        BeAssert(false);
        return;
        }

    m_keyPointsWithDynamicKeyPoint[index] = newDynamicKeyPoint;
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_UpdateDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    if (m_keyPointsWithDynamicKeyPoint.size() < index + newDynamicKeyPoints.size())
        {
        BeAssert(false);
        return;
        }

    for (size_t i = 0; i < newDynamicKeyPoints.size(); ++i)
        m_keyPointsWithDynamicKeyPoint[i + index] = newDynamicKeyPoints[i];
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveManipulationStrategy::_ResetDynamicKeyPoint()
    {
    m_dynamicKeyPointSet = false;
    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> CurvePrimitiveManipulationStrategy::_GetKeyPoints() const
    {
    if (_IsDynamicKeyPointSet())
        return m_keyPointsWithDynamicKeyPoint;

    return m_keyPoints;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr CurvePrimitiveManipulationStrategy::CreateDefaultPlacementStrategy()
    {
    return _CreateDefaultPlacementStrategy();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitiveManipulationStrategy::_IsEmpty() const 
    {
    return _GetKeyPoints().empty(); 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitiveManipulationStrategy::_IsSingleKeyPointLeft() const 
    { 
    return _GetKeyPoints().size() == 1; 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d CurvePrimitiveManipulationStrategy::_GetLastKeyPoint() const
    {
    return _GetKeyPoints().back();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d CurvePrimitiveManipulationStrategy::_GetFirstKeyPoint() const
    {
    return _GetKeyPoints().front();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitiveManipulationStrategy::IsEmpty() const
    {
    return _IsEmpty();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitiveManipulationStrategy::IsSingleKeyPointLeft() const
    {
    return _IsSingleKeyPointLeft();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d CurvePrimitiveManipulationStrategy::GetLastKeyPoint() const
    {
    return _GetLastKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d CurvePrimitiveManipulationStrategy::GetFirstKeyPoint() const
    {
    return _GetFirstKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitiveManipulationStrategyPtr CurvePrimitiveManipulationStrategy::Clone() const
    {
    return _Clone();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LinePlacementStrategyPtr CurvePrimitiveManipulationStrategy::CreateLinePlacementStrategy
(
    LinePlacementStrategyType strategyType
)
    {
    return _CreateLinePlacementStrategy(strategyType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategyPtr CurvePrimitiveManipulationStrategy::CreateArcPlacementStrategy
(
    ArcPlacementMethod method
)
    {
    return _CreateArcPlacementStrategy(method);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineStringPlacementStrategyPtr CurvePrimitiveManipulationStrategy::CreateLineStringPlacementStrategy
(
    LineStringPlacementStrategyType strategyType
)
    {
    return _CreateLineStringPlacementStrategy(strategyType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
SplinePlacementStrategyPtr CurvePrimitiveManipulationStrategy::CreateSplinePlacementStrategy
(
    SplinePlacementStrategyType strategyType
)
    {
    return _CreateSplinePlacementStrategy(strategyType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitiveManipulationStrategy::IsContinious() const
    {
    return _IsContinious();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr CurvePrimitiveManipulationStrategy::_FinishGeometry() const
    {
    ICurvePrimitivePtr primitive = _FinishPrimitive();
    if (primitive.IsNull())
        return nullptr;

    return IGeometry::Create(primitive);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitive::CurvePrimitiveType CurvePrimitiveManipulationStrategy::GetResultCurvePrimitiveType() const
    {
    return _GetResultCurvePrimitiveType();
    }