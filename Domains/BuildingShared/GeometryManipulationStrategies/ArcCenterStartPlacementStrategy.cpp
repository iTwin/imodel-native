/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcCenterStartPlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"
#include <limits>

#define INVALID_POINT DPoint3d::From(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max())

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d ArcCenterStartPlacementStrategy::CalculateVec90KeyPoint
(
    DPoint3dCR endPoint
) const
    {
    bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
    if (keyPoints.size() < 2)
        {
        BeAssert(false);
        return {0,0,0};
        }

    DPoint3d vec0Point = keyPoints[0];
    DPoint3d center = keyPoints[1];
    DVec3d vec0 = DVec3d::FromStartEnd(center, vec0Point);
    DVec3d endVec = DVec3d::FromStartEnd(center, endPoint);
    DVec3d axis = DVec3d::FromCrossProduct(vec0, endVec);
    if (DoubleOps::AlmostEqual(axis.Magnitude(), 0))
        axis = DVec3d::From(0, 0, 1); // if all points are inline - fallback to default axis.
    DVec3d vec90 = DVec3d::FromRotate90Around(vec0, DVec3d::FromCrossProduct(vec0, endVec));
    vec90.ScaleToLength(vec0.Magnitude());
    DPoint3d vec90Point = center;
    vec90Point.Add(vec90);
    return vec90Point;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcCenterStartPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    if (GetManipulationStrategy().GetAcceptedKeyPoints().size() == 0)
        {
        GetManipulationStrategyR().AppendKeyPoint(newKeyPoint);
        GetManipulationStrategyR().InsertDynamicKeyPoint(INVALID_POINT, 0);
        }
    else if (GetManipulationStrategy().GetAcceptedKeyPoints().size() == 1)
        {
        GetManipulationStrategyR().InsertKeyPoint(newKeyPoint, 0);
        }
    else
        {
        DPoint3d vec90point;
        bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
        if (keyPoints.size() >= 3)
            vec90point = keyPoints[2];
        else
            vec90point = CalculateVec90KeyPoint(newKeyPoint);

        GetManipulationStrategyR().AppendKeyPoint(vec90point);
        GetManipulationStrategyR().AppendKeyPoint(newKeyPoint);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcCenterStartPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    if (GetManipulationStrategy().GetAcceptedKeyPoints().size() == 0)
        {
        GetManipulationStrategyR().AppendDynamicKeyPoints({INVALID_POINT, newDynamicKeyPoint});
        }
    else if (GetManipulationStrategy().GetAcceptedKeyPoints().size() == 1)
        {
        GetManipulationStrategyR().InsertDynamicKeyPoint(newDynamicKeyPoint, 0);
        }
    else
        {
        DPoint3d vec90point;
        bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
        if (keyPoints.size() >= 3)
            vec90point = keyPoints[2];
        else
            vec90point = CalculateVec90KeyPoint(newDynamicKeyPoint);

        GetManipulationStrategyR().AppendDynamicKeyPoints({vec90point, newDynamicKeyPoint});
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcCenterStartPlacementStrategy::_PopKeyPoint()
    {
    bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
    if (keyPoints.size() == 4)
        {
        GetManipulationStrategyR().PopKeyPoint();
        GetManipulationStrategyR().PopKeyPoint();
        }
    else
        {
        T_Super::_PopKeyPoint();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr ArcCenterStartPlacementStrategy::_FinishPrimitive() const
    {
    if (GetManipulationStrategy().GetAcceptedKeyPoints().size() == 1 &&
        GetManipulationStrategy().IsDynamicKeyPointSet() &&
        GetManipulationStrategy().GetKeyPoints()[0].AlmostEqual(INVALID_POINT))
        {
        return nullptr;
        }

    return T_Super::_FinishPrimitive();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bool ArcCenterStartPlacementStrategy::_IsDynamicKeyPointSet() const
    {
    if (GetManipulationStrategy().GetAcceptedKeyPoints().size() == 1 &&
        GetManipulationStrategy().IsDynamicKeyPointSet() &&
        GetManipulationStrategy().GetKeyPoints()[0].AlmostEqual(INVALID_POINT))
        {
        return false;
        }

    return T_Super::_IsDynamicKeyPointSet();
    }