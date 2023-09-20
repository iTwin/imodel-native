/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlAliasResolutionTestFixture : ECDbTestFixture {};

#define THIS_TEST_NAME ::testing::Test::GetTestNameA()
#define ASSERT_ECSQL_SUCCESS(QUERY) { ECSqlStatement stmt; ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, QUERY)); }
#define ASSERT_ECSQL_INVALID(QUERY) { ECSqlStatement stmt; ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, QUERY)); }
#define ASSERT_ECSQL_ERROR(QUERY) { ECSqlStatement stmt; ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(m_ecdb, QUERY)); }
#define ASSERT_ECDB_SETUP ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb(SqlPrintfString("%s.ecdb", THIS_TEST_NAME).GetUtf8CP()));
#define ASSERT_OPEN_TEST_BIM ASSERT_EQ(DbResult::BE_SQLITE_OK, OpenECDbTestDataFile("test.bim"));
#define ECSQL(QUERY) ASSERT_ECSQL_SUCCESS(QUERY)
#define INVALID(QUERY) ASSERT_ECSQL_INVALID(QUERY)

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlAliasResolutionTestFixture, SubqueryAlias) {
    ASSERT_OPEN_TEST_BIM;
    ECSQL("with recursive cte0 (a,b) as ( select 100,200) select * from (select * from cte0 c0 where c0.a=100 and c0.b=200)");
    INVALID("SELECT * FROM (SELECT N FROM meta.ECClassDef)");
    ECSQL("SELECT 1 FROM meta.ECClassDef, (SELECT ECInstanceId FROM meta.ECPropertyDef) S WHERE ECClassDef.ECInstanceId=S.ECInstanceId");
    ECSQL("SELECT e.* FROM Bis.Element e");
    ECSQL("SELECT e.* FROM Bis.Element e JOIN Bis.Model m ON m.ECInstanceId = e.Model.Id");
    ECSQL("SELECT e.* FROM Bis.Element e WHERE e.Model.Id IN (SELECT m.ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id)");
    ECSQL("SELECT (SELECT ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id) FROM Bis.Element e");
    ECSQL("SELECT (SELECT ECInstanceId FROM (SELECT m.ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id)) FROM Bis.Element e");
    ECSQL("SELECT c.ECInstanceId, m.ECInstanceId FROM bis.SpatialCategory c JOIN bis.Model m ON m.ECInstanceId IN (SELECT e.Model.Id FROM bis.GeometricElement3d e WHERE e.Category.Id = c.ECInstanceId)");
    ECSQL("SELECT (SELECT ECInstanceId FROM (SELECT C.ECInstanceId FROM Meta.ECClassDef C WHERE C.ECInstanceId = E.ECClassId)) FROM Bis.Element E")
    ECSQL("SELECT (SELECT ECInstanceId FROM (SELECT ECInstanceId FROM Meta.ECClassDef WHERE ECInstanceId = E.ECClassId)) FROM Bis.Element E")
}


END_ECDBUNITTESTS_NAMESPACE