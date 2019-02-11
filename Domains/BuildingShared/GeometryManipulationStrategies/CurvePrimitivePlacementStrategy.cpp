/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurvePrimitivePlacementStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitivePlacementStrategy::_CopyKeyPointsTo
(
    ArcPlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.empty())
        return;

    switch (other.GetPlacementMethod())
        {
        case ArcPlacementMethod::StartCenter:
        case ArcPlacementMethod::StartEndMid:
        case ArcPlacementMethod::StartMidEnd:
            {
            other.AddKeyPoint(keyPoints.front());
            }
        default:
            break;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitivePlacementStrategy::_CopyKeyPointsTo
(
    LinePlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.empty())
        return;

    other.AddKeyPoint(keyPoints.front());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitivePlacementStrategy::_CopyKeyPointsTo
(
    LineStringPlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.empty())
        return;

    for (DPoint3dCR keyPoint : keyPoints)
        other.AddKeyPoint(keyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitivePlacementStrategy::_CopyKeyPointsTo
(
    SplineControlPointsPlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.empty())
        return;

    for (DPoint3dCR keyPoint : keyPoints)
        other.AddKeyPoint(keyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitivePlacementStrategy::_CopyKeyPointsTo
(
    SplineThroughPointsPlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.empty())
        return;

    for (DPoint3dCR keyPoint : keyPoints)
        other.AddKeyPoint(keyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr CurvePrimitivePlacementStrategy::_FinishPrimitive() const
    {
    CurvePrimitiveManipulationStrategyCR strategy = dynamic_cast<CurvePrimitiveManipulationStrategyCR>(GetManipulationStrategy());
    return strategy.FinishPrimitive();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr CurvePrimitivePlacementStrategy::FinishPrimitive() const
    {
    return _FinishPrimitive();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitivePlacementStrategy::_IsContinious() const
    {
    return _GetCurvePrimitiveManipulationStrategy().IsContinious();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitivePlacementStrategy::IsEmpty() const
    {
    return _IsEmpty();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitivePlacementStrategy::_IsEmpty() const
    {
    return _GetCurvePrimitiveManipulationStrategy().IsEmpty();
    }
