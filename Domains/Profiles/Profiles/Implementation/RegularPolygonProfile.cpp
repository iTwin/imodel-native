/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/RegularPolygonProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\RegularPolygonProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (RegularPolygonProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RegularPolygonProfile::GetSideCount() const
    {
    return GetPropertyValueDouble (PRF_PROP_RegularPolygonProfile_SideCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RegularPolygonProfile::SetSideCount (double val)
    {
    SetPropertyValue (PRF_PROP_RegularPolygonProfile_SideCount, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RegularPolygonProfile::GetSideLength() const
    {
    return GetPropertyValueDouble (PRF_PROP_RegularPolygonProfile_SideLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RegularPolygonProfile::SetSideLength (double val)
    {
    SetPropertyValue (PRF_PROP_RegularPolygonProfile_SideLength, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
