/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
SplineControlPointsManipulationStrategy::SplineControlPointsManipulationStrategy
(
    int order
) 
    : T_Super()
    , m_order(order) 
    {
    RegisterIntProperty(prop_Order());
    }

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

    int order = poles.size() < m_order ? static_cast<int>(poles.size()) : m_order; // order can't be higher than poles size

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

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void computeInterpolationCurveTangentPoints(DPoint3dR startTangentPt, DPoint3dR endTangentPt, MSInterpolationCurveCR curve)
    {
    startTangentPt.SumOf(curve.fitPoints[0], curve.startTangent, curve.fitPoints[0].Distance(curve.fitPoints[1]) * 0.5);
    endTangentPt.SumOf(curve.fitPoints[curve.params.numPoints-1], curve.endTangent, curve.fitPoints[curve.params.numPoints-1].Distance(curve.fitPoints[curve.params.numPoints-2]) * 0.5);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ConstructionGeometry> SplineControlPointsManipulationStrategy::_FinishConstructionGeometry() const
    {
    bvector<ConstructionGeometry> constructionGeometry;

    ICurvePrimitivePtr curve = _FinishPrimitive();
    if (curve.IsNull())
        return constructionGeometry;

    MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP();

    if (nullptr == bcurve || bcurve->GetIntOrder() < 1 || bcurve->GetIntNumPoles() < 1)
        return constructionGeometry;

    bvector<DPoint3d> poles;

    bcurve->GetUnWeightedPoles(poles);

    if (0 == poles.size())
        return constructionGeometry;

    constructionGeometry.push_back(ConstructionGeometry(*IGeometry::Create(ICurvePrimitive::CreatePointString(poles)), CONSTRUCTION_GEOMTYPE_SplinePolePoints));

    if (bcurve->params.closed)
        poles.push_back(poles.front());

    constructionGeometry.push_back(ConstructionGeometry(*IGeometry::Create(ICurvePrimitive::CreateLineString(poles)), CONSTRUCTION_GEOMTYPE_SplinePoleLines));

    return constructionGeometry;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void SplineControlPointsManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    int const& value
)
    {
    if (0 == strcmp(key, prop_Order()))
        {
        m_order = value;
        }

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SplineControlPointsManipulationStrategy::_TryGetProperty
(
    Utf8CP key,
    int& value
) const
    {
    if (0 == strcmp(key, prop_Order()))
        {
        value = m_order;
        return BSISUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }

/////////////////////////////////////////////////////////////////////////////////////////
// SplineThroughPointsManipulationStrategy
/////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
SplineThroughPointsManipulationStrategy::SplineThroughPointsManipulationStrategy() 
    : T_Super()
    {
    RegisterDVec3dProperty(prop_StartTangent());
    RegisterDVec3dProperty(prop_EndTangent());
    }

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

    DVec3d tangents[2] = {m_startTangent, m_endTangent};

    for (DVec3d & tangent : tangents)
        {
        if (!tangent.IsZero())
            tangent.Normalize();
        }

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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ConstructionGeometry> SplineThroughPointsManipulationStrategy::_FinishConstructionGeometry() const
    {
    ICurvePrimitivePtr curve = _FinishPrimitive();
    if (curve.IsNull())
        return bvector<ConstructionGeometry>();

    MSInterpolationCurveCP fitCurve = curve->GetInterpolationCurveCP();

    bvector<ConstructionGeometry> constructionGeometry;
    if (nullptr != fitCurve)
        {
        constructionGeometry.push_back(ConstructionGeometry(*IGeometry::Create(ICurvePrimitive::CreatePointString(fitCurve->fitPoints, fitCurve->params.numPoints)), CONSTRUCTION_GEOMTYPE_SplineFitPoints));

        if (!fitCurve->params.isPeriodic)
            {
            DPoint3d tangentPoints[4];

            // Compute interpolation curve tangent points...
            computeInterpolationCurveTangentPoints(tangentPoints[0], tangentPoints[2], *fitCurve);

            // Display fat dots for start/end tangent points...
            constructionGeometry.push_back(ConstructionGeometry(*IGeometry::Create(ICurvePrimitive::CreatePointString(&tangentPoints[0], 1)), CONSTRUCTION_GEOMTYPE_SplineStartTangentPoint));
            constructionGeometry.push_back(ConstructionGeometry(*IGeometry::Create(ICurvePrimitive::CreatePointString(&tangentPoints[2], 1)), CONSTRUCTION_GEOMTYPE_SplineEndTangentPoint));

            // Display dotted style start/end tangent lines...
            tangentPoints[1] = fitCurve->fitPoints[0];
            tangentPoints[3] = fitCurve->fitPoints[fitCurve->params.numPoints - 1];

            constructionGeometry.push_back(ConstructionGeometry(*IGeometry::Create(ICurvePrimitive::CreateLineString(&tangentPoints[0], 2)), CONSTRUCTION_GEOMTYPE_SplineStartTangentLine));
            constructionGeometry.push_back(ConstructionGeometry(*IGeometry::Create(ICurvePrimitive::CreateLineString(&tangentPoints[2], 2)), CONSTRUCTION_GEOMTYPE_SplineEndTangentLine));
            }
        }

    return constructionGeometry;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void SplineThroughPointsManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    DVec3d const& value
)
    {
    if (0 == strcmp(key, prop_StartTangent()))
        {
        m_startTangent = value;
        }
    else if (0 == strcmp(key, prop_EndTangent()))
        {
        m_endTangent = value;
        }

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SplineThroughPointsManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    DVec3d& value
) const
    {
    if (0 == strcmp(key, prop_StartTangent()))
        {
        value = m_startTangent;
        return BSISUCCESS;
        }
    else if (0 == strcmp(key, prop_EndTangent()))
        {
        value = m_endTangent;
        return BSISUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }