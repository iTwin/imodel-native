/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(CShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CShapeProfilePtr CShapeProfile::Create(DgnModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new CShapeProfile(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_CShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_FlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_CShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetWebThickness(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_WebThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_CShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetEdgeRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_CShapeProfile_EdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_EdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CShapeProfile::GetFlangeSlope() const
    {
    return GetPropertyValueDouble(PRF_PROP_CShapeProfile_FlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CShapeProfile::SetFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_CShapeProfile_FlangeSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
