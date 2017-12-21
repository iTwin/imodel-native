/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryManipulationStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>
#include "PublicApi/GeometryManipulationStrategyBase.h"
#include "PublicApi/GeometryManipulationStrategy.h"

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
    BeAssert(m_keyPoints.size() > index);
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
    BeAssert(m_keyPoints.size() > index);
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
    BeAssert(m_keyPoints.size() >= index);
    m_dynamicKeyPointSet = true;

    m_keyPointsWithDynamicKeyPoint = m_keyPoints;
    if (DynamicKeyPointType::Inserted == type)
        {
        if (m_keyPointsWithDynamicKeyPoint.size() == index)
            {
            m_keyPointsWithDynamicKeyPoint.push_back(newDynamicKeyPoint);
            }
        else
            {
            m_keyPointsWithDynamicKeyPoint.insert(&m_keyPointsWithDynamicKeyPoint[index], newDynamicKeyPoint);
            }
        }
    else if (DynamicKeyPointType::Updated == type)
        m_keyPointsWithDynamicKeyPoint[index] = newDynamicKeyPoint;
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