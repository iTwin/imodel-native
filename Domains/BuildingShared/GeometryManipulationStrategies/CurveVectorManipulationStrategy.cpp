/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurveVectorManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

#define DEFAULT_MANIPULATION_STRATEGY LineManipulationStrategy::Create()

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorManipulationStrategy::_Finish() const
    {
    CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    
    for (CurvePrimitiveManipulationStrategyPtr const& strategy : m_primitiveStrategies)
        {
        ICurvePrimitivePtr primitive = strategy->FinishPrimitive();
        if (primitive.IsNull())
            continue;

        cv->Add(primitive);
        }

    if (cv->empty())
        return nullptr;

    return cv;
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
bool CurveVectorManipulationStrategy::IsLastStrategyReadyForPop() const
    {
    if (m_primitiveStrategies.empty())
        return false;

    if (m_primitiveStrategies.size() == 1 && m_primitiveStrategies.back()->IsEmpty())
        return true;

    if (m_primitiveStrategies.size() > 1)
        {
        CurvePrimitiveManipulationStrategyR lastStrategy = *m_primitiveStrategies.back();
        if (lastStrategy.IsEmpty())
            return true;

        CurvePrimitiveManipulationStrategyR secondToLastStrategy = *m_primitiveStrategies[m_primitiveStrategies.size() - 2];
        if (lastStrategy.IsSingleKeyPointLeft() &&
            lastStrategy.GetFirstKeyPoint().AlmostEqual(secondToLastStrategy.GetLastKeyPoint()))
            return true;
        }

    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_PopKeyPoint()
    {
    if (m_primitiveStrategies.empty())
        return;

    m_primitiveStrategies.back()->CreateDefaultPlacementStrategy()->PopKeyPoint();

    if (IsLastStrategyReadyForPop())
        {
        m_primitiveStrategies.pop_back();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr CurveVectorManipulationStrategy::GetStrategyForAppend()
    {
    CurvePrimitivePlacementStrategyPtr placementStrategy;

    if (!m_primitiveStrategies.empty() && m_primitiveStrategies.back()->CanAcceptMorePoints())
        placementStrategy = m_primitiveStrategies.back()->CreateDefaultPlacementStrategy();
    else
        {
        CurvePrimitiveManipulationStrategyPtr manipulationStrategy;

        switch (m_geometryType)
            {
            case GeometryType::Arc:
                manipulationStrategy = ArcManipulationStrategy::Create();
                break;
            case GeometryType::Line:
                manipulationStrategy = LineManipulationStrategy::Create();
                break;
            default:
                if (m_primitiveStrategies.empty())
                    manipulationStrategy = DEFAULT_MANIPULATION_STRATEGY;
                else
                    manipulationStrategy = m_primitiveStrategies.back()->Clone();
                break;
            }

        BeAssert(manipulationStrategy.IsValid());

        placementStrategy = manipulationStrategy->CreateDefaultPlacementStrategy();
        if (!m_primitiveStrategies.empty())
            {
            bvector<DPoint3d> lastStrategyAcceptedKeyPoints = m_primitiveStrategies.back()->GetAcceptedKeyPoints();
            if (!lastStrategyAcceptedKeyPoints.empty())
                placementStrategy->AddKeyPoint(lastStrategyAcceptedKeyPoints.back());
            }
        m_primitiveStrategies.push_back(manipulationStrategy);
        }

    return placementStrategy;
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::ChangeGeometryType
(
    GeometryType newGeometryType
)
    {
    if (m_geometryType == newGeometryType)
        return;

    m_geometryType = newGeometryType;
    }