/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcStartCenterPlacementMethod.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartCenterPlacementMethod::_AddKeyPoint
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

    if (!strategy.IsCenterSet())
        {
        strategy.SetCenter(newKeyPoint);
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
void ArcStartCenterPlacementMethod::_AddDynamicKeyPoint
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

    if (strategy.IsCenterSet())
        {
        strategy.SetDynamicEnd(newDynamicKeyPoint);
        return;
        }

    if (strategy.IsStartSet())
        {
        strategy.SetDynamicCenter(newDynamicKeyPoint);
        return;
        }

    strategy.SetDynamicStart(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcStartCenterPlacementMethod::_PopKeyPoint()
    {
    BeAssert(!GetArcManipulationStrategy().IsDynamicKeyPointSet());
    ArcManipulationStrategyR strategy = GetArcManipulationStrategyForEdit();

    if (strategy.IsEndSet())
        {
        strategy.ResetEnd();
        return;
        }

    if (strategy.IsCenterSet())
        {
        strategy.ResetCenter();
        return;
        }

    if (strategy.IsStartSet())
        {
        strategy.ResetStart();
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> ArcStartCenterPlacementMethod::_GetKeyPoints() const
    {
    ArcManipulationStrategyCR strategy = GetArcManipulationStrategy();

    bvector<DPoint3d> keyPoints;
    if (strategy.IsStartSet())
        keyPoints.push_back(strategy.GetStart());
    if (strategy.IsCenterSet())
        keyPoints.push_back(strategy.GetCenter());
    if (strategy.IsEndSet())
        keyPoints.push_back(strategy.GetEnd());

    return keyPoints;
    }