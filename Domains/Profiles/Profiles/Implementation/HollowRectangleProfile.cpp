/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/HollowRectangleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\HollowRectangleProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (HollowRectangleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowRectangleProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowRectangleProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowRectangleProfile::SetWidth (double val)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_Width, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowRectangleProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowRectangleProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowRectangleProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowRectangleProfile::GetInnerFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowRectangleProfile_InnerFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowRectangleProfile::SetInnerFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_InnerFilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowRectangleProfile::GetOuterFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowRectangleProfile_OuterFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowRectangleProfile::SetOuterFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_OuterFilletRadius, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
