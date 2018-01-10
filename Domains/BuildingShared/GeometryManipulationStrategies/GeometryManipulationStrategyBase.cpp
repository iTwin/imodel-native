/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryManipulationStrategyBase.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

#define SET_PROPERTY_IMPL(value_type) \
    void GeometryManipulationStrategyBase::SetProperty(Utf8CP key, value_type const& value) { _SetProperty(key, value); }
#define TRY_GET_PROPERTY_IMPL(value_type) \
    BentleyStatus GeometryManipulationStrategyBase::TryGetProperty(Utf8CP key, value_type& value) const { return _TryGetProperty(key, value); }
#define SET_TRYGET_PROPERTY_IMPL(value_type) \
    SET_PROPERTY_IMPL(value_type) \
    TRY_GET_PROPERTY_IMPL(value_type)

SET_TRYGET_PROPERTY_IMPL(int)
SET_TRYGET_PROPERTY_IMPL(double)
SET_TRYGET_PROPERTY_IMPL(DVec3d)
SET_TRYGET_PROPERTY_IMPL(DPlane3d)
SET_TRYGET_PROPERTY_IMPL(DgnElementId)
SET_TRYGET_PROPERTY_IMPL(DgnElement)
SET_TRYGET_PROPERTY_IMPL(Utf8String)
SET_TRYGET_PROPERTY_IMPL(bvector<double>)
SET_TRYGET_PROPERTY_IMPL(bvector<Utf8String>)
SET_TRYGET_PROPERTY_IMPL(GeometryManipulationStrategyProperty)

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> GeometryManipulationStrategyBase::GetKeyPoints() const
    {
    return _GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryManipulationStrategyBase::IsDynamicKeyPointSet() const
    {
    return _IsDynamicKeyPointSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::ResetDynamicKeyPoint()
    {
    _ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryManipulationStrategyBase::IsComplete() const
    {
    return _IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryManipulationStrategyBase::CanAcceptMorePoints() const
    {
    return _CanAcceptMorePoints();
    }