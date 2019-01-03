/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/SchifflerizedLShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles/SchifflerizedLShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (SchifflerizedLShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetLegLength() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_LegLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetLegLength (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_LegLength, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetThickness (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_Thickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetLegBendOffset() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_LegBendOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetLegBendOffset (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_LegBendOffset, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_FilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_EdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_EdgeRadius, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
