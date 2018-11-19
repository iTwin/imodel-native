/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/SchifflerizedLShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
void SchifflerizedLShapeProfile::SetLegLength (double val)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_LegLength, ECN::ECValue (val));
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
void SchifflerizedLShapeProfile::SetThickness (double val)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_Thickness, ECN::ECValue (val));
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
void SchifflerizedLShapeProfile::SetFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_FilletRadius, ECN::ECValue (val));
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
void SchifflerizedLShapeProfile::SetEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_EdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetLegSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_LegSlope);
    }

void SchifflerizedLShapeProfile::SetLegSlope (double val)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_LegSlope, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetAngle() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_Angle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetAngle (double val)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_Angle, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
