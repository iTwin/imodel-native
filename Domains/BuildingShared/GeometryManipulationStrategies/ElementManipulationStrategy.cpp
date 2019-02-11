/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ElementManipulationStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _GetGeometryManipulationStrategyForEdit().AppendDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    _GetGeometryManipulationStrategyForEdit().AppendDynamicKeyPoints(adjusted);
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
    _GetGeometryManipulationStrategyForEdit().InsertDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint), index);
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
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    _GetGeometryManipulationStrategyForEdit().InsertDynamicKeyPoints(adjusted, index);
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
    _GetGeometryManipulationStrategyForEdit().UpdateDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint), index);
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
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    _GetGeometryManipulationStrategyForEdit().UpdateDynamicKeyPoints(adjusted, index);
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
    _GetGeometryManipulationStrategyForEdit().UpsertDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint), index);
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
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    _GetGeometryManipulationStrategyForEdit().UpsertDynamicKeyPoints(adjusted, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _GetGeometryManipulationStrategyForEdit().AppendKeyPoint(AdjustPoint(newKeyPoint));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_AppendKeyPoints
(
    bvector<DPoint3d> const& newKeyPoints
)
    {
    bvector<DPoint3d> adjusted(newKeyPoints.size());
    std::transform(newKeyPoints.begin(),
                   newKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    _GetGeometryManipulationStrategyForEdit().AppendKeyPoints(adjusted);
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
    _GetGeometryManipulationStrategyForEdit().InsertKeyPoint(AdjustPoint(newKeyPoint), index);
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
    _GetGeometryManipulationStrategyForEdit().ReplaceKeyPoint(AdjustPoint(newKeyPoint), index);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_SetDynamicState
(
    DynamicStateBaseCR state
)
    {
    _GetGeometryManipulationStrategyForEdit().SetDynamicState(state);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
DynamicStateBaseCPtr ElementManipulationStrategy::_GetDynamicState() const
    {
    return _GetGeometryManipulationStrategy().GetDynamicState();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<IGeometryPtr> ElementManipulationStrategy::_FinishConstructionGeometry() const
    {
    return _GetGeometryManipulationStrategy().FinishConstructionGeometry();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElementManipulationStrategy::_CopyPropertiesTo
(
    GeometryManipulationStrategyBaseR other
) const
    {
    GeometryPlacementStrategyCPtr geometryPlacementStrategy = _TryGetGeometryPlacementStrategy();
    if (geometryPlacementStrategy.IsValid())
        geometryPlacementStrategy->CopyPropertiesTo(other);
    else
        _GetGeometryManipulationStrategy().CopyPropertiesTo(other);
    
    T_Super::_CopyPropertiesTo(other);
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
GMS_PROPERTY_OVERRIDE_IMPL(RotMatrix)
GMS_PROPERTY_OVERRIDE_IMPL(Utf8String)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<double>)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<Utf8String>)
GMS_PROPERTY_OVERRIDE_IMPL(GeometryManipulationStrategyProperty)