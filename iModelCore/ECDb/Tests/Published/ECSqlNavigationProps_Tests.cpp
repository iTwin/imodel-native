/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlNavigationProps_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlNavigationPropertyTestFixture : ECDbTestFixture
    {
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, InsertOnOneEnd_ForeignKeyMapping)
    {
/*    ECDbR ecdb = SetupECDb("ecsqlnavprops.ecdb", BeFileName(L"ECSql_NavigationProperties.01.00.ecschema.xml"), 3, ECDb::OpenParams(Db::OpenMode::ReadWrite));

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT GetECClassId(), ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueInt64(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }
    */
    {
    /*ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO np.DgnElement(Model,Code) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "TestCode-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());*/
    }
    
    }

END_ECDBUNITTESTS_NAMESPACE

