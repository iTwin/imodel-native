/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ProfilesQuery.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesLogging.h>
#include <ProfilesInternal\ProfilesQuery.h>
#include <Profiles\DoubleCShapeProfile.h>
#include <Profiles\DoubleLShapeProfile.h>
#include <Profiles\DerivedProfile.h>
#include <Profiles\ArbitraryCompositeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

// Explicitly instantiate template query methods
template bvector<DoubleCShapeProfilePtr> ProfilesQuery::SelectByNavigationProperty (DgnDb const&, DgnElementId const&, Utf8CP, Utf8CP, DgnDbStatus*);
template bvector<DoubleLShapeProfilePtr> ProfilesQuery::SelectByNavigationProperty (DgnDb const&, DgnElementId const&, Utf8CP, Utf8CP, DgnDbStatus*);
template bvector<DerivedProfilePtr> ProfilesQuery::SelectByNavigationProperty (DgnDb const&, DgnElementId const&, Utf8CP, Utf8CP, DgnDbStatus*);
template bvector<ArbitraryCompositeProfilePtr> ProfilesQuery::SelectByAspectNavigationProperty (DgnDb const&, DgnElementId const& , Utf8CP , Utf8CP , DgnDbStatus*);

/*---------------------------------------------------------------------------------**//**
* Executes ECSql and returns vector of DgnElementId. Returns empty vector in case of error.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DgnElementId> selectElementIds (DgnDb const& db, Utf8CP pSqlString, DgnElementId const& referencedProfileId, DgnDbStatus& status)
    {
    ECSqlStatement sqlStatement;
    ECSqlStatus ecStatus = sqlStatement.Prepare (db, pSqlString);
    if (ecStatus != ECSqlStatus::Success)
        {
        status = DgnDbStatus::SQLiteError;
        return bvector<DgnElementId>();
        }

    ecStatus = sqlStatement.BindId (1, referencedProfileId);
    if (ecStatus != ECSqlStatus::Success)
        {
        status = DgnDbStatus::SQLiteError;
        return bvector<DgnElementId>();
        }

    bvector<DgnElementId> elementIds;
    while (sqlStatement.Step() == BE_SQLITE_ROW)
        {
        DgnElementId const elementId = sqlStatement.GetValueId<DgnElementId> (0);
        elementIds.push_back (elementId);
        }

    status = DgnDbStatus::Success;
    return elementIds;
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

    bvector<DgnElementId> profileIds = selectElementIds (db, sqlString.c_str(), referencedProfileId, *pStatus);

    return (profileIds.empty() ? DgnElementId() : profileIds[0]);
    }

/*---------------------------------------------------------------------------------**//**
* Perform ECSql SELECT to query first Profile whose Aspect has a navigation property
* referencing 'referencedProfileId'. Returns id of the HOST PROFILE of the 'pAspectClass'
* and invalid DgnElementId if profile is not referenced by the 'pAspectName'.
* @param referencedProfileId - id of profile to search for in navigation properties.
* @param pAspectName - name of Aspect ECEntityClass to perform the query on.
* @param pNavigationPropertyName - name of Aspect ECNavigationProperty to perform the query on.
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

    bvector<DgnElementId> profileIds = selectElementIds (db, sqlString.c_str(), referencedProfileId, *pStatus);

    return (profileIds.empty() ? DgnElementId() : profileIds[0]);
    }

/*---------------------------------------------------------------------------------**//**
* Executes ECSql to query elementIds and returns a vector of corresponding Profiles.
* Returns empty vector in case of error.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static bvector<RefCountedPtr<T>> selectProfiles (DgnDb const& db, DgnElementId const& referencedProfileId,
                                                 Utf8CP pSqlString, DgnDbStatus* pStatus)
    {
    DgnDbStatus dbStatus;
    if (pStatus == nullptr)
        pStatus = &dbStatus;

    bvector<DgnElementId> profileIds = selectElementIds (db, pSqlString, referencedProfileId, *pStatus);
    if (*pStatus != DgnDbStatus::Success)
        return bvector<RefCountedPtr<T>>();

    bvector<RefCountedPtr<T>> profiles;
    profiles.reserve (profileIds.size());

    for (DgnElementId profileId : profileIds)
        {
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
    Utf8String sqlString;
    sqlString += "SELECT DISTINCT ECInstanceId FROM " PRF_SCHEMA_NAME "." + Utf8String (pClassName) +
                 " WHERE " + Utf8String (pNavigationPropertyName) + ".Id=?";

    return selectProfiles<T> (db, referencedProfileId, sqlString.c_str(), pStatus);
    }

/*---------------------------------------------------------------------------------**//**
* Perform ECSql SELECT to query all Profiles whose Aspects have a navigation property
* referencing 'referencedProfileId'. Returns ids of the HOST PROFILES of the 'pAspectClass'
* and an empty vector if profile is not referenced by the 'pAspectName'.
* @param referencedProfileId - id of profile to search for in navigation properties.
* @param pAspectName - name of Aspects ECEntityClass to perform the query on.
* @param pNavigationPropertyName - name of Aspect ECNavigationProperty to perform the query on.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
bvector<RefCountedPtr<T>> ProfilesQuery::SelectByAspectNavigationProperty (DgnDb const& db, DgnElementId const& referencedProfileId,
                                                                           Utf8CP pAspectName, Utf8CP pNavigationPropertyName, DgnDbStatus* pStatus)
    {
    Utf8String sqlString;
    sqlString += "SELECT DISTINCT Element.Id FROM " PRF_SCHEMA_NAME "." + Utf8String (pAspectName) +
                 " WHERE " + Utf8String (pNavigationPropertyName) + ".Id=?";

    return selectProfiles<T> (db, referencedProfileId, sqlString.c_str(), pStatus);
    }

END_BENTLEY_PROFILES_NAMESPACE
