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
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowCircleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowCircleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double radius, double wallThickness)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , radius (radius)
    , wallThickness (wallThickness)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowCircleProfile::HollowCircleProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetRadius (params.radius);
    SetWallThickness (params.wallThickness);
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
