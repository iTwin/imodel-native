/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/Profile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\Profile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Profile::CreateParams::CreateParams (Dgn::DgnModel const& model, Dgn::DgnClassId const& classId, Utf8CP pName)
    : T_Super (model.GetDgnDb(), model.GetModelId(), classId)
    , name (pName)
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
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Profile::_Validate() const
    {
    Utf8String name = GetName();
    if (Utf8String::IsNullOrEmpty (name.c_str()))
        return BSIERROR;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr Profile::_CreateGeometry() const
    {
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::_OnInsert()
    {
    DgnDbStatus status = ValidateAndCreateGeometry();
    if (status != DgnDbStatus::Success)
        return status;

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::_OnUpdate (DgnElement const& original)
    {
    DgnDbStatus status = ValidateAndCreateGeometry();
    if (status != DgnDbStatus::Success)
        return status;

    return T_Super::_OnUpdate (original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::ValidateAndCreateGeometry()
    {
    if (_Validate() != BSISUCCESS)
        return DgnDbStatus::ValidationFailed;

    IGeometryPtr geometryPtr = _CreateGeometry();
    if (geometryPtr.IsNull())
        return DgnDbStatus::NoGeometry;

    SetShape (geometryPtr);
    return DgnDbStatus::Success;
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
