/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ArbitraryCenterLineProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>
#include <Profiles\ArbitraryCenterLineProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ArbitraryCenterLineProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCenterLineProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName, IGeometryPtr const& geometryPtr, double wallThickness)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName, geometryPtr)
    , wallThickness (wallThickness)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCenterLineProfile::ArbitraryCenterLineProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetWallThickness (params.wallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArbitraryCenterLineProfile::_Validate() const
    {
    if (!SinglePerimeterProfile::_Validate()) // Skip ArbitraryShapeProfile::_Validate because geometry hasn't been created yet
        return false;

    BeAssert (m_geometryPtr.IsValid() && "Null geometry should be handled in T_Super::_Validate()");
    if (!ProfilesProperty::IsGreaterThanZero (GetWallThickness()))
        return false;

    IGeometry::GeometryType type = m_geometryPtr->GetGeometryType();
    if (type != IGeometry::GeometryType::CurvePrimitive && type != IGeometry::GeometryType::CurveVector)
        return false;

    // TODO Karolis: Could have more validation for underlying geometry ..

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArbitraryCenterLineProfile::_CreateGeometry()
    {
    if (!T_Super::_CreateGeometry())
        return false;

    if (m_geometryPtr.IsValid())
        SetCenterLine (*m_geometryPtr);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ArbitraryCenterLineProfile::_CreateShapeGeometry() const
    {
    BeAssert (m_geometryPtr.IsValid() && "Null geometry should be handled in _Validate()");
    return ProfilesGeometry::CreateArbitraryCenterLineShape (*m_geometryPtr, GetWallThickness());
    }

END_BENTLEY_PROFILES_NAMESPACE
