/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/Profile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
    , m_geometryUpdated (false)
    {
    if (params.m_isLoadingElement)
        return;

    SetName (params.name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Profile::_Validate() const
    {
    Utf8String name = GetName();
    if (Utf8String::IsNullOrEmpty (name.c_str()))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::_OnInsert()
    {
    BeAssert (!m_geometryUpdated && "New profile doesn't have a geometry yet");
    if (!_Validate())
        return DgnDbStatus::ValidationFailed;

    if (!CreateGeometry())
        return DgnDbStatus::NoGeometry;

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::_OnUpdate (DgnElement const& original)
    {
    if (!_Validate())
        return DgnDbStatus::ValidationFailed;

    if (!CreateGeometry())
        return DgnDbStatus::NoGeometry;

    return T_Super::_OnUpdate (original);
    }

/*---------------------------------------------------------------------------------**//**
* Profiles should override this method to update geometry of profiles that reference/
* depend on it. See to Profile::UpdateGeometry.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::_UpdateInDb()
    {
    // TODO Karolis: Update geometry for Profiles that are referencing this profile:
    // DerivedProfile, ArbitraryCompositeProfile

    m_geometryUpdated = true;
    return T_Super::_UpdateInDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::_OnDelete() const
    {
    // TODO Karolis: Prohibit deletion of this profile if it is being referenced by profiles:
    // DerivedProfile, ArbitraryCompositeProfile

    return T_Super::_OnDelete();
    }

/*---------------------------------------------------------------------------------**//**
* Create and set geometry. Regular geometry creation is skipped if profiles geometry
* was updated via a call to Profile::UpdateGeometry.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool Profile::CreateGeometry()
    {
    if (m_geometryUpdated)
        return true;

    IGeometryPtr geometryPtr = _CreateGeometry();
    if (geometryPtr.IsNull())
        return false;
    SetShape (*geometryPtr);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Public, non exported method for internal use only. Used to update geometry of profiles
* that reference/depend on 'relatedProfile'. This method should be called in
* _UpdateInDb() passing 'this' as a parameter to profiles that need the geometry update
* e.g. LShapeProfile should call doubleLShapeProfile->UpdateGeometry (this); folowed
* by a doubleLShapeProfile->Update() to save geometry changes.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::UpdateGeometry (Profile const& relatedProfile)
    {
    BeAssert (!m_geometryUpdated && "UpdateGeometry should only be called once before an Update to the element");

    IGeometryPtr geometryPtr = _UpdateGeometry (relatedProfile);
    if (geometryPtr.IsNull())
        return DgnDbStatus::NoGeometry;

    SetShape (*geometryPtr);
    m_geometryUpdated = true;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::Validate() const
    {
    return _Validate() ? DgnDbStatus::Success : DgnDbStatus::ValidationFailed;
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
void Profile::SetShape (IGeometry const& val)
    {
    ECN::ECValue ecValue;
    ecValue.SetIGeometry (val);
    SetPropertyValue (PRF_PROP_Profile_Shape, ecValue);
    }

END_BENTLEY_PROFILES_NAMESPACE
