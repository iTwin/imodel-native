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