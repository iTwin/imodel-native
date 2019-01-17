/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ArbitraryCompositeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesLogging.h>
#include <ProfilesInternal\ArbitraryCompositeProfileAspect.h>
#include <Profiles\ArbitraryCompositeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ArbitraryCompositeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfileComponent::ArbitraryCompositeProfileComponent (DgnElementId const& singleProfileId, DPoint2d const& offset,
                                                                        Angle const& rotation, bool mirrorAboutYAxis)
    : singleProfileId (singleProfileId)
    , offset (offset)
    , rotation (rotation)
    , mirrorAboutYAxis (mirrorAboutYAxis)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName, bvector<ArbitraryCompositeProfileComponent> const& components)
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

    if (m_components.size() < 2)
        return false;

    for (ArbitraryCompositeProfileComponent const& component : m_components)
        {
        // TODO Karolis: Validate other component properties?
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
* Insert aspects for every component.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_InsertInDb()
    {
    int memberPriority = 0;
    for (ArbitraryCompositeProfileComponent& component : m_components)
        {
        ArbitraryCompositeProfileAspectPtr aspectPtr = ArbitraryCompositeProfileAspect::Create (component, memberPriority++);
        DgnElement::MultiAspect::AddAspect (*this, *aspectPtr);
        }

    return T_Super::_InsertInDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_OnUpdate (DgnElement const& original)
    {
    return T_Super::_OnUpdate (original);
    }

/*---------------------------------------------------------------------------------**//**
* Cleanup all relationships that this profile had.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_OnDelete() const
    {
    /*Utf8CP pSqlString = "DELETE FROM " PRF_SCHEMA (PRF_REL_ArbitraryCompositeProfileRefersToSinglePerimeterProfiles) " WHERE SourceECInstanceId=?";

    ECSqlStatement sqlStatement;
    ECSqlStatus sqlStatus = sqlStatement.Prepare (m_dgndb, pSqlString);
    if (sqlStatus != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    sqlStatus = sqlStatement.BindId (1, m_elementId);
    if (sqlStatus != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    if (sqlStatement.Step() != BE_SQLITE_DONE)
        return DgnDbStatus::SQLiteError;*/

    return T_Super::_OnDelete();
    }

/*---------------------------------------------------------------------------------**//**
* Element is loading from db - query aspects and populate local components vector.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_LoadFromDb()
    {
    Utf8CP pSqlString = "SELECT ECInstanceId FROM " PRF_SCHEMA (PRF_CLASS_ArbitraryCompositeProfileAspect) " WHERE Element.Id=?";

    ECSqlStatement sqlStatement;
    ECSqlStatus sqlStatus = sqlStatement.Prepare (m_dgndb, pSqlString);
    if (sqlStatus != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    sqlStatus = sqlStatement.BindId (1, m_elementId);
    if (sqlStatus != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    BeAssert (m_components.empty() && "Profile is loading from db - components should be empty");
    while (sqlStatement.Step() == BE_SQLITE_ROW)
        {
        ECClass const* pAspectClass = QueryClass (m_dgndb, PRF_CLASS_ArbitraryCompositeProfileAspect);
        ECInstanceId aspectId = sqlStatement.GetValueId<ECInstanceId> (0);

        ArbitraryCompositeProfileAspectCPtr aspectPtr = MultiAspect::Get<ArbitraryCompositeProfileAspect> (*this, *pAspectClass, aspectId);
        if (aspectPtr.IsNull())
            return DgnDbStatus::BadElement;

        ArbitraryCompositeProfileComponent component (aspectPtr->singleProfileId, aspectPtr->offset, aspectPtr->rotation, aspectPtr->mirrorAboutYAxis);
        component.m_memberPriority = aspectPtr->memberPriority;

        // TODO Karolis: make sure compoent is valid.
        m_components.push_back (component);
        }

    return T_Super::_LoadFromDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryCompositeProfile::_CopyFrom (DgnElement const& source)
    {
    if (auto const* pSourceProfile = dynamic_cast<ArbitraryCompositeProfile const*> (&source))
        m_components = pSourceProfile->m_components;
    else
        ProfilesLog::FailedCopyFrom_InvalidElement (PRF_CLASS_ArbitraryCompositeProfile, m_elementId, source.GetElementId());

    return T_Super::_CopyFrom (source);
    }

/*---------------------------------------------------------------------------------**//**
* Return a copy of components vector.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ArbitraryCompositeProfileComponent> ArbitraryCompositeProfile::GetComponents() const
    {
    return m_components;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryCompositeProfile::SetComponents (bvector<ArbitraryCompositeProfileComponent> const& components)
    {
    m_components = components;
    }

END_BENTLEY_PROFILES_NAMESPACE
