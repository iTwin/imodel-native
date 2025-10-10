/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlAliasResolutionTestFixture : ECDbTestFixture {};

#define THIS_TEST_NAME ::testing::Test::GetTestNameA()
#define ASSERT_ECSQL_SUCCESS(QUERY) { ECSqlStatement stmt; ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, QUERY)); }
#define EXPECT_ECSQL_SUCCESS(QUERY) { ECSqlStatement stmt; EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, QUERY)); }
#define ASSERT_ECSQL_INVALID(QUERY) { ECSqlStatement stmt; ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, QUERY)); }
#define ASSERT_ECSQL_ERROR(QUERY) { ECSqlStatement stmt; ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(m_ecdb, QUERY)); }
#define ASSERT_ECDB_SETUP ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb(SqlPrintfString("%s.ecdb", THIS_TEST_NAME).GetUtf8CP()));
#define ASSERT_OPEN_TEST_BIM ASSERT_EQ(DbResult::BE_SQLITE_OK, OpenECDbTestDataFile("test.bim"));
#define ASSERT_ECSQL(QUERY) ASSERT_ECSQL_SUCCESS(QUERY)
#define EXPECT_ECSQL(QUERY) EXPECT_ECSQL_SUCCESS(QUERY)
#define INVALID(QUERY) ASSERT_ECSQL_INVALID(QUERY)

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlAliasResolutionTestFixture, SubqueryAlias) {
    ASSERT_OPEN_TEST_BIM;
    ASSERT_ECSQL("with recursive cte0 (a,b) as ( select 100,200) select * from (select * from cte0 c0 where c0.a=100 and c0.b=200)");
    INVALID("SELECT * FROM (SELECT N FROM meta.ECClassDef)");
    ASSERT_ECSQL("SELECT 1 FROM meta.ECClassDef, (SELECT ECInstanceId FROM meta.ECPropertyDef) S WHERE ECClassDef.ECInstanceId=S.ECInstanceId");
    ASSERT_ECSQL("SELECT e.* FROM Bis.Element e");
    ASSERT_ECSQL("SELECT e.* FROM Bis.Element e JOIN Bis.Model m ON m.ECInstanceId = e.Model.Id");
    ASSERT_ECSQL("SELECT e.* FROM Bis.Element e WHERE e.Model.Id IN (SELECT m.ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id)");
    ASSERT_ECSQL("SELECT (SELECT ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id) FROM Bis.Element e");
    ASSERT_ECSQL("SELECT (SELECT ECInstanceId FROM (SELECT m.ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id)) FROM Bis.Element e");
    ASSERT_ECSQL("SELECT c.ECInstanceId, m.ECInstanceId FROM bis.SpatialCategory c JOIN bis.Model m ON m.ECInstanceId IN (SELECT e.Model.Id FROM bis.GeometricElement3d e WHERE e.Category.Id = c.ECInstanceId)");
    ASSERT_ECSQL("SELECT (SELECT ECInstanceId FROM (SELECT C.ECInstanceId FROM Meta.ECClassDef C WHERE C.ECInstanceId = E.ECClassId)) FROM Bis.Element E")
    ASSERT_ECSQL("SELECT (SELECT ECInstanceId FROM (SELECT ECInstanceId FROM Meta.ECClassDef WHERE ECInstanceId = E.ECClassId)) FROM Bis.Element E")
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlAliasResolutionTestFixture, ColumnAliasesWithClauses) {
    ASSERT_OPEN_TEST_BIM;
    EXPECT_ECSQL("select * from (select 'Test' as literalAlias, COUNT(*) from meta.ECClassDef group by literalAlias)")

    EXPECT_ECSQL("select ECInstanceId as idAlias from meta.ECClassDef where idAlias < 30")
    EXPECT_ECSQL("select 'Test' as literalAlias from meta.ECClassDef where literalAlias < 30")

    EXPECT_ECSQL("select ECInstanceId as idAlias from meta.ECClassDef group by idAlias")
    EXPECT_ECSQL("select 'Test' as literalAlias from meta.ECClassDef group by literalAlias")

    EXPECT_ECSQL("select ECInstanceId as idAlias from meta.ECClassDef order by idAlias desc")
    EXPECT_ECSQL("select 'Test' as literalAlias from meta.ECClassDef order by literalAlias desc")

    EXPECT_ECSQL("select ECInstanceId as idAlias from meta.ECClassDef order by idAlias desc")
    EXPECT_ECSQL("select 'Test' as literalAlias from meta.ECClassDef order by literalAlias desc")

    EXPECT_ECSQL("select ECInstanceId as idAlias from meta.ECClassDef order by idAlias desc")
    EXPECT_ECSQL("select 'Test' as literalAlias from meta.ECClassDef order by literalAlias desc")

    EXPECT_ECSQL("select ECInstanceId as idAlias from meta.ECClassDef group by idAlias having idAlias < 30")
    EXPECT_ECSQL(R"sql(select ECInstanceId as idAlias, Name as nameAlias from meta.ECClassDef group by idAlias having nameAlias like 'Db%')sql")

    EXPECT_ECSQL("select ECInstanceId as idAlias, Name as nameAlias from meta.ECClassDef where idAlias < 30 group by nameAlias")
    EXPECT_ECSQL("select ECInstanceId as idAlias, 'Test' as literalAlias from meta.ECClassDef where idAlias < 30 order by literalAlias desc")

    EXPECT_ECSQL("select ECInstanceId, Name from (select ECInstanceId as idAlias, Name as nameAlias from meta.ECClassDef where idAlias < 30) group by nameAlias order by idAlias desc")

    EXPECT_ECSQL("select * from (select * from (select ECInstanceId as test from meta.ECClassDef)) group by test having test < 30")

    EXPECT_ECSQL("select count(*) cnt from meta.ECClassDef group by ECClassId having cnt > 10")
    EXPECT_ECSQL("select (select 1) a from meta.ECClassDef where a <> 1")
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlAliasResolutionTestFixture, Cte_derived_property_expanding_asterisk) {
    ASSERT_OPEN_TEST_BIM;
    ASSERT_ECSQL("WITH e AS (SELECT f.* FROM Bis.Element f) SELECT e.Model.Id FROM e");
    ASSERT_ECSQL("WITH e(a,b) AS (SELECT f.* FROM (select 100, 200) f) SELECT a, b FROM e");
    ASSERT_ECSQL("WITH e(a,b) AS (SELECT f.* FROM (select 100, 200) f) SELECT e.a, e.b FROM e");
    ASSERT_ECSQL("WITH e AS (SELECT f.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id) FROM e");
    ASSERT_ECSQL("WITH e AS (SELECT f.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM (SELECT m.ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id)) FROM e");
    INVALID("WITH e AS (SELECT f.Model.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id) FROM e");
    INVALID("WITH e AS (SELECT Model.* FROM Bis.Element f) SELECT (SELECT ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id) FROM e");
    INVALID("WITH e AS (SELECT f.Model.* FROM Bis.Element) SELECT (SELECT ECInstanceId FROM Bis.Model m WHERE m.ECInstanceId = e.Model.Id) FROM e");
}


END_ECDBUNITTESTS_NAMESPACE