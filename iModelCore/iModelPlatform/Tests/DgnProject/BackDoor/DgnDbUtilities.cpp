/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include "PublicAPI/BackDoor/DgnProject/DgnDbUtilities.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
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

    if (ECSqlStatus::Success != statement.BindId(1, targetKey.GetClassId()))
        return ECInstanceKey();

    if (ECSqlStatus::Success != statement.BindId(2, targetKey.GetInstanceId()))
        return ECInstanceKey();

    if (BE_SQLITE_ROW != statement.Step())
        return ECInstanceKey();

    return ECInstanceKey(statement.GetValueId<ECClassId>(0), statement.GetValueId<ECInstanceId>(1));
    }

END_DGNDB_UNIT_TESTS_NAMESPACE