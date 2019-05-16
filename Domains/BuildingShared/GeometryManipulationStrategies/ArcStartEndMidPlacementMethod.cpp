/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartEndMidPlacementMethod::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    BeAssert(!GetArcManipulationStrategy().IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyForEdit();

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
void ArcStartEndMidPlacementMethod::_PopKeyPoint()
    {
    BeAssert(!GetArcManipulationStrategy().IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyForEdit();

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
void ArcStartEndMidPlacementMethod::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    BeAssert(!GetArcManipulationStrategy().IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyForEdit();

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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> ArcStartEndMidPlacementMethod::_GetKeyPoints() const
    {
    ArcManipulationStrategyCR strategy = GetArcManipulationStrategy();

    bvector<DPoint3d> keyPoints;
    if (strategy.IsStartSet())
        keyPoints.push_back(strategy.GetStart());
    if (strategy.IsEndSet())
        keyPoints.push_back(strategy.GetEnd());
    if (strategy.IsMidSet())
        keyPoints.push_back(strategy.GetMid());

    return keyPoints;
    }