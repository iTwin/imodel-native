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

HANDLER_DEFINE_MEMBERS(HollowEllipseProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowEllipseProfilePtr HollowEllipseProfile::Create(DgnModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new HollowEllipseProfile(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowEllipseProfile::GetXRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_HollowEllipseProfile_XRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowEllipseProfile::SetXRadius(double val)
    {
    SetPropertyValue(PRF_PROP_HollowEllipseProfile_XRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowEllipseProfile::GetYRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_HollowEllipseProfile_YRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowEllipseProfile::SetYRadius(double val)
    {
    SetPropertyValue(PRF_PROP_HollowEllipseProfile_YRadius, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
