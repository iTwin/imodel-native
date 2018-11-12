/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/HollowEllipseProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\HollowEllipseProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(HollowCircleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowCircleProfilePtr HollowCircleProfile::Create(DgnModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new HollowCircleProfile(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowCircleProfile::GetXRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_HollowCircleProfile_XRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowCircleProfile::SetXRadius(double val)
    {
    SetPropertyValue(PRF_PROP_HollowCircleProfile_XRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowCircleProfile::GetYRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_HollowCircleProfile_YRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowCircleProfile::SetYRadius(double val)
    {
    SetPropertyValue(PRF_PROP_HollowCircleProfile_YRadius, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
