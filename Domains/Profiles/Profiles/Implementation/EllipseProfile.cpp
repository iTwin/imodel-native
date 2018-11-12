/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/EllipseProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\EllipseProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(EllipseProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
EllipseProfilePtr EllipseProfile::Create(DgnModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new EllipseProfile(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double EllipseProfile::GetXRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_EllipseProfile_XRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void EllipseProfile::SetXRadius(double val)
    {
    SetPropertyValue(PRF_PROP_EllipseProfile_XRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double EllipseProfile::GetYRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_EllipseProfile_XRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void EllipseProfile::SetYRadius(double val)
    {
    SetPropertyValue(PRF_PROP_EllipseProfile_XRadius, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
