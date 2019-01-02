/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/CenterLineLShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CenterLineLShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CenterLineLShapeProfileHandler)

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
