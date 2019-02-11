/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryPlacementStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _GetManipulationStrategyForEdit().AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _ResetDynamicKeyPoint();
    _AddKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> GeometryPlacementStrategy::_GetKeyPoints() const
    {
    return _GetManipulationStrategy().GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_PopKeyPoint()
    {
    _GetManipulationStrategyForEdit().PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::PopKeyPoint()
    {
    _ResetDynamicKeyPoint();
    _PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryPlacementStrategy::_IsDynamicKeyPointSet() const
    {
    return _GetManipulationStrategy().IsDynamicKeyPointSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _GetManipulationStrategyForEdit().AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_AddDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    _GetManipulationStrategyForEdit().AppendDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _ResetDynamicKeyPoint();
    _AddDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::AddDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    _ResetDynamicKeyPoint();
    _AddDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_ResetDynamicKeyPoint()
    {
    _GetManipulationStrategyForEdit().ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
GeometryManipulationStrategyCR GeometryPlacementStrategy::GetManipulationStrategy() const
    {
    return _GetManipulationStrategy();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryPlacementStrategy::_IsComplete() const
    {
    return _GetManipulationStrategy().IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryPlacementStrategy::_CanAcceptMorePoints() const
    {
    return _GetManipulationStrategy().CanAcceptMorePoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr GeometryPlacementStrategy::_FinishGeometry() const
    {
    return _GetManipulationStrategy().FinishGeometry();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_SetDynamicState(DynamicStateBaseCR state)
    {
    _GetManipulationStrategyForEdit().SetDynamicState(state);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
DynamicStateBaseCPtr GeometryPlacementStrategy::_GetDynamicState() const
    {
    return _GetManipulationStrategy().GetDynamicState();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ConstructionGeometry> GeometryPlacementStrategy::_FinishConstructionGeometry() const
    {
    return _GetManipulationStrategy().FinishConstructionGeometry();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_CopyPropertiesTo
(
    GeometryManipulationStrategyBaseR other
) const
    {
    // Without this the placement strategy would need to register properties that are used
    // in the manipulation strategy.
    _GetManipulationStrategy().CopyPropertiesTo(other);
    T_Super::_CopyPropertiesTo(other);
    }

#define GMS_PROPERTY_OVERRIDE_IMPL(value_type) \
    void GeometryPlacementStrategy::_SetProperty(Utf8CP key, value_type const& value) \
        { \
        _GetManipulationStrategyForEdit().SetProperty(key, value); \
        } \
    BentleyStatus GeometryPlacementStrategy::_TryGetProperty(Utf8CP key, value_type& value) const \
        { \
        return _GetManipulationStrategy().TryGetProperty(key, value); \
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