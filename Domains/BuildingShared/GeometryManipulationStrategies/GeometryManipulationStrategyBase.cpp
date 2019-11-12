/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
DynamicStateBasePtr DynamicStateBase::Clone() const
    {
    return _Clone();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
DynamicStateBaseCPtr IResettableDynamic::GetDynamicState() const
    {
    return _GetDynamicState();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void IResettableDynamic::SetDynamicState
(
    DynamicStateBaseCR state
)
    {
    _SetDynamicState(state);
    }

#define SET_PROPERTY_IMPL(value_type) \
    void GeometryManipulationStrategyBase::SetProperty(Utf8CP key, value_type const& value) \
        { \
        if (nullptr == key) { BeAssert(false && "NULL key"); return; } \
        _SetProperty(key, value); \
        _OnPropertySet(key); \
        }
#define TRY_GET_PROPERTY_IMPL(value_type) \
    BentleyStatus GeometryManipulationStrategyBase::TryGetProperty(Utf8CP key, value_type& value) const \
    { \
    if (nullptr == key) { BeAssert(false && "NULL key"); return BentleyStatus::ERROR; } \
    return _TryGetProperty(key, value); \
    }
#define SET_TRYGET_PROPERTY_IMPL(value_type) \
    SET_PROPERTY_IMPL(value_type) \
    TRY_GET_PROPERTY_IMPL(value_type)

SET_TRYGET_PROPERTY_IMPL(bool)
SET_TRYGET_PROPERTY_IMPL(int)
SET_TRYGET_PROPERTY_IMPL(double)
SET_TRYGET_PROPERTY_IMPL(DPoint2d)
SET_TRYGET_PROPERTY_IMPL(DVec3d)
SET_TRYGET_PROPERTY_IMPL(DPlane3d)
SET_TRYGET_PROPERTY_IMPL(RotMatrix)
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr GeometryManipulationStrategyBase::FinishGeometry() const
    {
    return _FinishGeometry();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ConstructionGeometry> GeometryManipulationStrategyBase::FinishConstructionGeometry() const
    {
    return _FinishConstructionGeometry();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterProperty
(
    Utf8StringCR propertyName, 
    bvector<Utf8String>& allProperties
)
    {
    auto existing = std::find(allProperties.begin(), allProperties.end(), propertyName);
    if (allProperties.end() == existing)
        allProperties.push_back(propertyName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::UnregisterProperty
(
    Utf8StringCR propertyName, 
    bvector<Utf8String>& allProperties
)
    {
    auto existing = std::find(allProperties.begin(), allProperties.end(), propertyName);
    if (allProperties.end() != existing)
        allProperties.erase(existing);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterBoolProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredBoolProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterIntProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredIntProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterDoubleProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredDoubleProperties);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Mindaugas.Butkus                06/2019
//--------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterDPoint2dProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredDPoint2dProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterDVec3dProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredDVec3dProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterDPlane3dProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredDPlane3dProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterRotMatrixProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredRotMatrixProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterUtf8StringProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredUtf8StringProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterDoubleVecProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredDoubleVecProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterUtf8StringVecProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredUtf8StringVecProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::RegisterCustomProperty
(
    Utf8StringCR propertyName
)
    {
    RegisterProperty(propertyName, m_registeredCustomProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::UnregisterProperty
(
    Utf8StringCR propertyName
)
    {
    UnregisterProperty(propertyName, m_registeredBoolProperties);
    UnregisterProperty(propertyName, m_registeredIntProperties);
    UnregisterProperty(propertyName, m_registeredDoubleProperties);
    UnregisterProperty(propertyName, m_registeredDPoint2dProperties);
    UnregisterProperty(propertyName, m_registeredDVec3dProperties);
    UnregisterProperty(propertyName, m_registeredDPlane3dProperties);
    UnregisterProperty(propertyName, m_registeredRotMatrixProperties);
    UnregisterProperty(propertyName, m_registeredUtf8StringProperties);
    UnregisterProperty(propertyName, m_registeredDoubleVecProperties);
    UnregisterProperty(propertyName, m_registeredUtf8StringVecProperties);
    UnregisterProperty(propertyName, m_registeredCustomProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
template <typename T>
void GeometryManipulationStrategyBase::CopyPropertiesTo
(
    GeometryManipulationStrategyBaseCR from,
    GeometryManipulationStrategyBaseR to,
    bvector<Utf8String> const& propertyNames
)
    {
    for (Utf8StringCR propertyName : propertyNames)
        {
        T value;
        if (BentleyStatus::SUCCESS == from.TryGetProperty(propertyName.c_str(), value))
            to.SetProperty(propertyName.c_str(), value);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::_CopyPropertiesTo
(
    GeometryManipulationStrategyBaseR other
) const
    {
    CopyPropertiesTo<bool>(*this, other, m_registeredBoolProperties);
    CopyPropertiesTo<int>(*this, other, m_registeredIntProperties);
    CopyPropertiesTo<double>(*this, other, m_registeredDoubleProperties);
    CopyPropertiesTo<DPoint2d>(*this, other, m_registeredDPoint2dProperties);
    CopyPropertiesTo<DVec3d>(*this, other, m_registeredDVec3dProperties);
    CopyPropertiesTo<DPlane3d>(*this, other, m_registeredDPlane3dProperties);
    CopyPropertiesTo<RotMatrix>(*this, other, m_registeredRotMatrixProperties);
    CopyPropertiesTo<Utf8String>(*this, other, m_registeredUtf8StringProperties);
    CopyPropertiesTo<bvector<double>>(*this, other, m_registeredDoubleVecProperties);
    CopyPropertiesTo<bvector<Utf8String>>(*this, other, m_registeredUtf8StringVecProperties);
    CopyPropertiesTo<GeometryManipulationStrategyProperty>(*this, other, m_registeredCustomProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategyBase::CopyPropertiesTo
(
    GeometryManipulationStrategyBaseR other
) const
    {
    _CopyPropertiesTo(other);
    }