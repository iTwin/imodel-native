/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <BeRapidJson/BeJsValue.h>
USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlPragmasTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, explain_query){
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("explain_query.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"x(
        PRAGMA explain_query(
            "SELECT * FROM meta.ECClassDef WHERE Name='Element'"
        ))x"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(stmt.GetValueInt(0), 3); // id
    ASSERT_EQ(stmt.GetValueInt(1), 0); // parent
    ASSERT_EQ(stmt.GetValueInt(2), 0); // notused
    ASSERT_STREQ(stmt.GetValueText(3), "SEARCH main.ec_Class USING INDEX ix_ec_Class_Name (Name=?)"); // detail
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, ecdb_ver){
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ecdb_ver.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA ecdb_ver"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ(stmt.GetValueText(0), m_ecdb.GetECDbProfileVersion().ToString().c_str());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, experimental_features_enabled){
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("experimental_features_enabled.ecdb"));

    if ("experimental_features_enabled is false by default") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA experimental_features_enabled"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(stmt.GetValueBoolean(0), false);
    }

    if ("set experimental_features_enabled=true") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA experimental_features_enabled=true"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(stmt.GetValueBoolean(0), true);
    }

    if ("set experimental_features_enabled=false") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA experimental_features_enabled=false"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(stmt.GetValueBoolean(0), false);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, view_generator_must_use_escaped_class_name_when_checking_disqualifed_check) {
   ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bug.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='Generic' alias='g' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'> "
        "  <ECEntityClass typeName='Base' modifier='None'>"
        "    <ECCustomAttributes>"
        "      <ClassMap xmlns='ECDbMap.02.00'>"
        "        <MapStrategy>TablePerHierarchy</MapStrategy>"
        "      </ClassMap>"
        "    </ECCustomAttributes>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Group' modifier='None'>"
        "    <BaseClass>Base</BaseClass>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Other' modifier='None'>"
        "    <BaseClass>Base</BaseClass>"
        "  </ECEntityClass>"
        "</ECSchema>")));

    auto groupClassId = m_ecdb.Schemas().GetClassId("Generic", "Group");
    auto otherClassId = m_ecdb.Schemas().GetClassId("Generic", "Other");
    if ("unescaped GROUP keyword as class name should fail the statement") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT 1 FROM Generic.Group"));
    }

    if ("unescaped GROUP keyword as class name should fail the pragma") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "PRAGMA disqualify_type_index FOR Generic:Group"));
    }

    if ("escaped GROUP keyword as class name should prepare query fine") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM Generic:[Group]"));
        ASSERT_STREQ(SqlPrintfString("SELECT 1 FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[g_Base] WHERE [g_Base].ECClassId=%s) [Group]", groupClassId.ToString().c_str()), stmt.GetNativeSql());
    }

    if ("escaped GROUP keyword as class name should set disqualify_type_index correctly") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA disqualify_type_index=TRUE FOR Generic:[Group]"));
    }

    if ("escaped GROUP keyword as class name should prepare query fine and should not disqualified (+) at view generator as there is no other class accessed in query") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM Generic:[Group]"));
        ASSERT_STREQ(SqlPrintfString("SELECT 1 FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[g_Base] WHERE [g_Base].ECClassId=%s) [Group]", groupClassId.ToString().c_str()), stmt.GetNativeSql());
    }

    if ("escaped GROUP keyword as class name should prepare query fine and should not disqualified (+) at view generator as there are more then on classes in query") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM Generic:[Group] a, Generic:[Other] b WHERE a.ECInstanceid = b.ECInstanceId"));
        ASSERT_STREQ(SqlPrintfString("SELECT 1 FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[g_Base] WHERE +[g_Base].ECClassId=%s) [a],(SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[g_Base] WHERE [g_Base].ECClassId=%s) [b] WHERE [a].[ECInstanceId]=[b].[ECInstanceId]", groupClassId.ToString().c_str(), otherClassId.ToString().c_str()), stmt.GetNativeSql());
    }
}
END_ECDBUNITTESTS_NAMESPACE