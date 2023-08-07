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
TEST_F(ECSqlPragmasTestFixture, parse_tree){
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("parse_tree.ecdb"));

    if ("disable by default due to marked as experimental feature") {
        ECIssueListener issueListener(m_ecdb);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus(BE_SQLITE_ERROR), stmt.Prepare(m_ecdb, R"x(
            PRAGMA parse_tree(
                "SELECT * FROM meta.ECClassDef WHERE Name='Element'"
            ))x"));
        auto issue = issueListener.GetIssue();
        ASSERT_EQ(issue.has_value(), true);
        ASSERT_STREQ(issue.message.c_str(), "'PRAGMA parse_tree' is experimental feature and disabled by default.");
    }

    auto beautify = [](Utf8CP str) -> Utf8String {
        BeJsDocument json;
        json.Parse(str);
        return json.Stringify(StringifyFormat::Indented);
    };

    if ("enable using ECSQLOPTIONS") {
        ECIssueListener issueListener(m_ecdb);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"x(
            PRAGMA parse_tree("SELECT * FROM meta.ECClassDef WHERE Name='Element'") ECSQLOPTIONS enable_experimental_features
            )x"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        auto expectedJson = R"*(
            {
            "id": "SelectStatementExp",
            "select": {
                "id": "SingleSelectStatementExp",
                "selection": [
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "ECInstanceId"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "ECClassId"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "Schema"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "Name"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "DisplayLabel"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "Description"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "Type"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "Modifier"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "CustomAttributeContainerType"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "RelationshipStrength"
                    }
                },
                {
                    "id": "DerivedPropertyExp",
                    "exp": {
                    "id": "PropertyNameExp",
                    "path": "RelationshipStrengthDirection"
                    }
                }
                ],
                "from": [
                {
                    "id": "ClassNameExp",
                    "tableSpace": "",
                    "schemaName": "ECDbMeta",
                    "className": "ECClassDef"
                }
                ],
                "where": {
                "id": "BinaryBooleanExp",
                "op": "=",
                "lhs": {
                    "id": "PropertyNameExp",
                    "path": "Name"
                },
                "rhs": {
                    "id": "LiteralValueExp",
                    "kind": "STRING",
                    "value": "Element"
                }
                }
            }
        })*";
        ASSERT_STREQ(beautify(stmt.GetValueText(0)).c_str(), beautify(expectedJson).c_str());
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
        "</ECSchema>")));

    auto groupClassId = m_ecdb.Schemas().GetClassId("Generic", "Group");
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

    if ("escaped GROUP keyword as class name should prepare query fine and should be disqualified (+) at view generator") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM Generic:[Group]"));
        ASSERT_STREQ(SqlPrintfString("SELECT 1 FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[g_Base] WHERE +[g_Base].ECClassId=%s) [Group]", groupClassId.ToString().c_str()), stmt.GetNativeSql());
    }
}
END_ECDBUNITTESTS_NAMESPACE