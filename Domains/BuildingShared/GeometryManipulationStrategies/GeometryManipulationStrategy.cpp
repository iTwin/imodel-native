/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
GeometryManipulationStrategy::GeometryManipulationStrategy()
    : T_Super()
    , m_dynamicKeyPointSet(false)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    m_keyPoints.push_back(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _ResetDynamicKeyPoint();
    _AppendKeyPoint(newKeyPoint);
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::InsertKeyPoint
(
    DPoint3dCR newKeyPoint,
    size_t index
)
    {
    if (m_keyPoints.size() >= index)
        {
        _ResetDynamicKeyPoint();
        if (m_keyPoints.size() > index)
            m_keyPoints.insert(&m_keyPoints[index], newKeyPoint);
        else
            _AppendKeyPoint(newKeyPoint);
        }
    else
        BeAssert(false);

    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::ReplaceKeyPoint
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

    _ResetDynamicKeyPoint();
    m_keyPoints[index] = newKeyPoint;
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::PopKeyPoint()
    {
    _ResetDynamicKeyPoint();
    m_keyPoints.pop_back();
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::RemoveKeyPoint
(
    size_t index
)
    {
    if (m_keyPoints.size() <= index)
        {
        BeAssert(false);
        return;
        }

    _ResetDynamicKeyPoint();
    m_keyPoints.erase(&m_keyPoints[index]);
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
    m_keyPointsWithDynamicKeyPoint.push_back(newDynamicKeyPoint);
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
    for (DPoint3dCR newDynamicKeyPoint : newDynamicKeyPoints)
        m_keyPointsWithDynamicKeyPoint.push_back(newDynamicKeyPoint);
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::_InsertDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
) 
    {
    if (m_keyPoints.size() < index)
        {
        BeAssert(false);
        return;
        }

    if (m_keyPoints.size() == index)
        {
        _AppendDynamicKeyPoint(newDynamicKeyPoint);
        return;
        }

    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
    m_keyPointsWithDynamicKeyPoint.insert(&m_keyPointsWithDynamicKeyPoint[index], newDynamicKeyPoint);
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::_InsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    if (m_keyPoints.size() < index)
        {
        BeAssert(false);
        return;
        }

    if (m_keyPoints.size() == index)
        {
        _AppendDynamicKeyPoints(newDynamicKeyPoints);
        return;
        }

    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
    for(size_t i = index; i-index < newDynamicKeyPoints.size(); ++i)
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
void GeometryManipulationStrategy::_UpsertDynamicKeyPoint
(
    DPoint3d newDynamicKeyPoint, 
    size_t index
)
    {
    if (m_keyPoints.size() == index)
        _AppendDynamicKeyPoint(newDynamicKeyPoint);
    else
        _UpdateDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::_UpsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    if (index > m_keyPoints.size())
        {
        BeAssert(false);
        return;
        }

    if (newDynamicKeyPoints.size() == 1)
        {
        _UpsertDynamicKeyPoint(newDynamicKeyPoints.front(), index);
        return;
        }

    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
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
void GeometryManipulationStrategy::_UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
) 
    {
    if (m_keyPoints.size() <= index)
        {
        BeAssert(false);
        return;
        }

    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
    m_keyPointsWithDynamicKeyPoint[index] = newDynamicKeyPoint;
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::_UpdateDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    if (m_keyPoints.size() < index + newDynamicKeyPoints.size())
        {
        BeAssert(false);
        return;
        }

    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
    for (size_t i = 0; i < newDynamicKeyPoints.size(); ++i)
        m_keyPointsWithDynamicKeyPoint[i + index] = newDynamicKeyPoints[i];
    m_dynamicKeyPointSet = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::_ResetDynamicKeyPoint()
    {
    m_dynamicKeyPointSet = false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> const& GeometryManipulationStrategy::_GetKeyPoints() const
    {
    if (_IsDynamicKeyPointSet())
        return m_keyPointsWithDynamicKeyPoint;
    
    return m_keyPoints;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _ResetDynamicKeyPoint();
    _AppendDynamicKeyPoint(newDynamicKeyPoint);
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    _ResetDynamicKeyPoint();
    _AppendDynamicKeyPoints(newDynamicKeyPoints);
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::InsertDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    _ResetDynamicKeyPoint();
    _InsertDynamicKeyPoint(newDynamicKeyPoint, index);
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::InsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    _ResetDynamicKeyPoint();
    _InsertDynamicKeyPoints(newDynamicKeyPoints, index);
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    _ResetDynamicKeyPoint();
    _UpdateDynamicKeyPoint(newDynamicKeyPoint, index);
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::UpdateDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    _ResetDynamicKeyPoint();
    _UpdateDynamicKeyPoints(newDynamicKeyPoints, index);
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::UpsertDynamicKeyPoint
(
    DPoint3d newDynamicKeyPoint,
    size_t index
)
    {
    _ResetDynamicKeyPoint();
    _UpsertDynamicKeyPoint(newDynamicKeyPoint, index);
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::UpsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    _ResetDynamicKeyPoint();
    _UpsertDynamicKeyPoints(newDynamicKeyPoints, index);
    _OnKeyPointsChanged();
    }