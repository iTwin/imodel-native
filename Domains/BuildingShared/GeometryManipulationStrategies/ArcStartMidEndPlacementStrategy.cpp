/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcStartMidEndPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcManipulationStrategyR ArcStartMidEndPlacementStrategy::GetArcManipulationStrategyR()
    {
    return dynamic_cast<ArcManipulationStrategyR>(GetManipulationStrategyR());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartMidEndPlacementStrategy::_AddKeyPoint
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

    if (!strategy.IsMidSet())
        {
        strategy.SetMid(newKeyPoint);
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
void ArcStartMidEndPlacementStrategy::_PopKeyPoint()
    {
    ResetDynamicKeyPoint();
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyR();

    if (strategy.IsEndSet())
        {
        strategy.ResetEnd();
        return;
        }

    if (strategy.IsMidSet())
        {
        strategy.ResetMid();
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
void ArcStartMidEndPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    ResetDynamicKeyPoint();
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyR();

    if (strategy.IsEndSet())
        {
        return;
        }

    if (strategy.IsMidSet())
        {
        strategy.SetDynamicEnd(newDynamicKeyPoint);
        return;
        }

    if (strategy.IsStartSet())
        {
        strategy.SetDynamicMid(newDynamicKeyPoint);
        return;
        }

    strategy.SetDynamicStart(newDynamicKeyPoint);
    }