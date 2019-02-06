/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/Profile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesLogging.h>
#include <ProfilesInternal\ProfilesQuery.h>
#include <ProfilesInternal\ArbitraryCompositeProfileAspect.h>
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
* Validation for CodeValues should come from platform and CodeSpec, CodeSpecFragment's,
* but today platform doesn't provide it - detailed validation of CodeValue is skipped
* here too ..
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCatalogCode Profile::GetStandardCatalogCode() const
    {
    Utf8String const& codeValueStr = GetCode().GetValue().GetUtf8();

    size_t offset = 0;
    Utf8String tokens[4];
    int tokenIndex = 0;
    for (; tokenIndex < _countof (tokens); ++tokenIndex)
        {
        offset = codeValueStr.GetNextToken (tokens[tokenIndex], PRF_CODEVALUE_DELIM_StandardCatalogProfile, offset);
        if (offset == Utf8String::npos)
            break;
        }

    // Validate values that are too short and don't fill out StandardCatalogCode
    if (tokenIndex != _countof (tokens))
        return StandardCatalogCode();

    StandardCatalogCode catalogCode;
    catalogCode.manufacturer = tokens[0].c_str();
    catalogCode.standardsOrganization = tokens[1].c_str();
    catalogCode.revision = tokens[2].c_str();
    catalogCode.designation = tokens[3].c_str();

    return catalogCode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::SetStandardCatalogCode (StandardCatalogCode const& catalogCode)
    {
    Utf8CP pDesignation = GetName().c_str();
    Utf8CP pManufacturer = catalogCode.manufacturer.c_str();
    Utf8CP pStandardsOrganization = catalogCode.standardsOrganization.c_str();
    Utf8CP pRevision = catalogCode.revision.c_str();

    if (Utf8String::IsNullOrEmpty (pDesignation) || Utf8String::IsNullOrEmpty (pManufacturer) ||
        Utf8String::IsNullOrEmpty (pStandardsOrganization) ||Utf8String::IsNullOrEmpty (pRevision))
        return DgnDbStatus::InvalidCode;

    Utf8String codeValue;
    codeValue.Sprintf (PRF_CODEVALUE_FORMAT_StandardCatalogProfile, pManufacturer, pStandardsOrganization, pRevision, pDesignation);

    CodeSpecCPtr codeSpecPtr = m_dgndb.CodeSpecs().GetCodeSpec (PRF_CODESPEC_StandardCatalogProfile);
    if (codeSpecPtr.IsNull())
        return DgnDbStatus::InvalidCodeSpec;

    DgnCode code = codeSpecPtr->CreateCode (codeValue);

    return SetCode (code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Profile::SetStandardCatalogCode (nullptr_t)
    {
    CodeSpecCPtr codeSpecPtr = m_dgndb.CodeSpecs().GetCodeSpec (PRF_CODESPEC_StandardCatalogProfile);
    if (codeSpecPtr.IsNull())
        return DgnDbStatus::InvalidCodeSpec;

    DgnCode code = codeSpecPtr->CreateCode ("");

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

END_BENTLEY_PROFILES_NAMESPACE
