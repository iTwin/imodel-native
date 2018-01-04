/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurveVectorManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

#define DEFAULT_MANIPULATION_STRATEGY ArcManipulationStrategy::Create()

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorManipulationStrategy::_Finish() const
    {
    BeAssert(false && "Not implemented.");
    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorManipulationStrategy::Finish() const
    {
    return _Finish();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorManipulationStrategy::_IsComplete() const
    {
    return !m_primitiveStrategies.empty() &&
        std::none_of(m_primitiveStrategies.begin(), m_primitiveStrategies.end(),
                    [] (CurvePrimitiveManipulationStrategyPtr const& s) { return !s->IsComplete(); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> CurveVectorManipulationStrategy::_GetKeyPoints() const
    {
    bvector<DPoint3d> allKeyPoints;

    for (CurvePrimitiveManipulationStrategyPtr const& strategy : m_primitiveStrategies)
        {
        bvector<DPoint3d> const& strategyKeyPoints = strategy->GetKeyPoints();
        allKeyPoints.insert(allKeyPoints.end(), strategyKeyPoints.begin(), strategyKeyPoints.end());
        }

    return allKeyPoints;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorManipulationStrategy::_IsDynamicKeyPointSet() const
    {
    return std::any_of(m_primitiveStrategies.rbegin(), m_primitiveStrategies.rend(),
                       [] (CurvePrimitiveManipulationStrategyPtr const& s) { return s->IsDynamicKeyPointSet(); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_ResetDynamicKeyPoint()
    {
    for (CurvePrimitiveManipulationStrategyPtr const& strategy : m_primitiveStrategies)
        {
        strategy->ResetDynamicKeyPoint();
        }

    std::remove_if(m_primitiveStrategies.begin(), m_primitiveStrategies.end(),
                   [] (CurvePrimitiveManipulationStrategyPtr const& strategy) { return strategy->GetKeyPoints().empty(); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_PopKeyPoint()
    {
    if (m_primitiveStrategies.empty())
        return;

    if (m_primitiveStrategies.size() > 1 && m_primitiveStrategies.back()->GetAcceptedKeyPoints().empty())
        {
        m_primitiveStrategies.pop_back();
        }

    m_primitiveStrategies.back()->PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr CurveVectorManipulationStrategy::GetStrategyForAppend()
    {
    CurvePrimitiveManipulationStrategyPtr manipulationStrategy;

    if (!m_primitiveStrategies.empty() && m_primitiveStrategies.back()->CanAcceptMorePoints())
        manipulationStrategy = m_primitiveStrategies.back();
    else
        {
        manipulationStrategy = DEFAULT_MANIPULATION_STRATEGY;
        if (!m_primitiveStrategies.empty())
            {
            bvector<DPoint3d> lastStrategyAcceptedKeyPoints = m_primitiveStrategies.back()->GetAcceptedKeyPoints();
            if (!lastStrategyAcceptedKeyPoints.empty())
                manipulationStrategy->AppendKeyPoint(lastStrategyAcceptedKeyPoints.back());
            }
        m_primitiveStrategies.push_back(manipulationStrategy);
        }

    return manipulationStrategy->CreateDefaultPlacementStrategy();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    GetStrategyForAppend()->AddKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    GetStrategyForAppend()->AddKeyPoint(newDynamicKeyPoint);
    }