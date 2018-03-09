/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnElementManipulationStrategies/DgnElementPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/DgnElementManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementPlacementStrategy::_FinishElement
(
    Dgn::DgnModelR model
)
    {
    return _GetDgnElementManipulationStrategyForEdit().FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementPlacementStrategy::_FinishElement()
    {
    return _GetDgnElementManipulationStrategyForEdit().FinishElement();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementPlacementStrategy::FinishElement
(
    Dgn::DgnModelR model
)
    {
    return _FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementPlacementStrategy::FinishElement()
    {
    return _FinishElement();
    }

#define EPS_V_PROPERTY_IMPL(value_type)                                                                 \
    void DgnElementPlacementStrategy::_SetProperty(Utf8CP key, value_type const& value)                 \
        {                                                                                               \
        _GetDgnElementManipulationStrategyForEdit().SetProperty(key, value);                            \
        }                                                                                               \
    BentleyStatus DgnElementPlacementStrategy::_TryGetProperty(Utf8CP key, value_type& value) const     \
        {                                                                                               \
        return _GetDgnElementManipulationStrategy().TryGetProperty(key, value);                         \
        }

EPS_V_PROPERTY_IMPL(Dgn::DgnElementCP)
EPS_V_PROPERTY_IMPL(Dgn::DgnElementId)
EPS_V_PROPERTY_IMPL(Dgn::ColorDef)

#define SET_PROPERTY_IMPL(value_type) \
    void DgnElementPlacementStrategy::SetProperty(Utf8CP key, value_type const& value) \
        { \
        if (nullptr == key) { BeAssert(false && "NULL key"); return; } \
        _SetProperty(key, value); \
        _OnPropertySet(key); \
        }
#define TRY_GET_PROPERTY_IMPL(value_type) \
    BentleyStatus DgnElementPlacementStrategy::TryGetProperty(Utf8CP key, value_type& value) const \
        { \
        if (nullptr == key) { BeAssert(false && "NULL key"); return BentleyStatus::ERROR; } \
        return _TryGetProperty(key, value); \
        }
#define SET_TRYGET_PROPERTY_IMPL(value_type) \
    SET_PROPERTY_IMPL(value_type) \
    TRY_GET_PROPERTY_IMPL(value_type)

SET_TRYGET_PROPERTY_IMPL(Dgn::DgnElementCP)
SET_TRYGET_PROPERTY_IMPL(Dgn::DgnElementId)
SET_TRYGET_PROPERTY_IMPL(Dgn::ColorDef)