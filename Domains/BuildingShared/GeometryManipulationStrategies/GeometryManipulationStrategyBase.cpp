/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryManipulationStrategyBase.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> const& GeometryManipulationStrategyBase::GetKeyPoints() const
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
void GeometryManipulationStrategyBase::AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    _AppendDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::InsertDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    _InsertDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::InsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    _InsertDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    _UpdateDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::UpdateDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    _UpdateDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::ResetDynamicKeyPoint()
    {
    _ResetDynamicKeyPoint();
    }