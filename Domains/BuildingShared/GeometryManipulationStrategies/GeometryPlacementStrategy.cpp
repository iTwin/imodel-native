/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    _GetManipulationStrategyR().AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _GetManipulationStrategyR()._ResetDynamicKeyPoint();
    _AddKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> const& GeometryPlacementStrategy::_GetKeyPoints() const
    {
    return _GetManipulationStrategy().GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_PopKeyPoint()
    {
    _GetManipulationStrategyR().PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::PopKeyPoint()
    {
    _GetManipulationStrategyR()._ResetDynamicKeyPoint();
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
    _GetManipulationStrategyR().AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_AddDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    _GetManipulationStrategyR().AppendDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _GetManipulationStrategyR()._ResetDynamicKeyPoint();
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
    _GetManipulationStrategyR()._ResetDynamicKeyPoint();
    _AddDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryPlacementStrategy::_ResetDynamicKeyPoint()
    {
    _GetManipulationStrategyR().ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
GeometryManipulationStrategyCR GeometryPlacementStrategy::GetManipulationStrategy() const
    {
    return _GetManipulationStrategy();
    }

#define GMS_PROPERTY_OVERRIDE_IMPL(value_type) \
    void GeometryPlacementStrategy::_SetProperty(Utf8CP key, value_type const& value) \
        { \
        _GetManipulationStrategyR().SetProperty(key, value); \
        } \
    BentleyStatus GeometryPlacementStrategy::_TryGetProperty(Utf8CP key, value_type& value) const \
        { \
        return _GetManipulationStrategy().TryGetProperty(key, value); \
        }

GMS_PROPERTY_OVERRIDE_IMPL(int)
GMS_PROPERTY_OVERRIDE_IMPL(double)
GMS_PROPERTY_OVERRIDE_IMPL(DVec3d)
GMS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElementId)
GMS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElement)
GMS_PROPERTY_OVERRIDE_IMPL(Utf8String)