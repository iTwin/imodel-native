/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/LShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\LShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(LShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LShapeProfilePtr LShapeProfile::Create(DgnModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new LShapeProfile(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_LShapeProfile_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetThickness(double val)
    {
    SetPropertyValue(PRF_PROP_LShapeProfile_Thickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_LShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_LShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetEdgeRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_LShapeProfile_EdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_LShapeProfile_EdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetHorizontalLegSlope() const
    {
    return GetPropertyValueDouble(PRF_PROP_LShapeProfile_HorizontalLegSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetHorizontalLegSlope(double val)
    {
    SetPropertyValue(PRF_PROP_LShapeProfile_HorizontalLegSlope, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetVerticalLegSlope() const
    {
    return GetPropertyValueDouble(PRF_PROP_LShapeProfile_VerticalLegSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetVerticalLegSlope(double val)
    {
    SetPropertyValue(PRF_PROP_LShapeProfile_VerticalLegSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
