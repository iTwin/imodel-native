/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/StandardCatalogCodeAspect.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\StandardCatalogCodeAspect.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (StandardCatalogCodeAspectHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCatalogCodeAspect::StandardCatalogCodeAspect (StandardCatalogCode const& code)
    : manufacturer (code.manufacturer)
    , standardsOrganization (code.standardsOrganization)
    , revision (code.revision)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StandardCatalogCodeAspect::_LoadProperties (DgnElement const& element)
    {
    Utf8CP pSqlString = "SELECT " PRF_PROP_StandardCatalogCodeAspect_Manufacturer
                           ", "   PRF_PROP_StandardCatalogCodeAspect_StandardsOrganization
                           ", "   PRF_PROP_StandardCatalogCodeAspect_Revision
                        " FROM " PRF_SCHEMA (PRF_CLASS_StandardCatalogCodeAspect)
                        " WHERE Element.Id=?";

    ECSqlStatement sqlStatement;
    ECSqlStatus ecStatus = sqlStatement.Prepare (element.GetDgnDb(), pSqlString);
    if (ecStatus != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    ecStatus = sqlStatement.BindId (1, element.GetElementId());
    if (ecStatus != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    if (sqlStatement.Step() != BE_SQLITE_ROW)
        return DgnDbStatus::ReadError;

    manufacturer = sqlStatement.GetValueText (0);
    standardsOrganization = sqlStatement.GetValueText (1);
    revision = sqlStatement.GetValueText (2);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StandardCatalogCodeAspect::_UpdateProperties (DgnElement const& element, ECCrudWriteToken const* writeToken)
    {
    Utf8CP pSqlString = "UPDATE ONLY " PRF_SCHEMA (PRF_CLASS_StandardCatalogCodeAspect) " SET "
                                PRF_PROP_StandardCatalogCodeAspect_Manufacturer "=?, "
                                PRF_PROP_StandardCatalogCodeAspect_StandardsOrganization "=?, "
                                PRF_PROP_StandardCatalogCodeAspect_Revision "=? "
                        " WHERE ECInstanceId=?";

    DgnDb& db = element.GetDgnDb();
    CachedECSqlStatementPtr sqlStatementPtr = db.GetNonSelectPreparedECSqlStatement (pSqlString, writeToken);
    if (sqlStatementPtr.IsNull())
        return DgnDbStatus::SQLiteError;

    sqlStatementPtr->BindText (1, manufacturer.c_str(), IECSqlBinder::MakeCopy::No, (int)manufacturer.SizeInBytes());
    sqlStatementPtr->BindText (2, standardsOrganization.c_str(), IECSqlBinder::MakeCopy::No, (int)manufacturer.SizeInBytes());
    sqlStatementPtr->BindText (3, revision.c_str(), IECSqlBinder::MakeCopy::No, (int)manufacturer.SizeInBytes());
    sqlStatementPtr->BindId (4, GetAspectInstanceId());

    if (sqlStatementPtr->Step() != BE_SQLITE_DONE)
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StandardCatalogCodeAspect::_GetPropertyValue (ECValue& value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    return DgnDbStatus::NotEnabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StandardCatalogCodeAspect::_SetPropertyValue (Utf8CP propertyName, ECValue const& value, PropertyArrayIndex const& arrayIndex)
    {
    return DgnDbStatus::NotEnabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCatalogCodeAspectCPtr StandardCatalogCodeAspect::Get (Profile const& profile)
    {
    ECClass const* pAspectClass = QueryClass (profile.GetDgnDb());
    if (pAspectClass == nullptr)
        {
        BeAssert (false);
        return nullptr;
        }

    return T_Super::Get<StandardCatalogCodeAspect> (profile, *pAspectClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCatalogCodeAspectPtr StandardCatalogCodeAspect::GetForEdit (Profile& profile)
    {
    ECClass const* pAspectClass = QueryClass (profile.GetDgnDb());
    if (pAspectClass == nullptr)
        {
        BeAssert (false);
        return nullptr;
        }

    return GetP<StandardCatalogCodeAspect> (profile, *pAspectClass);
    }

END_BENTLEY_PROFILES_NAMESPACE
