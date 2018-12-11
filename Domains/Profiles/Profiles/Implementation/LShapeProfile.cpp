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

HANDLER_DEFINE_MEMBERS (LShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetWidth (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Width, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetThickness (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Thickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_FilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_EdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_EdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetLegSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_LegSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetLegSlope (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_LegSlope, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
