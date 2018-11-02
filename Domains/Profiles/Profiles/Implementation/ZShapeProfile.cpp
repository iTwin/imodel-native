/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ZShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ZShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(ZShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ZShapeProfilePtr ZShapeProfile::Create(DgnModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new ZShapeProfile(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble(PRF_PROP_ZShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFlangeWidth(double val)
    {
    SetPropertyValue(PRF_PROP_ZShapeProfile_FlangeWidth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble(PRF_PROP_ZShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetDepth(double val)
    {
    SetPropertyValue(PRF_PROP_ZShapeProfile_Depth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_ZShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_ZShapeProfile_FlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_ZShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetWebThickness(double val)
    {
    SetPropertyValue(PRF_PROP_ZShapeProfile_WebThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_ZShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_ZShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_ZShapeProfile_FlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFlangeEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_ZShapeProfile_FlangeEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ZShapeProfile::GetFlangeSlope() const
    {
    return GetPropertyValueDouble(PRF_PROP_ZShapeProfile_FlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ZShapeProfile::SetFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_ZShapeProfile_FlangeSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
