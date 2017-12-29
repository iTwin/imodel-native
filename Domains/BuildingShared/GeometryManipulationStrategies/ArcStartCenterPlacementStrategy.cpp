/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcStartCenterPlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d ArcStartCenterPlacementStrategy::CalculateVec90KeyPoint
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
void ArcStartCenterPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
    if (keyPoints.size() < 2)
        GetManipulationStrategyR().AppendKeyPoint(newKeyPoint);
    else
        {
        DPoint3d vec90point;
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
void ArcStartCenterPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
    if ((!IsDynamicKeyPointSet() && keyPoints.size() < 2) ||
        (IsDynamicKeyPointSet() && keyPoints.size() <= 2))
        {
        GetManipulationStrategyR().AppendDynamicKeyPoint(newDynamicKeyPoint);
        }
    else
        {
        DPoint3d vec90point;
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
void ArcStartCenterPlacementStrategy::_PopKeyPoint()
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