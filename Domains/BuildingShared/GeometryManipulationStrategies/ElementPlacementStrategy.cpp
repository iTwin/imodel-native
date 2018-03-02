/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ElementPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_BENTLEY_DGN

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DgnElementPtr ElementPlacementStrategy::_FinishElement
(
    DgnModelR model
)
    {
    return _GetElementManipulationStrategyForEdit().FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DgnElementPtr ElementPlacementStrategy::FinishElement
(
    DgnModelR model
)
    {
    return _FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> ElementPlacementStrategy::_GetKeyPoints() const
    {
    GeometryPlacementStrategyCPtr strategy = TryGetGeometryPlacementStrategy();
    if (strategy.IsNull())
        {
        BeAssert(false && "Invalid geometry placement strategy");
        return bvector<DPoint3d>();
        }
    return strategy->GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ElementPlacementStrategy::_IsDynamicKeyPointSet() const
    {
    GeometryPlacementStrategyCPtr strategy = TryGetGeometryPlacementStrategy();
    if (strategy.IsNull())
        {
        BeAssert(false && "Invalid geometry placement strategy");
        return false;
        }
    return strategy->IsDynamicKeyPointSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementPlacementStrategy::_AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint)
    {
    GeometryPlacementStrategyPtr strategy = TryGetGeometryPlacementStrategyForEdit();
    if (strategy.IsNull())
        {
        BeAssert(false && "Invalid geometry placement strategy");
        return;
        }

    strategy->AddDynamicKeyPoint(_GetManipulationStrategyForEdit().AdjustPoint(newDynamicKeyPoint));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementPlacementStrategy::_AddDynamicKeyPoints(bvector<DPoint3d> const & newDynamicKeyPoints)
    {
    GeometryPlacementStrategyPtr strategy = TryGetGeometryPlacementStrategyForEdit();
    if (strategy.IsNull())
        {
        BeAssert(false && "Invalid geometry placement strategy");
        return;
        }
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return _GetManipulationStrategyForEdit().AdjustPoint(point); });
    strategy->AddDynamicKeyPoints(adjusted);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementPlacementStrategy::_ResetDynamicKeyPoint()
    {
    GeometryPlacementStrategyPtr strategy = TryGetGeometryPlacementStrategyForEdit();
    if (strategy.IsNull())
        {
        BeAssert(false && "Invalid geometry placement strategy");
        return;
        }

    strategy->ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementPlacementStrategy::_AddKeyPoint(DPoint3dCR newKeyPoint)
    {
    GeometryPlacementStrategyPtr strategy = TryGetGeometryPlacementStrategyForEdit();
    if (strategy.IsNull())
        {
        BeAssert(false && "Invalid geometry placement strategy");
        return;
        }
    strategy->AddKeyPoint(_GetManipulationStrategyForEdit().AdjustPoint(newKeyPoint));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementPlacementStrategy::_PopKeyPoint()
    {
    GeometryPlacementStrategyPtr strategy = TryGetGeometryPlacementStrategyForEdit();
    if (strategy.IsNull())
        {
        BeAssert(false && "Invalid geometry placement strategy");
        return;
        }
    strategy->PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ElementPlacementStrategy::_IsComplete() const
    {
    GeometryPlacementStrategyCPtr strategy = TryGetGeometryPlacementStrategy();
    if (strategy.IsNull())
        {
        return T_Super::_IsComplete();
        }

    return strategy->IsComplete() && T_Super::_IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ElementPlacementStrategy::_CanAcceptMorePoints() const
    {
    GeometryPlacementStrategyCPtr strategy = TryGetGeometryPlacementStrategy();
    if (strategy.IsNull())
        {
        return false;
        }

    return strategy->CanAcceptMorePoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
GeometryPlacementStrategyCPtr ElementPlacementStrategy::TryGetGeometryPlacementStrategy() const
    {
    return _GetElementManipulationStrategy()._TryGetGeometryPlacementStrategy();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
GeometryPlacementStrategyPtr ElementPlacementStrategy::TryGetGeometryPlacementStrategyForEdit()
    {
    return _GetElementManipulationStrategyForEdit()._TryGetGeometryPlacementStrategyForEdit();
    }

#define EPS_PROPERTY_OVERRIDE_IMPL(value_type)                                                          \
    void ElementPlacementStrategy::_SetProperty(Utf8CP key, value_type const& value)                    \
        {                                                                                               \
        _GetManipulationStrategyForEdit().SetProperty(key, value);                                            \
        GeometryPlacementStrategyPtr strategy = TryGetGeometryPlacementStrategyForEdit();                    \
        if (strategy.IsValid())                                                                         \
            {                                                                                           \
            strategy->SetProperty(key, value);                                                          \
            }                                                                                           \
        }                                                                                               \
    BentleyStatus ElementPlacementStrategy::_TryGetProperty(Utf8CP key, value_type& value) const        \
        {                                                                                               \
        GeometryPlacementStrategyCPtr strategy = TryGetGeometryPlacementStrategy();                    \
        if (strategy.IsValid())                                                                         \
            {                                                                                           \
            if ((BentleyStatus::SUCCESS == _GetManipulationStrategy().TryGetProperty(key, value)) ||    \
                (BentleyStatus::SUCCESS == strategy->TryGetProperty(key, value)))                       \
            return BentleyStatus::SUCCESS;                                                              \
            }                                                                                           \
        return _GetManipulationStrategy().TryGetProperty(key, value);                                   \
        }

EPS_PROPERTY_OVERRIDE_IMPL(bool)
EPS_PROPERTY_OVERRIDE_IMPL(int)
EPS_PROPERTY_OVERRIDE_IMPL(double)
EPS_PROPERTY_OVERRIDE_IMPL(DVec3d)
EPS_PROPERTY_OVERRIDE_IMPL(DPlane3d)
EPS_PROPERTY_OVERRIDE_IMPL(RotMatrix)
EPS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElementId)
EPS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElementCP)
EPS_PROPERTY_OVERRIDE_IMPL(Utf8String)
EPS_PROPERTY_OVERRIDE_IMPL(bvector<double>)
EPS_PROPERTY_OVERRIDE_IMPL(bvector<Utf8String>)
EPS_PROPERTY_OVERRIDE_IMPL(GeometryManipulationStrategyProperty)