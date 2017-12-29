/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcStartMidEndPlacementStrategy.cpp $
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
void ArcStartMidEndPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    bvector<DPoint3d> const& acceptedPoints = GetManipulationStrategy().GetAcceptedKeyPoints();

    if (acceptedPoints.size() == 0)
        {
        GetManipulationStrategyR().AppendKeyPoint(newKeyPoint);
        return;
        }
    
    if (acceptedPoints.size() == 1)
        {
        GetManipulationStrategyR().AppendKeyPoint(newKeyPoint);
        GetManipulationStrategyR().InsertDynamicKeyPoint(INVALID_POINT, 1);
        return;
        }

    if (acceptedPoints.size() == 2)
        {
        DEllipse3d tmpArc = DEllipse3d::FromPointsOnArc(acceptedPoints[0], acceptedPoints[1], newKeyPoint);
        GetManipulationStrategyR().InsertKeyPoint(tmpArc.center, 1);
        GetManipulationStrategyR().AppendKeyPoint(newKeyPoint);
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartMidEndPlacementStrategy::_PopKeyPoint()
    {
    bvector<DPoint3d> const& acceptedPoints = GetManipulationStrategy().GetAcceptedKeyPoints();

    if (acceptedPoints.size() == 4)
        {
        GetManipulationStrategyR().PopKeyPoint();
        GetManipulationStrategyR().RemoveKeyPoint(1);
        GetManipulationStrategyR().InsertDynamicKeyPoint(INVALID_POINT, 1);
        return;
        }

    T_Super::_PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartMidEndPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    bvector<DPoint3d> const& acceptedPoints = GetManipulationStrategy().GetAcceptedKeyPoints();

    if (acceptedPoints.size() == 0)
        {
        GetManipulationStrategyR().AppendDynamicKeyPoint(newDynamicKeyPoint);
        return;
        }

    if (acceptedPoints.size() == 1)
        {
        GetManipulationStrategyR().AppendDynamicKeyPoints({INVALID_POINT, newDynamicKeyPoint});
        return;
        }

    if (acceptedPoints.size() == 2)
        {
        DEllipse3d tmpArc = DEllipse3d::FromPointsOnArc(acceptedPoints[0], acceptedPoints[1], newDynamicKeyPoint);
        GetManipulationStrategyR().UpsertDynamicKeyPoints({tmpArc.center, acceptedPoints[1], newDynamicKeyPoint}, 1);
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr ArcStartMidEndPlacementStrategy::_FinishPrimitive() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.size() > 2 &&
        keyPoints[1].AlmostEqual(INVALID_POINT))
        {
        return nullptr;
        }

    if (keyPoints.size() == 4)
        {
        DEllipse3d arc = DEllipse3d::FromPointsOnArc(keyPoints[0], keyPoints[2], keyPoints[3]);
        return ICurvePrimitive::CreateArc(arc);
        }

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bool ArcStartMidEndPlacementStrategy::_IsDynamicKeyPointSet() const
    {
    bvector<DPoint3d> const& keyPointsWithDynamic = GetKeyPoints();
    bvector<DPoint3d> const& keyPoints = GetManipulationStrategy().GetAcceptedKeyPoints();

    if (keyPoints.size() == 1 &&
        keyPointsWithDynamic.size() >= 3 &&
        GetManipulationStrategy().IsDynamicKeyPointSet())
        {
        return true;
        }

    if (keyPoints.size() == 2 &&
        keyPointsWithDynamic.size() >= 3 &&
        GetManipulationStrategy().IsDynamicKeyPointSet() &&
        keyPointsWithDynamic[1].AlmostEqual(INVALID_POINT))
        {
        return false;
        }

    return T_Super::_IsDynamicKeyPointSet();
    }