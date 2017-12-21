/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryManipulationStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
void GeometryManipulationStrategy::AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    m_keyPoints.push_back(newKeyPoint);
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
    if (m_keyPoints.size() > index)
        m_keyPoints.insert(&m_keyPoints[index], newKeyPoint);
    else if (m_keyPoints.size() == index)
        AppendKeyPoint(newKeyPoint);
    else
        BeAssert(false);
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

    m_keyPoints[index] = newKeyPoint;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::PopKeyPoint()
    {
    m_keyPoints.pop_back();
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

    m_keyPoints.erase(&m_keyPoints[index]);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::_SetDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index,
    GeometryManipulationStrategyBase::DynamicKeyPointType type
)
    {
    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
    if (DynamicKeyPointType::Insert == type)
        {
        if (m_keyPointsWithDynamicKeyPoint.size() == index)
            {
            m_keyPointsWithDynamicKeyPoint.push_back(newDynamicKeyPoint);
            }
        else if (m_keyPointsWithDynamicKeyPoint.size() > index)
            {
            m_keyPointsWithDynamicKeyPoint.insert(&m_keyPointsWithDynamicKeyPoint[index], newDynamicKeyPoint);
            }
        else
            {
            BeAssert(false);
            return;
            }
        }
    else if (DynamicKeyPointType::Update == type && m_keyPointsWithDynamicKeyPoint.size() > index)
        m_keyPointsWithDynamicKeyPoint[index] = newDynamicKeyPoint;
    else
        {
        BeAssert(false);
        return;
        }

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