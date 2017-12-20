/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryManipulationStrategyBase.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>
#include "PublicApi/GeometryManipulationStrategyBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
GeometryManipulationStrategyBase::GeometryManipulationStrategyBase()
    {
    }

#define SET_PROPERTY_IMPL(value_type) \
    void GeometryManipulationStrategyBase::SetProperty(Utf8CP key, value_type const& value) { _SetProperty(key, value); }
#define TRY_GET_PROPERTY_IMPL(value_type) \
    bool GeometryManipulationStrategyBase::TryGetProperty(Utf8CP key, value_type& value) { return _TryGetProperty(key, value); }
#define SET_TRYGET_PROPERTY_IMPL(value_type) \
    SET_PROPERTY_IMPL(value_type) \
    TRY_GET_PROPERTY_IMPL(value_type)

SET_TRYGET_PROPERTY_IMPL(int)
SET_TRYGET_PROPERTY_IMPL(double)
SET_TRYGET_PROPERTY_IMPL(DgnElementId)
SET_TRYGET_PROPERTY_IMPL(DgnElement)
SET_TRYGET_PROPERTY_IMPL(Utf8String)
