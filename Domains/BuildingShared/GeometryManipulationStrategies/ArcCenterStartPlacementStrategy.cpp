/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcCenterStartPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    BeAssert(!_IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyR();

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
void ArcCenterStartPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    BeAssert(!_IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyR();

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
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcCenterStartPlacementStrategy::_PopKeyPoint()
    {
    BeAssert(!_IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyR();

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