/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/DerivedProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\DerivedProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (DerivedProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DerivedProfilePtr DerivedProfile::Create (/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool DerivedProfile::GetMirrorProfileAboutYAxis() const
    {
    return GetPropertyValueBoolean (PRF_PROP_DerivedProfile_MirrorProfileAboutYAxis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetMirrorProfileAboutYAxis (bool val)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_MirrorProfileAboutYAxis, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d DerivedProfile::GetOffset() const
    {
    return GetPropertyValueDPoint2d (PRF_PROP_DerivedProfile_Offset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetOffset (DPoint2dCR val)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_Offset, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d DerivedProfile::GetRotation() const
    {
    return GetPropertyValueDPoint2d (PRF_PROP_DerivedProfile_Rotation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetRotation (DPoint2dCR val)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_Rotation, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d DerivedProfile::GetScale() const
    {
    return GetPropertyValueDPoint2d (PRF_PROP_DerivedProfile_Scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedProfile::SetScale (DPoint2dCR val)
    {
    SetPropertyValue (PRF_PROP_DerivedProfile_Scale, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
