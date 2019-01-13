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
* Perform ECSql SELECT to query first Profile that has a navigation property referencing
* 'referencedProfileId', returns invalid DgnElementId if no one is referencing the profile.
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

    ECSqlStatement sqlStatement;
    ECSqlStatus status = sqlStatement.Prepare (db, sqlString.c_str());
    if (status != ECSqlStatus::Success)
        {
        *pStatus = DgnDbStatus::SQLiteError;
        return DgnElementId();
        }

    status = sqlStatement.BindId (1, referencedProfileId);
    if (status != ECSqlStatus::Success)
        {
        *pStatus = DgnDbStatus::SQLiteError;
        return DgnElementId();
        }

    DgnElementId elementId;
    if (sqlStatement.Step() == DbResult::BE_SQLITE_ROW)
        elementId = sqlStatement.GetValueId<DgnElementId> (0);

    *pStatus = DgnDbStatus::Success;
    return elementId;
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
