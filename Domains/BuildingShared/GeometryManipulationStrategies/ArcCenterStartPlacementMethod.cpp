/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"
#include <limits>

#define INVALID_POINT DPoint3d::From(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max())

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcCenterStartPlacementMethod::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    BeAssert(!GetArcManipulationStrategy().IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyForEdit();

    if (!strategy.IsCenterSet())
        {
        strategy.SetCenter(newKeyPoint);
        return;
        }

    if (!strategy.IsStartSet())
        {
        strategy.SetStart(newKeyPoint);
        return;
        }

    if (!strategy.IsEndSet())
        {
        strategy.SetEnd(newKeyPoint);
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcCenterStartPlacementMethod::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    BeAssert(!GetArcManipulationStrategy().IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyForEdit();

    if (strategy.IsEndSet())
        {
        return;
        }

    if (strategy.IsStartSet())
        {
        strategy.SetDynamicEnd(newDynamicKeyPoint);
        return;
        }

    if (strategy.IsCenterSet())
        {
        strategy.SetDynamicStart(newDynamicKeyPoint);
        return;
        }

    strategy.SetDynamicCenter(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcCenterStartPlacementMethod::_AddDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    if (newDynamicKeyPoints.size() == 1)
        {
        _AddDynamicKeyPoint(newDynamicKeyPoints.front());
        return;
        }

    BeAssert(!GetArcManipulationStrategy().IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyForEdit();

    if (strategy.IsEndSet())
        {
        return;
        }

    if (strategy.IsStartSet())
        {
        strategy.SetDynamicEnd(newDynamicKeyPoints[0]);
        return;
        }

    if (strategy.IsCenterSet())
        {
        strategy.SetDynamicStart(newDynamicKeyPoints[0]);
        strategy.SetDynamicEnd(newDynamicKeyPoints[1]);
        return;
        }

    strategy.SetDynamicCenter(newDynamicKeyPoints[0]);
    strategy.SetDynamicStart(newDynamicKeyPoints[1]);
    if (newDynamicKeyPoints.size() >= 3)
        strategy.SetDynamicEnd(newDynamicKeyPoints[2]);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcCenterStartPlacementMethod::_PopKeyPoint()
    {
    BeAssert(!GetArcManipulationStrategy().IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyForEdit();

    if (strategy.IsEndSet())
        {
        strategy.ResetEnd();
        return;
        }

    if (strategy.IsStartSet())
        {
        strategy.ResetStart();
        return;
        }

    if (strategy.IsCenterSet())
        {
        strategy.ResetCenter();
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> ArcCenterStartPlacementMethod::_GetKeyPoints() const
    {
    ArcManipulationStrategyCR strategy = GetArcManipulationStrategy();

    bvector<DPoint3d> keyPoints;
    if (strategy.IsCenterSet())
        keyPoints.push_back(strategy.GetCenter());
    if (strategy.IsStartSet())
        keyPoints.push_back(strategy.GetStart());
    if (strategy.IsEndSet())
        keyPoints.push_back(strategy.GetEnd());

    return keyPoints;
    }