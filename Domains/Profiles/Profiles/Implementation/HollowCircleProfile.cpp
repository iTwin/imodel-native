/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/HollowCircleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\HollowCircleProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (HollowCircleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowCircleProfilePtr HollowCircleProfile::Create (DgnModelCR model)
    {
    CreateParams params (model.GetDgnDb(), model.GetModelId(), QueryClassId (model.GetDgnDb()));
    return new HollowCircleProfile (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowCircleProfile::GetRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowCircleProfile_Radius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowCircleProfile::SetRadius (double val)
    {
    SetPropertyValue (PRF_PROP_HollowCircleProfile_Radius, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
