/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitive::CurvePrimitiveType CurvePrimitivePlacementStrategy::GetResultCurvePrimitiveType() const
    {
    return _GetCurvePrimitiveManipulationStrategy().GetResultCurvePrimitiveType();
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitivePlacementStrategy::PrepareReplacement
(
    CurvePrimitivePlacementStrategyR replacement
)
    {
    ScopedDynamicKeyPointResetter dynamicKeyPointResetter(*this);
    CopyPropertiesTo(replacement);

    switch (replacement.GetResultCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            ArcPlacementStrategyP arcStrategy = dynamic_cast<ArcPlacementStrategyP>(&replacement);
            if (nullptr == arcStrategy)
                {
                BeAssert(nullptr != arcStrategy);
                return;
                }
            _CopyKeyPointsTo(*arcStrategy);
            break;
            }
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            {
            SplineControlPointsPlacementStrategyP bsplineStrategy = dynamic_cast<SplineControlPointsPlacementStrategyP>(&replacement);
            if (nullptr == bsplineStrategy)
                {
                BeAssert(nullptr != bsplineStrategy);
                return;
                }
            _CopyKeyPointsTo(*bsplineStrategy);
            break;
            }
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            {
            SplineThroughPointsPlacementStrategyP intCurveStrategy = dynamic_cast<SplineThroughPointsPlacementStrategyP>(&replacement);
            if (nullptr == intCurveStrategy)
                {
                BeAssert(nullptr != intCurveStrategy);
                return;
                }
            _CopyKeyPointsTo(*intCurveStrategy);
            break;
            }
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            LinePlacementStrategyP lineStrategy = dynamic_cast<LinePlacementStrategyP>(&replacement);
            if (nullptr == lineStrategy)
                {
                BeAssert(nullptr != lineStrategy);
                return;
                }
            _CopyKeyPointsTo(*lineStrategy);
            break;
            }
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            LineStringPlacementStrategyP lineStringStrategy = dynamic_cast<LineStringPlacementStrategyP>(&replacement);
            if (nullptr == lineStringStrategy)
                {
                BeAssert(nullptr != lineStringStrategy);
                return;
                }
            _CopyKeyPointsTo(*lineStringStrategy);
            break;
            }
        default:
            {
            BeAssert(false && "Not implemented");
            return;
            }
        }
    }