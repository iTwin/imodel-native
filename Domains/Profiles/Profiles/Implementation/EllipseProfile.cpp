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

HANDLER_DEFINE_MEMBERS(CircleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CircleProfilePtr CircleProfile::Create(DgnModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new CircleProfile(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CircleProfile::GetXRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_CircleProfile_XRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CircleProfile::SetXRadius(double val)
    {
    SetPropertyValue(PRF_PROP_CircleProfile_XRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CircleProfile::GetYRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_CircleProfile_XRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CircleProfile::SetYRadius(double val)
    {
    SetPropertyValue(PRF_PROP_CircleProfile_XRadius, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
