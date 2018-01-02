/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcStartEndMidPlacementStrategy.cpp $
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
void ArcStartEndMidPlacementStrategy::_AddKeyPoint
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
        GetManipulationStrategyR().InsertDynamicKeyPoints({INVALID_POINT, INVALID_POINT}, 1);
        return;
        }

    if (acceptedPoints.size() == 2)
        {
        DEllipse3d tmpArc = DEllipse3d::FromPointsOnArc(acceptedPoints[0], newKeyPoint, acceptedPoints[1]);
        GetManipulationStrategyR().InsertKeyPoint(tmpArc.center, 1);
        GetManipulationStrategyR().InsertKeyPoint(newKeyPoint, 2);
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartEndMidPlacementStrategy::_PopKeyPoint()
    {
    bvector<DPoint3d> const& acceptedPoints = GetManipulationStrategy().GetAcceptedKeyPoints();

    if (acceptedPoints.size() == 4)
        {
        GetManipulationStrategyR().RemoveKeyPoint(1);
        GetManipulationStrategyR().RemoveKeyPoint(1);
        GetManipulationStrategyR().InsertDynamicKeyPoints({INVALID_POINT, INVALID_POINT}, 1);
        return;
        }

    T_Super::_PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartEndMidPlacementStrategy::_AddDynamicKeyPoint
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
        GetManipulationStrategyR().AppendDynamicKeyPoints({INVALID_POINT, INVALID_POINT, newDynamicKeyPoint});
        return;
        }

    if (acceptedPoints.size() == 2)
        {
        DEllipse3d tmpArc = DEllipse3d::FromPointsOnArc(acceptedPoints[0], newDynamicKeyPoint, acceptedPoints[1]);
        GetManipulationStrategyR().UpsertDynamicKeyPoints({tmpArc.center, newDynamicKeyPoint, acceptedPoints[1]}, 1);
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr ArcStartEndMidPlacementStrategy::_FinishPrimitive() const
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
bool ArcStartEndMidPlacementStrategy::_IsDynamicKeyPointSet() const
    {
    bvector<DPoint3d> const& keyPointsWithDynamic = GetKeyPoints();
    bvector<DPoint3d> const& keyPoints = GetManipulationStrategy().GetAcceptedKeyPoints();

    if (keyPoints.size() == 1 && !GetManipulationStrategy().IsDynamicKeyPointSet())
        return false;

    if (keyPoints.size() == 2 &&
        keyPointsWithDynamic[1].AlmostEqual(INVALID_POINT) &&
        keyPointsWithDynamic[2].AlmostEqual(INVALID_POINT))
        return false;

    return T_Super::_IsDynamicKeyPointSet();
    }