/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnElementManipulationStrategies/DgnElementManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/DgnElementManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

#define SET_PROPERTY_IMPL(value_type) \
    void DgnElementManipulationStrategy::SetProperty(Utf8CP key, value_type const& value) \
        { \
        if (nullptr == key) { BeAssert(false && "NULL key"); return; } \
        _SetProperty(key, value); \
        _OnPropertySet(key); \
        }
#define TRY_GET_PROPERTY_IMPL(value_type) \
    BentleyStatus DgnElementManipulationStrategy::TryGetProperty(Utf8CP key, value_type& value) const \
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementManipulationStrategy::FinishElement
(
    Dgn::DgnModelR model
)
    {
    return _FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementManipulationStrategy::FinishElement()
    {
    return _FinishElement();
    }