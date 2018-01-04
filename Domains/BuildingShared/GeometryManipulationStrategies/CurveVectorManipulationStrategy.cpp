/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurveVectorManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

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