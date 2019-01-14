/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ArbitraryCompositeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesGeometry.h>
#include <Profiles\ArbitraryCompositeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ArbitraryCompositeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CompositeProfileComponent::CompositeProfileComponent (SinglePerimeterProfile const& singleProfile, bool mirrorAboutYAxis, DPoint2d const& offset, Angle const& rotation)
    : singleProfileId (singleProfile.GetElementId())
    , mirrorAboutYAxis (mirrorAboutYAxis)
    , offset (offset)
    , rotation (rotation)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CompositeProfileComponent::CompositeProfileComponent (DgnElementId const& singleProfileId, bool mirrorAboutYAxis, DPoint2d const& offset, Angle const& rotation)
    : singleProfileId (singleProfileId)
    , mirrorAboutYAxis (mirrorAboutYAxis)
    , offset (offset)
    , rotation (rotation)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName, bvector<CompositeProfileComponent> const& components)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , components (components)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfile::ArbitraryCompositeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetComponents (params.components);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArbitraryCompositeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    if (m_components.empty())
        return false;

    for (CompositeProfileComponent const& component : m_components)
        {
        SinglePerimeterProfilePtr singleProfilePtr = SinglePerimeterProfile::GetForEdit (m_dgndb, component.singleProfileId);
        if (singleProfilePtr.IsNull())
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ArbitraryCompositeProfile::_CreateGeometry() const
    {
    return ProfilesGeometry::CreateArbitraryCompositeShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_InsertInDb()
    {
    int memberPriority = 0;
    for (CompositeProfileComponent const& component : m_components)
        {
        DgnDbStatus status;
        status = InsertRelationship (component, memberPriority++);
        if (status != DgnDbStatus::Success)
            return status;
        }

    return T_Super::_InsertInDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_OnUpdate (DgnElement const& original)
    {
    // TODO Karolis: Update temporary not allowed.
    return DgnDbStatus::ReadOnly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_OnDelete() const
    {
    // TODO Karolis: Delete temporary not allowed.
    return DgnDbStatus::DeletionProhibited;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::InsertRelationship (CompositeProfileComponent const& component, int memberPriority)
    {
    ECRelationshipClass const* pRelationshipClass = QueryClass (m_dgndb, PRF_REL_ArbitraryCompositeProfileRefersToSinglePerimeterProfiles)->GetRelationshipClassCP();
    if (pRelationshipClass == nullptr)
        DgnDbStatus::SQLiteError;

    StandaloneECRelationshipEnablerPtr relationshipEnablerPtr = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*pRelationshipClass);
    if (relationshipEnablerPtr.IsNull())
        DgnDbStatus::SQLiteError;

    StandaloneECRelationshipInstancePtr relationshipInstancePtr = relationshipEnablerPtr->CreateRelationshipInstance();
    if (relationshipInstancePtr.IsNull())
        DgnDbStatus::SQLiteError;

    std::array<ECObjectsStatus, 4> ecStatusArray =
        {
        relationshipInstancePtr->SetValue (PRF_PROP_ArbitraryCompositeProfileRefersToSinglePerimeterProfiles_MemberPriority, ECValue (memberPriority)),
        relationshipInstancePtr->SetValue (PRF_PROP_ArbitraryCompositeProfileRefersToSinglePerimeterProfiles_MirrorProfileAboutYAxis, ECValue (component.mirrorAboutYAxis)),
        relationshipInstancePtr->SetValue (PRF_PROP_ArbitraryCompositeProfileRefersToSinglePerimeterProfiles_Offset, ECValue (component.offset)),
        relationshipInstancePtr->SetValue (PRF_PROP_ArbitraryCompositeProfileRefersToSinglePerimeterProfiles_Rotation, ECValue (component.rotation.Radians()))
        };
    if (std::any_of (ecStatusArray.begin(), ecStatusArray.end(), [](ECObjectsStatus status) { return status != ECObjectsStatus::Success; }))
        return DgnDbStatus::SQLiteError;

    ECInstanceKey relationshipKey;
    BeSQLite::DbResult dbResult = m_dgndb.InsertLinkTableRelationship (relationshipKey, *pRelationshipClass, m_elementId, component.singleProfileId, relationshipInstancePtr.get());
    if (dbResult != BeSQLite::BE_SQLITE_OK)
        return DgnDbStatus::SQLiteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CompositeProfileComponent> ArbitraryCompositeProfile::GetComponents() const
    {
    bvector<CompositeProfileComponent> components (m_components);
    for (auto& component : components)
        component.singleProfilePtr = SinglePerimeterProfile::GetForEdit (m_dgndb, component.singleProfileId);

    return components;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryCompositeProfile::SetComponents (bvector<CompositeProfileComponent> const& components)
    {
    m_components = components;
    }

END_BENTLEY_PROFILES_NAMESPACE
