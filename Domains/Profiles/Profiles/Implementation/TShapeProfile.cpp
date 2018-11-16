/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/TShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\TShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(TShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TShapeProfilePtr TShapeProfile::Create(DgnModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new TShapeProfile(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble(PRF_PROP_TShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeWidth(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_FlangeWidth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble(PRF_PROP_TShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetDepth(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_Depth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_TShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_FlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_TShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWebThickness(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_WebThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_TShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_TShapeProfile_FlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_FlangeEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetFlangeSlope() const
    {
    return GetPropertyValueDouble(PRF_PROP_TShapeProfile_FlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_FlangeSlope, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetWebEdgeRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_TShapeProfile_WebEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWebEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_WebEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TShapeProfile::GetWebSlope() const
    {
    return GetPropertyValueDouble(PRF_PROP_TShapeProfile_WebSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TShapeProfile::SetWebSlope(double val)
    {
    SetPropertyValue(PRF_PROP_TShapeProfile_WebSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
