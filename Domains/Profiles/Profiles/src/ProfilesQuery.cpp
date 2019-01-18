/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ProfilesQuery.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesLogging.h>
#include <ProfilesInternal\ProfilesQuery.h>
#include <Profiles\DoubleCShapeProfile.h>
#include <Profiles\DoubleLShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_BENTLEY_PROFILES_NAMESPACE

// Explicitly instantiate template query methods for "double shapes"
template bvector<DoubleCShapeProfilePtr> ProfilesQuery::SelectByNavigationProperty (DgnDb const&, DgnElementId const&, Utf8CP, Utf8CP, DgnDbStatus*);
template bvector<DoubleLShapeProfilePtr> ProfilesQuery::SelectByNavigationProperty (DgnDb const&, DgnElementId const&, Utf8CP, Utf8CP, DgnDbStatus*);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId selectFirst (DgnDb const& db, Utf8CP pSqlString, DgnElementId const& referencedProfileId, DgnDbStatus& status)
    {
    ECSqlStatement sqlStatement;
    ECSqlStatus ecStatus = sqlStatement.Prepare (db, pSqlString);
    if (ecStatus != ECSqlStatus::Success)
        {
        status = DgnDbStatus::SQLiteError;
        return DgnElementId();
        }

    ecStatus = sqlStatement.BindId (1, referencedProfileId);
    if (ecStatus != ECSqlStatus::Success)
        {
        status = DgnDbStatus::SQLiteError;
        return DgnElementId();
        }

    DgnElementId elementId;
    if (sqlStatement.Step() == BE_SQLITE_ROW)
        elementId = sqlStatement.GetValueId<DgnElementId> (0);

    status = DgnDbStatus::Success;
    return elementId;
    }

/*---------------------------------------------------------------------------------**//**
* Perform ECSql SELECT to query first Profile that has a navigation property referencing
* 'referencedProfileId'. Returns id of the PROFILE that is referencing and invalid
* DgnElementId if profile is not referenced by the 'pClassName'.
* @param referencedProfileId - id of profile to search for in navigation properties.
* @param pClassName - name of ECEntityClass to perform the query on.
* @param pNavigationPropertyName - name of ECNavigationProperty to perform the query on.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ProfilesQuery::SelectFirstByNavigationProperty (DgnDb const& db, DgnElementId const& referencedProfileId,
                                                             Utf8CP pClassName, Utf8CP pNavigationPropertyName, DgnDbStatus* pStatus)
    {
    DgnDbStatus dbStatus;
    if (pStatus == nullptr)
        pStatus = &dbStatus;

    Utf8String sqlString;
    sqlString += "SELECT ECInstanceId FROM " PRF_SCHEMA_NAME "." + Utf8String (pClassName) +
                 " WHERE " + Utf8String (pNavigationPropertyName) + ".Id=? LIMIT 1";

    return selectFirst (db, sqlString.c_str(), referencedProfileId, *pStatus);
    }

/*---------------------------------------------------------------------------------**//**
* Perform ECSql SELECT to query first Profiles Aspect that has a navigation property
* referencing 'referencedProfileId'. Returns id of the HOST PROFILE of the 'pAspectClass'
* and invalid DgnElementId if profile is not referenced by the 'pAspectName'.
* @param referencedProfileId - id of profile to search for in navigation properties.
* @param pAspectName - name of Aspect ECEntityClass to perform the query on.
* @param pNavigationPropertyName - name of ECNavigationProperty to perform the query on.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ProfilesQuery::SelectFirstByAspectNavigationProperty (DgnDb const& db, DgnElementId const& referencedProfileId,
                                                                   Utf8CP pAspectName, Utf8CP pNavigationPropertyName, DgnDbStatus* pStatus)
    {
    DgnDbStatus dbStatus;
    if (pStatus == nullptr)
        pStatus = &dbStatus;

    Utf8String sqlString;
    sqlString += "SELECT Element.Id FROM " PRF_SCHEMA_NAME "." + Utf8String (pAspectName) +
                 " WHERE " + Utf8String (pNavigationPropertyName) + ".Id=? LIMIT 1";

    return selectFirst (db, sqlString.c_str(), referencedProfileId, *pStatus);

    }

/*---------------------------------------------------------------------------------**//**
* Perform EcSql SELECT to query all Profiles that have a navigation property referencing
* 'referencedProfileId'.
* @param referencedProfileId - id of profile to search for in navigation properties.
* @param pClassName - name of ECEntityClass to perform the query on.
* @param pNavigationPropertyName - name of ECNavigationProperty to perform the query on.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
bvector<RefCountedPtr<T>> ProfilesQuery::SelectByNavigationProperty (DgnDb const& db, DgnElementId const& referencedProfileId,
                                                                     Utf8CP pClassName, Utf8CP pNavigationPropertyName, DgnDbStatus* pStatus)
    {
    DgnDbStatus dbStatus;
    if (pStatus == nullptr)
        pStatus = &dbStatus;

    Utf8String sqlString;
    sqlString += "SELECT ECInstanceId FROM " PRF_SCHEMA_NAME "." + Utf8String (pClassName) +
                 " WHERE " + Utf8String (pNavigationPropertyName) + ".Id=?";

    ECSqlStatement sqlStatement;
    ECSqlStatus sqlStatus = sqlStatement.Prepare (db, sqlString.c_str());
    if (sqlStatus != ECSqlStatus::Success)
        {
        *pStatus = DgnDbStatus::SQLiteError;
        return bvector<RefCountedPtr<T>>();
        }

    sqlStatus = sqlStatement.BindId (1, referencedProfileId);
    if (sqlStatus != ECSqlStatus::Success)
        {
        *pStatus = DgnDbStatus::SQLiteError;
        return bvector<RefCountedPtr<T>>();
        }

    bvector<RefCountedPtr<T>> profiles;
    while (sqlStatement.Step() == DbResult::BE_SQLITE_ROW)
        {
        DgnElementId profileId = sqlStatement.GetValueId<DgnElementId> (0);
        RefCountedPtr<T> profilePtr = typename T::GetForEdit (db, profileId);
        if (profilePtr.IsNull())
            {
            *pStatus = DgnDbStatus::BadElement;
            return bvector<RefCountedPtr<T>>();
            }
        profiles.push_back (profilePtr);
        }

    *pStatus = DgnDbStatus::Success;
    return profiles;
    }

END_BENTLEY_PROFILES_NAMESPACE
