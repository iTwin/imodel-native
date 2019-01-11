/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/CenterLineCShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CenterLineCShapeProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CenterLineCShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineCShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineCShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double girth, double wallThickness, double filletRadius /*= 0.0*/)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , flangeWidth (flangeWidth)
    , depth (depth)
    , girth(girth)
    , wallThickness (wallThickness)
    , filletRadius (filletRadius)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineCShapeProfile::CenterLineCShapeProfile(CreateParams const& params)
    : T_Super (params)
    {
    if (false != params.m_isLoadingElement)
        {
        return;
        }

    SetFlangeWidth (params.flangeWidth);
    SetDepth (params.depth);
    SetGirth (params.girth);
    SetFilletRadius (params.filletRadius);
    SetWallThickness (params.wallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CenterLineCShapeProfile::_Validate() const
    {
    bool bValid(T_Super::_Validate());

    bValid = bValid && std::isfinite(GetFlangeWidth()) && (GetFlangeWidth() > 0.0);
    bValid = bValid && std::isfinite(GetDepth()) && (GetDepth() > 0.0);
    bValid = bValid && std::isfinite(GetGirth()) && (GetGirth() >= 0.0);
    bValid = bValid && std::isfinite(GetWallThickness()) && (GetWallThickness() > 0.0);
    bValid = bValid && std::isfinite(GetFilletRadius()) && (GetFilletRadius() >= 0.0);

    bValid = bValid && (GetWallThickness() < GetFlangeWidth() / 2.0) && (GetWallThickness() < GetDepth() / 2.0);

    if (GetGirth() > 0.0)
        {
        bValid = bValid && (GetGirth() < (GetDepth() / 2.0));
        }

    if (GetFilletRadius() > 0.0)
        {
        bValid = bValid && (GetFilletRadius() <= (GetFlangeWidth() / 2.0 - GetWallThickness())) &&
            (GetFilletRadius() <= (GetDepth() / 2.0 - GetWallThickness()));
        }

    return bValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CenterLineCShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeometry::CreateCenterLineCShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineCShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetFlangeWidth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineCShapeProfile_FlangeWidth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineCShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineCShapeProfile_Depth, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetFilletRadius() const 
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineCShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineCShapeProfile_FilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineCShapeProfile::GetGirth() const 
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineCShapeProfile_Girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineCShapeProfile::SetGirth (double value)
    {
    SetPropertyValue (PRF_PROP_CenterLineCShapeProfile_Girth, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
