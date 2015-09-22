/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/BackDoor/DgnDbUtilities.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnProject/DgnDbUtilities.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2014
//---------------------------------------------------------------------------------------
ECSqlStatus DgnDbUtilities::InsertRelationship (ECInstanceKey& rkey, ECSqlStatement& statement, ECDbR db, Utf8CP relationshipName, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey)
    {
    if (!relationshipName || !*relationshipName)
        return ECSqlStatus::Error;

    if (!sourceKey.IsValid() || !targetKey.IsValid())
        return ECSqlStatus::Error;

    if (!statement.IsPrepared())
        {
        Utf8PrintfString sql ("INSERT INTO %s (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES (?,?,?,?)", relationshipName);

        ECSqlStatus status = statement.Prepare (db, sql.c_str());
        if (!status.IsSuccess())
            return status;
        }
    else
        {
        statement.Reset();
        statement.ClearBindings();
        }

    statement.BindInt64 (1, sourceKey.GetECClassId());
    statement.BindId    (2, sourceKey.GetECInstanceId());
    statement.BindInt64 (3, targetKey.GetECClassId());
    statement.BindId    (4, targetKey.GetECInstanceId());

    if (BE_SQLITE_DONE != statement.Step(rkey))
        return ECSqlStatus::Error;

    return ECSqlStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    12/2014
//---------------------------------------------------------------------------------------
ECInstanceKey DgnDbUtilities::QueryRelationshipSourceFromTarget (ECDbR db, Utf8CP relationshipName, ECInstanceKeyCR targetKey)
    {
    if (!targetKey.IsValid())
        return ECInstanceKey();

    Utf8PrintfString sql ("SELECT SourceECClassId,SourceECInstanceId FROM %s WHERE TargetECClassId=? AND TargetECInstanceId=?", relationshipName);

    ECSqlStatement statement;
    ECSqlStatus status = statement.Prepare (db, sql.c_str());
    if (!status.IsSuccess())
        return ECInstanceKey();

    if (ECSqlStatus::Success != statement.BindInt64 (1, targetKey.GetECClassId()))
        return ECInstanceKey();

    if (ECSqlStatus::Success != statement.BindId (2, targetKey.GetECInstanceId()))
        return ECInstanceKey();

    if (BE_SQLITE_ROW != statement.Step())
        return ECInstanceKey();

    return ECInstanceKey (statement.GetValueInt64 (0), statement.GetValueId<ECInstanceId> (1));
    }

END_DGNDB_UNIT_TESTS_NAMESPACE