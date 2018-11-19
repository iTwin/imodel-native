/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CircleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CircleProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CircleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CircleProfilePtr CircleProfile::Create (DgnModelCR model)
    {
    CreateParams params (model.GetDgnDb(), model.GetModelId(), QueryClassId (model.GetDgnDb()));
    return new CircleProfile (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CircleProfile::GetRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CircleProfile_Radius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CircleProfile::SetRadius (double val)
    {
    SetPropertyValue (PRF_PROP_CircleProfile_Radius, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
