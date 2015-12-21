/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/SqliteJSonSupport.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

TEST(BeSQLite, JSonSupport)
    {
    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);
    BeSQLiteLib::Initialize(tmpDir);
    Db db;  
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(":memory:"));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("CREATE TABLE JsonTest(Id INTEGER PRIMARY KEY, JsonColumn JSON)")) << "JSON column type not supported";
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql(
        "INSERT INTO JsonTest VALUES (1, json_object('ex','[52,3.14159]'))"
        )) << "JSON column type not supported";

    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql(
        "INSERT INTO JsonTest VALUES (2, json_object('ex',json_array(52,3.14159)))"
        )) << "JSON column type not supported";

    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql(
        "INSERT INTO JsonTest VALUES (3, json_array(1,2,'3',4))"
        )) << "JSON column type not supported";

    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql(
        "INSERT INTO JsonTest VALUES (4, json_array('[1,2]'))"
        )) << "JSON column type not supported";

    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql(
        "INSERT INTO JsonTest VALUES (5, json_array(1,null,'3',json('[4,5]'),json('{\"six\":7.7}')))"
        )) << "JSON column type not supported";

    }
END_ECDBUNITTESTS_NAMESPACE
