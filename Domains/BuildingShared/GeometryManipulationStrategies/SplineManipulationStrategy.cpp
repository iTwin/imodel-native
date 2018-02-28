/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/SplineManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

#define DEFAULT_ORDER 3

/////////////////////////////////////////////////////////////////////////////////////////
// SplineManipulationStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
SplineManipulationStrategyPtr SplineManipulationStrategy::Create
(
    SplinePlacementStrategyType placementStrategy
)
    {
    switch (placementStrategy)
        {
        case SplinePlacementStrategyType::ControlPoints:
            return SplineControlPointsManipulationStrategy::Create(DEFAULT_ORDER);
        case SplinePlacementStrategyType::ThroughPoints:
            return SplineThroughPointsManipulationStrategy::Create();
        default:
            BeAssert(false);
            return nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
SplinePlacementStrategyPtr SplineManipulationStrategy::CreatePlacement()
    {
    return _CreatePlacement();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
SplinePlacementStrategyType SplineManipulationStrategy::GetType() const 
    {
        return _GetType();
    }

/////////////////////////////////////////////////////////////////////////////////////////
// SplineControlPointsManipulationStrategy
/////////////////////////////////////////////////////////////////////////////////////////
const int SplineControlPointsManipulationStrategy::default_Order = 3;

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SplineControlPointsManipulationStrategy::_IsComplete() const
    {
    return _GetKeyPoints().size() >= m_order;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr SplineControlPointsManipulationStrategy::_FinishPrimitive() const
    {
    bvector<DPoint3d> poles = _GetKeyPoints();

    bvector<double> weights;
    for (DPoint3d point : poles)
        weights.push_back(1.0);

    int order = poles.size() < m_order ? poles.size() : m_order; // order can't be higher than poles size

    bvector<double> knots;
    for (int i = 0; i < order + poles.size(); ++i)
        knots.push_back(i);

    MSBsplineCurvePtr bspline = MSBsplineCurve::CreateFromPolesAndOrder(poles, &weights, &knots, order, false, false);
    if (bspline.IsNull())
        return nullptr;

    return ICurvePrimitive::CreateBsplineCurve(bspline);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr SplineControlPointsManipulationStrategy::_CreateDefaultPlacementStrategy() 
    {
    return SplineControlPointsPlacementStrategy::Create(*this); 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
SplinePlacementStrategyPtr SplineControlPointsManipulationStrategy::_CreatePlacement()
    {
    return SplineControlPointsPlacementStrategy::Create(*this);
    }

/////////////////////////////////////////////////////////////////////////////////////////
// SplineThroughPointsManipulationStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SplineThroughPointsManipulationStrategy::_IsComplete() const
    {
    return _GetKeyPoints().size() >= 2;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr SplineThroughPointsManipulationStrategy::_FinishPrimitive() const
    {
    MSInterpolationCurvePtr curve = MSInterpolationCurve::CreatePtr();
    if (curve.IsNull())
        return nullptr;

    bvector<DPoint3d> poles = _GetKeyPoints();
    if (poles.size() < 2)
        return nullptr;

    DVec3d tangents[2] = { m_startTangent, m_endTangent };

    if (SUCCESS != curve->InitFromPointsAndEndTangents(poles, false, 0.0, tangents, false, false, false, false))
        return nullptr;

    return ICurvePrimitive::CreateInterpolationCurveSwapFromSource(*curve);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr SplineThroughPointsManipulationStrategy::_CreateDefaultPlacementStrategy() 
    { 
    return SplineThroughPointsPlacementStrategy::Create(*this); 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SplineThroughPointsManipulationStrategy::SetStartTangent(DVec3d startTangent) 
    {
    startTangent.Normalize();
    _SetStartTangent(startTangent); 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SplineThroughPointsManipulationStrategy::SetEndTangent(DVec3d endTangent) 
    { 
    endTangent.Normalize();
    _SetEndTangent(endTangent); 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
SplinePlacementStrategyPtr SplineThroughPointsManipulationStrategy::_CreatePlacement()
    {
    return SplineThroughPointsPlacementStrategy::Create(*this);
    }
