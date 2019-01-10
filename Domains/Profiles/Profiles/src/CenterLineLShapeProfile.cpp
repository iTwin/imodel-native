/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/CenterLineLShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CenterLineLShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CenterLineLShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineLShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineLShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double girth, double wallThickness, double filletRadius /*= 0.0*/)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , width (width)
    , depth (depth)
    , girth (girth)
    , wallThickness (wallThickness)
    , filletRadius (filletRadius)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineLShapeProfile::CenterLineLShapeProfile(CreateParams const& params)
    : T_Super (params)
    {
    if (false != params.m_isLoadingElement)
        {
        return;
        }

    SetWidth (params.width);
    SetDepth (params.depth);
    SetGirth (params.girth);
    SetFilletRadius (params.filletRadius);
    SetWallThickness (params.wallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineLShapeProfile::_Validate() const
    {
    bool bValid(T_Super::_Validate());

    bValid = bValid && std::isfinite(GetWidth()) && (GetWidth() > 0.0);
    bValid = bValid && std::isfinite(GetDepth()) && (GetDepth() > 0.0);
    bValid = bValid && std::isfinite(GetGirth()) && (GetGirth() >= 0.0);
    bValid = bValid && std::isfinite(GetWallThickness()) && (GetWallThickness() > 0.0);
    bValid = bValid && std::isfinite(GetFilletRadius());

    return bValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CenterLineLShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeometry::CreateCenterLineLShape (this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineLShapeProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineLShapeProfile_Width, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineLShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineLShapeProfile_Depth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineLShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineLShapeProfile_FilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineLShapeProfile::GetGirth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineLShapeProfile_Girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineLShapeProfile::SetGirth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineLShapeProfile_Girth, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
