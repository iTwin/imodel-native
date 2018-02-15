/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ElementManipulationStrategy.cpp $
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
DgnElementPtr ElementManipulationStrategy::FinishElement
(
    DgnModelR model
)
    {
    return _FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DgnElementPtr ElementManipulationStrategy::FinishElement()
    {
    return _FinishElement();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _GetGeometryManipulationStrategyForEdit().AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    _GetGeometryManipulationStrategyForEdit().AppendDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_InsertDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
)
    {
    _GetGeometryManipulationStrategyForEdit().InsertDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_InsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    _GetGeometryManipulationStrategyForEdit().InsertDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
)
    {
    _GetGeometryManipulationStrategyForEdit().UpdateDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_UpdateDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    _GetGeometryManipulationStrategyForEdit().UpdateDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_UpsertDynamicKeyPoint
(
    DPoint3d newDynamicKeyPoint, 
    size_t index
)
    {
    _GetGeometryManipulationStrategyForEdit().UpsertDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_UpsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    _GetGeometryManipulationStrategyForEdit().UpsertDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _GetGeometryManipulationStrategyForEdit().AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_AppendKeyPoints
(
    bvector<DPoint3d> const& newKeyPoints
)
    {
    _GetGeometryManipulationStrategyForEdit().AppendKeyPoints(newKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_InsertKeyPoint
(
    DPoint3dCR newKeyPoint, 
    size_t index
)
    {
    _GetGeometryManipulationStrategyForEdit().InsertKeyPoint(newKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_ReplaceKeyPoint
(
    DPoint3dCR newKeyPoint, 
    size_t index
)
    {
    _GetGeometryManipulationStrategyForEdit().ReplaceKeyPoint(newKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_PopKeyPoint()
    {
    _GetGeometryManipulationStrategyForEdit().PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_RemoveKeyPoint
(
    size_t index
)
    {
    _GetGeometryManipulationStrategyForEdit().RemoveKeyPoint(index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_Clear()
    {
    _GetGeometryManipulationStrategyForEdit().Clear();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ElementManipulationStrategy::_FinishGeometry() const
    {
    return _GetGeometryManipulationStrategy().FinishGeometry();
    }

#define GMS_PROPERTY_OVERRIDE_IMPL(value_type) \
    void ElementManipulationStrategy::_SetProperty(Utf8CP key, value_type const& value) \
        { \
        _GetGeometryManipulationStrategyForEdit().SetProperty(key, value); \
        } \
    BentleyStatus ElementManipulationStrategy::_TryGetProperty(Utf8CP key, value_type& value) const \
        { \
        return _GetGeometryManipulationStrategy().TryGetProperty(key, value); \
        }

GMS_PROPERTY_OVERRIDE_IMPL(bool)
GMS_PROPERTY_OVERRIDE_IMPL(int)
GMS_PROPERTY_OVERRIDE_IMPL(double)
GMS_PROPERTY_OVERRIDE_IMPL(DVec3d)
GMS_PROPERTY_OVERRIDE_IMPL(DPlane3d)
GMS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElementId)
GMS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElement)
GMS_PROPERTY_OVERRIDE_IMPL(Utf8String)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<double>)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<Utf8String>)
GMS_PROPERTY_OVERRIDE_IMPL(GeometryManipulationStrategyProperty)