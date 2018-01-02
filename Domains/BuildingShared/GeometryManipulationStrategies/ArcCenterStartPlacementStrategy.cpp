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