/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ChildCurveVectorManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ChildCurveVectorManipulationStrategyPtr ChildCurveVectorManipulationStrategy::Create
(
    CurveVectorCR cv
)
    {
    ChildCurveVectorManipulationStrategyPtr strategy = Create();
    strategy->m_cvManipulationStrategy->Init(cv);
    strategy->m_boundaryType = cv.GetBoundaryType();
    return strategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ChildCurveVectorManipulationStrategy::ChildCurveVectorManipulationStrategy()
    : T_Super()
    , m_cvManipulationStrategy(CurveVectorManipulationStrategy::Create())
    , m_boundaryType(CurveVector::BOUNDARY_TYPE_Outer)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr ChildCurveVectorManipulationStrategy::_FinishPrimitive() const
    {
    CurveVectorPtr child = m_cvManipulationStrategy->Finish();
    if (child.IsNull())
        return nullptr;

    return ICurvePrimitive::CreateChildCurveVector(child);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ChildCurveVectorManipulationStrategy::_IsComplete() const 
    {
    return m_cvManipulationStrategy->IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ChildCurveVectorManipulationStrategy::_CanAcceptMorePoints() const 
    {
    return m_cvManipulationStrategy->CanAcceptMorePoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitiveManipulationStrategyPtr ChildCurveVectorManipulationStrategy::_Clone() const
    {
    return Create();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr ChildCurveVectorManipulationStrategy::_CreateDefaultPlacementStrategy()
    {
    return ChildCurveVectorPlacementStrategy::Create(*this);
    }