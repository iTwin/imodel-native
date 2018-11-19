/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/Profile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\Profile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Profile::CreateParams::CreateParams (Dgn::DgnModel const& model, Dgn::DgnClassId const& classId)
    : T_Super (model.GetDgnDb(), model.GetModelId(), classId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Profile::Profile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetName (params.name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Profile::GetName() const
    {
    return GetPropertyValueString (PRF_PROP_Profile_Name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Profile::SetName (Utf8String val)
    {
    SetPropertyValue (PRF_PROP_Profile_Name, ECN::ECValue (val.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr Profile::GetShape() const
    {
    ECN::ECValue ecValue;
    GetPropertyValue (ecValue, PRF_PROP_Profile_Shape);
    return ecValue.GetIGeometry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Profile::SetShape (IGeometryPtr val)
    {
    ECN::ECValue ecValue;
    ecValue.SetIGeometry (*val);
    SetPropertyValue (PRF_PROP_Profile_Shape, ecValue);
    }

END_BENTLEY_PROFILES_NAMESPACE
