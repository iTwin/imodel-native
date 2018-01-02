/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcStartEndMidPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartEndMidPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    ResetDynamicKeyPoint();
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyR();

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

    if (!strategy.IsMidSet())
        {
        strategy.SetMid(newKeyPoint);
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartEndMidPlacementStrategy::_PopKeyPoint()
    {
    ResetDynamicKeyPoint();
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyR();

    if (strategy.IsMidSet())
        {
        strategy.ResetMid();
        return;
        }

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
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartEndMidPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    ResetDynamicKeyPoint();
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyR();

    if (strategy.IsMidSet())
        {
        return;
        }

    if (strategy.IsEndSet())
        {
        strategy.SetDynamicMid(newDynamicKeyPoint);
        return;
        }

    if (strategy.IsStartSet())
        {
        strategy.SetDynamicEnd(newDynamicKeyPoint);
        return;
        }

    strategy.SetDynamicStart(newDynamicKeyPoint);
    }