/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ArbitraryCompositeProfileAspect.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ArbitraryCompositeProfileAspectHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfileAspect::ArbitraryCompositeProfileAspect (ArbitraryCompositeProfileComponent const& component)
    : singleProfileId (component.singleProfileId)
    , offset (component.offset)
    , rotation (component.rotation)
    , mirrorAboutYAxis (component.mirrorAboutYAxis)
    , memberPriority (component.GetMemberPriority())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfileAspect::_LoadProperties (DgnElement const& element)
    {
    Utf8CP pSqlString = "SELECT " PRF_PROP_ArbitraryCompositeProfileAspect_SingleProfile ".Id"
                           ", \"" PRF_PROP_ArbitraryCompositeProfileAspect_Offset "\""
                           ", "   PRF_PROP_ArbitraryCompositeProfileAspect_Rotation
                           ", "   PRF_PROP_ArbitraryCompositeProfileAspect_MirrorAboutYAxis
                           ", "   PRF_PROP_ArbitraryCompositeProfileAspect_MemberPriority
                        " FROM " PRF_SCHEMA (PRF_CLASS_ArbitraryCompositeProfileAspect)
                        " WHERE ECInstanceId=?";

    ECSqlStatement sqlStatement;
    ECSqlStatus ecStatus = sqlStatement.Prepare (element.GetDgnDb(), pSqlString);
    if (ecStatus != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    ecStatus = sqlStatement.BindId (1, GetAspectInstanceId());
    if (ecStatus != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    if (sqlStatement.Step() != BE_SQLITE_ROW)
        return DgnDbStatus::ReadError;

    singleProfileId = sqlStatement.GetValueId<DgnElementId> (0);
    offset = sqlStatement.GetValuePoint2d (1);
    rotation = Angle::FromRadians (sqlStatement.GetValueDouble (2));
    mirrorAboutYAxis = sqlStatement.GetValueBoolean (3);
    memberPriority = sqlStatement.GetValueInt (4);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfileAspect::_UpdateProperties (DgnElement const& element, ECCrudWriteToken const* writeToken)
    {
    Utf8CP pSqlString = "UPDATE ONLY " PRF_SCHEMA (PRF_CLASS_ArbitraryCompositeProfileAspect) " SET "
                                PRF_PROP_ArbitraryCompositeProfileAspect_SingleProfile "=?, "
                           "\"" PRF_PROP_ArbitraryCompositeProfileAspect_Offset "\"=?, "
                                PRF_PROP_ArbitraryCompositeProfileAspect_Rotation "=?, "
                                PRF_PROP_ArbitraryCompositeProfileAspect_MirrorAboutYAxis "=?, "
                                PRF_PROP_ArbitraryCompositeProfileAspect_MemberPriority "=? "
                        " WHERE ECInstanceId=?";

    DgnDb& db = element.GetDgnDb();
    CachedECSqlStatementPtr sqlStatementPtr = db.GetNonSelectPreparedECSqlStatement (pSqlString, writeToken);
    if (sqlStatementPtr.IsNull())
        return DgnDbStatus::SQLiteError;

    sqlStatementPtr->BindNavigationValue (1, singleProfileId);
    sqlStatementPtr->BindPoint2d (2, offset);
    sqlStatementPtr->BindDouble (3, rotation.Radians());
    sqlStatementPtr->BindBoolean (4, mirrorAboutYAxis);
    sqlStatementPtr->BindInt (5, memberPriority);
    sqlStatementPtr->BindId (6, GetAspectInstanceId());

    if (sqlStatementPtr->Step() != BE_SQLITE_DONE)
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfileAspect::_GetPropertyValue (ECValue& value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    return DgnDbStatus::NotEnabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfileAspect::_SetPropertyValue (Utf8CP propertyName, ECValue const& value, PropertyArrayIndex const& arrayIndex)
    {
    return DgnDbStatus::NotEnabled;
    }

END_BENTLEY_PROFILES_NAMESPACE
