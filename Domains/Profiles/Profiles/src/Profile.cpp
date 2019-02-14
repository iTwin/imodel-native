/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/Profile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesCardinalPoints.h>
#include <ProfilesInternal\ProfilesLogging.h>
#include <ProfilesInternal\ProfilesQuery.h>
#include <ProfilesInternal\ArbitraryCompositeProfileAspect.h>
#include <ProfilesInternal\StandardCatalogCodeAspect.h>
#include <Profiles\Profile.h>

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

    if (!_CreateGeometry())
        return DgnDbStatus::NoGeometry;

    DgnDbStatus status = ProfilesCardinalPoints::AddStandardCardinalPoints (*this);
    if (status != DgnDbStatus::Success)
        return status;

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::_OnUpdate (DgnElement const& original)
    {
    if (!_Validate())
        return DgnDbStatus::ValidationFailed;

    if (!_CreateGeometry())
        return DgnDbStatus::NoGeometry;

    return T_Super::_OnUpdate (original);
    }

/*---------------------------------------------------------------------------------**//**
* Profiles should override this method to update geometry of profiles that reference/
* depend on it. If overriden, T_Super must be called. See Profile::UpdateGeometry.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::_UpdateInDb()
    {
    // Finishing update operation for this Profile - set flag for geometry update for new calls to Update().
    m_geometryUpdated = false;

    return T_Super::_UpdateInDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void Profile::_CopyFrom (DgnElement const& source)
    {
    if (auto const* pSourceProfile = dynamic_cast<Profile const*> (&source))
        m_geometryUpdated = pSourceProfile->m_geometryUpdated;
    else
        ProfilesLog::FailedCopyFrom_InvalidElement (PRF_CLASS_Profile, m_elementId, source.GetElementId());

    return T_Super::_CopyFrom (source);
    }

/*---------------------------------------------------------------------------------**//**
* Create and set geometry. Regular geometry creation is skipped if profiles geometry
* was updated via a call to Profile::UpdateGeometry.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool Profile::_CreateGeometry()
    {
    if (m_geometryUpdated)
        return true;

    IGeometryPtr geometryPtr = _CreateShapeGeometry();
    if (geometryPtr.IsNull())
        return false;

    BeAssert (geometryPtr->GetGeometryType() == IGeometry::GeometryType::CurveVector);
    SetShape (*geometryPtr);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Public, non exported method for internal use only. Used to update geometry of profiles
* that reference/depend on 'relatedProfile'. This method should be called in
* _UpdateInDb() passing 'this' as a parameter to profiles that need the geometry update.
* Note _UpdateInDb() callback is important here because the profile geometry will get
* updated during _OnUpdate() which happens before _UpdateInDb().
* e.g. LShapeProfile should call doubleLShapeProfile->UpdateGeometry (this); folowed
* by a doubleLShapeProfile->Update() to save geometry changes
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::UpdateGeometry (Profile const& relatedProfile)
    {
    BeAssert (!m_geometryUpdated && "UpdateGeometry should only be called once before an Update to the element");

    IGeometryPtr geometryPtr = _UpdateShapeGeometry (relatedProfile);
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
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCatalogCode Profile::GetStandardCatalogCode() const
    {
    StandardCatalogCodeAspectCPtr aspectPtr = StandardCatalogCodeAspect::Get (*this);
    if (aspectPtr.IsNull())
        return StandardCatalogCode();

    StandardCatalogCode catalogCode (aspectPtr->manufacturer.c_str(), aspectPtr->standardsOrganization.c_str(), aspectPtr->revision.c_str());
    catalogCode.designation = GetName();

    return catalogCode;
    }

/*---------------------------------------------------------------------------------**//**
* Creates unique StandardCatalogCode aspect and sets DgnCode representing it.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::SetStandardCatalogCode (StandardCatalogCode const& catalogCode)
    {
    Utf8String const designation = GetName();

    if (designation.empty() || catalogCode.manufacturer.empty() || catalogCode.standardsOrganization.empty() || catalogCode.revision.empty())
        return DgnDbStatus::InvalidCode;

    StandardCatalogCodeAspectPtr aspectPtr = StandardCatalogCodeAspect::Create (catalogCode);
    StandardCatalogCodeAspect::SetAspect (*this, *aspectPtr);

    Utf8String codeValue;
    codeValue.Sprintf (PRF_CODEVALUE_FORMAT_StandardCatalogProfile, catalogCode.manufacturer.c_str(), catalogCode.standardsOrganization.c_str(),
                       catalogCode.revision.c_str(), designation.c_str());

    CodeSpecCPtr codeSpecPtr = m_dgndb.CodeSpecs().GetCodeSpec (PRF_CODESPEC_StandardCatalogProfile);
    if (codeSpecPtr.IsNull())
        return DgnDbStatus::InvalidCodeSpec;

    DgnCode code = codeSpecPtr->CreateCode (codeValue);

    return SetCode (code);
    }

/*---------------------------------------------------------------------------------**//**
* Deletes the unique StandardCatalogCode aspect and sets DgnCode to null.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::SetStandardCatalogCode (nullptr_t)
    {
    StandardCatalogCodeAspectPtr aspectPtr = StandardCatalogCodeAspect::GetForEdit (*this);
    if (aspectPtr.IsValid())
        aspectPtr->Delete();

    CodeSpecCPtr codeSpecPtr = m_dgndb.CodeSpecs().GetCodeSpec (PRF_CODESPEC_StandardCatalogProfile);
    if (codeSpecPtr.IsNull())
        return DgnDbStatus::InvalidCodeSpec;

    DgnCode code = codeSpecPtr->CreateCode (nullptr);

    return SetCode (code);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CardinalPoint> Profile::GetCardinalPoints() const
    {
    return ProfilesCardinalPoints::GetCardinalPoints (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::GetCardinalPoint (StandardCardinalPoint standardType, CardinalPoint& cardinalPoint) const
    {
    return ProfilesCardinalPoints::GetCardinalPoint (*this, standardType, cardinalPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::GetCardinalPoint (Utf8String const& name, CardinalPoint& cardinalPoint) const
    {
    return ProfilesCardinalPoints::GetCardinalPoint (*this, name.c_str(), cardinalPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::AddCustomCardinalPoint (CardinalPoint const& customCardinalPoint)
    {
    return ProfilesCardinalPoints::AddCustomCardinalPoint (*this, customCardinalPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::RemoveCustomCardinalPoint (Utf8String const& name)
    {
    return ProfilesCardinalPoints::RemoveCustomCardinalPoint (*this, name);
    }

END_BENTLEY_PROFILES_NAMESPACE
