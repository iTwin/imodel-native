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
    ASSERT_EQ(stmt.GetValueInt(2), 62); // notused
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
TEST_F(ECSqlPragmasTestFixture, ecsql_ver)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ecsql_ver.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA ecsql_ver"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ(stmt.GetValueText(0), ECDb::GetECSqlVersion().ToString().c_str());
    }

//---------------------------------------------------------------------------------------
// A pragma must execute its underlying logic against the data-source connection (the read-only
// connection used to execute the statement), not the schema/parse connection. This mirrors regular
// ECSQL, which prepares/steps against the data-source connection. In concurrent query the parse
// connection is a shared schema-source connection that does NOT carry the executing connection's
// synced attached databases, so running a pragma there would observe the wrong connection's state.
//
// PRAGMA db_list reports the attached databases of whichever connection runs it, so it is a direct,
// observable proxy for which connection executed the pragma.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, pragma_runs_on_data_source_connection)
    {
    // A standalone ECDb file that will be attached as a table space to the data-source connection only.
    BeFileName attachedECDbPath = ECDbTestFixture::BuildECDbPath("pragmaDataSourceAttached.ecdb");
    if (attachedECDbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, attachedECDbPath.BeDeleteFile());

    {
    ECDb attached;
    ASSERT_EQ(BE_SQLITE_OK, attached.CreateNewDb(attachedECDbPath));
    ASSERT_EQ(BE_SQLITE_OK, attached.SaveChanges());
    attached.CloseDb();
    }

    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("pragmaDataSource.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    BeFileName mainFile(m_ecdb.GetDbFileName(), true);

    // Two independent read-only connections to the SAME ECDb file. 'parseConn' plays the role of the
    // shared schema-source connection (no attachments); 'dataSourceConn' plays the role of a worker
    // connection that has an attached table space synced to it.
    ECDb parseConn;
    ASSERT_EQ(BE_SQLITE_OK, parseConn.OpenBeSQLiteDb(mainFile, Db::OpenParams(Db::OpenMode::Readonly)));

    ECDb dataSourceConn;
    ASSERT_EQ(BE_SQLITE_OK, dataSourceConn.OpenBeSQLiteDb(mainFile, Db::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(BE_SQLITE_OK, dataSourceConn.AttachDb(attachedECDbPath.GetNameUtf8().c_str(), "attached"));

    // PRAGMA db_list columns: sno(0), alias(1), fileName(2), profile(3).
    auto listsAttachedAlias = [](ECSqlStatement& stmt)
        {
        bool found = false;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            if (Utf8String(stmt.GetValueText(1)).EqualsIAscii("attached"))
                found = true;
            }
        return found;
        };

    // Multi-threaded prepare: parse against parseConn, execute against dataSourceConn. The pragma must
    // run on the data-source connection and therefore observe ITS attached 'attached' table space.
    ECSqlStatement multiConnStmt;
    ASSERT_EQ(ECSqlStatus::Success, multiConnStmt.Prepare(parseConn.Schemas(), dataSourceConn, "PRAGMA db_list"));
    EXPECT_EQ(static_cast<Db const*>(&dataSourceConn), multiConnStmt.GetDataSourceDb())
        << "pragma should be associated with the data-source connection";
    EXPECT_TRUE(listsAttachedAlias(multiConnStmt))
        << "PRAGMA db_list must run on the data-source connection, which has the 'attached' table space";

    // Control: the parse connection has no attachment, so a single-connection db_list omits it. This is
    // what the multi-connection statement would have reported if the pragma had (incorrectly) run on the
    // parse connection.
    ECSqlStatement parseConnStmt;
    ASSERT_EQ(ECSqlStatus::Success, parseConnStmt.Prepare(parseConn, "PRAGMA db_list"));
    EXPECT_FALSE(listsAttachedAlias(parseConnStmt))
        << "the parse/schema-source connection has no 'attached' table space";

    // Sanity: a single-connection db_list on the data-source connection lists it too.
    ECSqlStatement dataSourceStmt;
    ASSERT_EQ(ECSqlStatus::Success, dataSourceStmt.Prepare(dataSourceConn, "PRAGMA db_list"));
    EXPECT_TRUE(listsAttachedAlias(dataSourceStmt));
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
        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus(BE_SQLITE_ERROR), stmt.Prepare(m_ecdb, R"x(
            PRAGMA parse_tree(
                "SELECT * FROM meta.ECClassDef WHERE Name='Element'"
            ))x"));
        ASSERT_FALSE(issueListener.IsEmpty());
        ASSERT_STREQ(issueListener.GetLastMessage().c_str(), "'PRAGMA parse_tree' is experimental feature and disabled by default.");
    }

    auto beautify = [](Utf8CP str) -> Utf8String {
        BeJsDocument json;
        json.Parse(str);
        return json.Stringify(StringifyFormat::Indented);
    };

    if ("enable using ECSQLOPTIONS") {
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

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, purge_orphan_relationships)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("purge_orphan_relationships.ecdb"));

    // Test syntax of PRAGMA purge_orphan_relationships
    for (const auto& [testCaseNumber, ecsql, expectedOutput] : std::vector<std::tuple<int, Utf8CP, ECSqlStatus::Status>>
        {
            { 1, "PRAGMA purge_orphan_relationships", ECSqlStatus::Status::SQLiteError },
            { 2, "PRAGMA purge_orphan_relationships()", ECSqlStatus::Status::InvalidECSql },
            { 3, "PRAGMA purge_orphan_relationships=", ECSqlStatus::Status::InvalidECSql },
            { 4, "PRAGMA purge_orphan_relationships=TRUE", ECSqlStatus::Status::SQLiteError },
            { 5, "PRAGMA purge_orphan_relationships=false", ECSqlStatus::Status::SQLiteError },
            { 6, "PRAGMA purge_orphan_relationships=0", ECSqlStatus::Status::SQLiteError },
            { 7, "PRAGMA purge_orphan_relationships=1", ECSqlStatus::Status::SQLiteError },

            { 8, "PRAGMA purge_orphan_relationships options enable_experimental_features", ECSqlStatus::Status::Success },
            { 9, "PRAGMA purge_orphan_relationships() options enable_experimental_features", ECSqlStatus::Status::InvalidECSql },
            {10, "PRAGMA purge_orphan_relationships= options enable_experimental_features", ECSqlStatus::Status::InvalidECSql },
            {11, "PRAGMA purge_orphan_relationships=true options enable_experimental_features", ECSqlStatus::Status::SQLiteError },
            {12, "PRAGMA purge_orphan_relationships=FALSE options enable_experimental_features", ECSqlStatus::Status::SQLiteError },
            {13, "PRAGMA purge_orphan_relationships=1 options enable_experimental_features", ECSqlStatus::Status::SQLiteError },
            {14, "PRAGMA purge_orphan_relationships=0 options enable_experimental_features", ECSqlStatus::Status::SQLiteError },
        })
        {
        ECSqlStatement stmt;
        ASSERT_EQ(expectedOutput, stmt.Prepare(m_ecdb, ecsql).Get()) << "Test case " << testCaseNumber << " failed";

        if (expectedOutput == ECSqlStatus::Status::Success)
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Test case " << testCaseNumber << " failed";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, PurgeOrphanLinkTableRelationships)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("PurgeOrphanLinkTableRelationships.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="rst" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
	        <ECSchemaReference name="ECDbMap" version="2.0.0" alias="ecdbmap" />

            <ECEntityClass typeName="ClassA" modifier="None">
                <ECProperty propertyName="ClassAProp" typeName="string" displayLabel="ClassAProp"/>
            </ECEntityClass>
            <ECEntityClass typeName="ClassB" modifier="None">
                <ECProperty propertyName="ClassBProp" typeName="string" displayLabel="ClassBProp"/>
            </ECEntityClass>

            <ECRelationshipClass typeName="RelA_A" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassA"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelB_B" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassB"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelA_B" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassB"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelB_A" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassA"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));

    auto insertEntry = [&](Utf8CP className, Utf8CP propName)
        {
        ECSqlStatement stmt;
        ECInstanceKey outKey;
        if (ECSqlStatus::Success == stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO TestSchema.%s(%sProp) VALUES('%s')", className, className, propName).c_str()))
            stmt.Step(outKey);
        return outKey;
        };

    auto insertRelationship = [&](Utf8CP className, const ECInstanceKey& sourceKey, const ECInstanceKey& targetKey)
        {
        ECSqlStatement stmt;
        ECInstanceKey outKey;
        if (ECSqlStatus::Success == stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO TestSchema.%s(SourceECInstanceId, TargetECInstanceId) VALUES(%s,%s)", className, sourceKey.GetInstanceId().ToString().c_str(), targetKey.GetInstanceId().ToString().c_str()).c_str()))
            stmt.Step(outKey);
        return outKey;
        };

    // Insert class instances
    const auto classA1 = insertEntry("ClassA", "A1");
    const auto classA2 = insertEntry("ClassA", "A2");
    const auto classA3 = insertEntry("ClassA", "A3");
    const auto classB1 = insertEntry("ClassB", "B1");
    const auto classB2 = insertEntry("ClassB", "B2");
    const auto classB3 = insertEntry("ClassB", "B3");

    // Insert link table relationships
    const auto relA_A1 = insertRelationship("RelA_A", classA1, classA2);
    const auto relA_A2 = insertRelationship("RelA_A", classA1, classA3);
    const auto relB_B1 = insertRelationship("RelB_B", classB1, classB2);
    const auto relB_B2 = insertRelationship("RelB_B", classB2, classB3);

    const auto relA_B1 = insertRelationship("RelA_B", classA1, classB1);
    const auto relA_B2 = insertRelationship("RelA_B", classA2, classB2);
    const auto relA_B3 = insertRelationship("RelA_B", classA1, classB2);
    const auto relA_B4 = insertRelationship("RelA_B", classA2, classB3);

    const auto relB_A1 = insertRelationship("RelB_A", classB2, classA2);
    const auto relB_A2 = insertRelationship("RelB_A", classB1, classA1);
    const auto relB_A3 = insertRelationship("RelB_A", classB2, classA1);
    const auto relB_A4 = insertRelationship("RelB_A", classB3, classA3);

    // Delete instance A1 from ClassA and B2 from ClassB, thus creating 4 orphan relationship rows in all link table relationships.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("DELETE FROM TestSchema.ClassA WHERE ECInstanceId=%s", classA1.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("DELETE FROM TestSchema.ClassB WHERE ECInstanceId=%s", classB2.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    // Run the integrity check command to verify that 4 orphan relationship rows exist
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "pragma integrity_check(check_linktable_fk_ids) options enable_experimental_features"));

    for (const auto& [testCaseNumber, id, relationshipName, property, keyId, primaryClass] : std::vector<std::tuple<const int, const ECInstanceKey&, Utf8CP, Utf8CP, const ECInstanceKey&, Utf8CP>>
        {
            { 1, relA_A1, "TestSchema:RelA_A", "SourceECInstanceId", classA1, "TestSchema:ClassA" },  // A1 (missing) -> A2
            { 2, relA_A2, "TestSchema:RelA_A", "SourceECInstanceId", classA1, "TestSchema:ClassA" },  // A1 (missing) -> A3

            { 3, relA_B1, "TestSchema:RelA_B", "SourceECInstanceId", classA1, "TestSchema:ClassA" },  // A1 (missing) -> B1
            { 4, relA_B3, "TestSchema:RelA_B", "SourceECInstanceId", classA1, "TestSchema:ClassA" },  // A1 (missing) -> B2 (missing)
            { 5, relA_B2, "TestSchema:RelA_B", "TargetECInstanceId", classB2, "TestSchema:ClassB" },  // A2 -> B2 (missing)
            { 6, relA_B3, "TestSchema:RelA_B", "TargetECInstanceId", classB2, "TestSchema:ClassB" },  // A1 (missing) -> B1 (missing)

            { 7, relB_A1, "TestSchema:RelB_A", "SourceECInstanceId", classB2, "TestSchema:ClassB" },  // B2 (missing) -> A2
            { 8, relB_A3, "TestSchema:RelB_A", "SourceECInstanceId", classB2, "TestSchema:ClassB" },  // B2 (missing) -> A1 (missing)
            { 9, relB_A2, "TestSchema:RelB_A", "TargetECInstanceId", classA1, "TestSchema:ClassA" },  // B1 -> A1 (missing)
            {10, relB_A3, "TestSchema:RelB_A", "TargetECInstanceId", classA1, "TestSchema:ClassA" },  // B2 (missing) -> A1 (missing)

            {11, relB_B2, "TestSchema:RelB_B", "SourceECInstanceId", classB2, "TestSchema:ClassB" },  // B2 (missing) -> B3
            {12, relB_B1, "TestSchema:RelB_B", "TargetECInstanceId", classB2, "TestSchema:ClassB" },  // B1 -> B2 (missing)
        })
        {
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step())                                           << "Test case " << testCaseNumber << " failed";
        EXPECT_STREQ(stmt.GetValueText(1), id.GetInstanceId().ToHexStr().c_str())       << "Test case " << testCaseNumber << " failed";
        EXPECT_STREQ(stmt.GetValueText(2), relationshipName)                            << "Test case " << testCaseNumber << " failed";
        EXPECT_STREQ(stmt.GetValueText(3), property)                                    << "Test case " << testCaseNumber << " failed";
        EXPECT_STREQ(stmt.GetValueText(4), keyId.GetInstanceId().ToHexStr().c_str())    << "Test case " << testCaseNumber << " failed";
        EXPECT_STREQ(stmt.GetValueText(5), primaryClass)                                << "Test case " << testCaseNumber << " failed";
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    // Run the purge_orphan_relationships to remove the orphan relationship rows
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "pragma purge_orphan_relationships options enable_experimental_features"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    // Run the integrity check command to verify that all orphan relationship rows have been deleted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "pragma integrity_check(check_linktable_fk_ids) options enable_experimental_features"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, sqlite_sql)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("sqlite_sql.ecdb"));

    // Valid input cases
    for (const auto& [testCaseName, ecsqlStmt] : std::vector<std::pair<std::string, std::string>> {
        std::make_pair("Valid ECSQL", "SELECT * FROM meta.ECClassDef WHERE Name='Element'"),
        std::make_pair("Parameterized ECSQL", "SELECT * FROM meta.ECClassDef WHERE Name=?"),
        std::make_pair("Complex ECSQL with joins", "SELECT a.Name, b.Name FROM meta.ECClassDef a JOIN meta.ECClassDef b ON a.Name=b.Name"),
        std::make_pair("CTE", "WITH el AS (SELECT ECInstanceId, ECClassId FROM meta.ECClassDef) SELECT * FROM el"),
    })
        {
        // Prepare the ECSql directly to compare the Pragma results
        ECSqlStatement expectedStmt;
        ASSERT_EQ(ECSqlStatus::Success, expectedStmt.Prepare(m_ecdb, ecsqlStmt.c_str()));

        ECSqlStatement pragmaStmt;
        ASSERT_EQ(ECSqlStatus::Success, pragmaStmt.Prepare(m_ecdb, SqlPrintfString("PRAGMA sqlite_sql(\"%s\")", ecsqlStmt.c_str()))) << "Test case '" << testCaseName << "' failed to prepare";
        ASSERT_EQ(BE_SQLITE_ROW, pragmaStmt.Step()) << "Test case '" << testCaseName << "' failed to execute";

        ASSERT_STREQ(pragmaStmt.GetValueText(0), expectedStmt.GetNativeSql()) << "Test case '" << testCaseName << "' failed to match expected output";
        }

    // Invalid input cases:
    for (const auto& [testCaseName, ecsqlStmt] : std::vector<std::pair<std::string, std::string>> {
        std::make_pair("Empty String", "\"\""),
        std::make_pair("Non-String input", "12345"),
        std::make_pair("Invalid ECSQL input", "\"SELECT * FROM non_existent_table\""),
        std::make_pair("Malformed ECSQL", "\"SELECT FROM WHERE\""),
    })
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Status::SQLiteError, stmt.Prepare(m_ecdb, SqlPrintfString("PRAGMA sqlite_sql(%s)", ecsqlStmt.c_str()))) << "Test case '" << testCaseName << "' was expected to fail";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_view_returns_blob) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("schema_view.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Foo' modifier='None'>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    { // basic pragma returns one row with expected columns
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA schema_view"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        // Column 0: format (string)
        ASSERT_STREQ("binary", stmt.GetValueText(0));
        // Column 1: formatVersion (integer)
        ASSERT_EQ(1, stmt.GetValueInt(1));
        // Column 2: data (base64-encoded binary, stored as string with "encoding=base64;" prefix)
        Utf8CP dataText = stmt.GetValueText(2);
        ASSERT_NE(nullptr, dataText);
        static constexpr Utf8CP base64Prefix = "encoding=base64;";
        static constexpr size_t base64PrefixLen = 16;
        ASSERT_EQ(0, strncmp(dataText, base64Prefix, base64PrefixLen));
        ByteStream decoded;
        Utf8CP b64 = dataText + base64PrefixLen;
        Base64Utilities::Decode(decoded, b64, strlen(b64));
        ASSERT_GT((int)decoded.GetSize(), 9); // at least header size: magic(4) + version(1) + stringTableOffset(4)

        // Verify magic bytes "CSCH" = 0x43534348
        auto bytes = decoded.GetData();
        uint32_t magic = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
        ASSERT_EQ(0x43534348u, magic) << "Expected CSCH magic";
        // Verify format version in blob header matches
        ASSERT_EQ(1, bytes[4]);

        // Column 3: schemaToken (string, non-empty SHA hash)
        Utf8CP token = stmt.GetValueText(3);
        ASSERT_NE(nullptr, token);
        ASSERT_GT(strlen(token), 0u);

        // Should be exactly one row
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_view_explicit_version) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("schema_view_ver.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Bar' modifier='None'>"
        "    <ECProperty propertyName='Value' typeName='int' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    { // explicit version 1 should succeed
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA schema_view(1)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(1)); // formatVersion column
        Utf8CP dataText = stmt.GetValueText(2);
        ASSERT_NE(nullptr, dataText);
        static constexpr Utf8CP base64Prefix = "encoding=base64;";
        static constexpr size_t base64PrefixLen = 16;
        ASSERT_EQ(0, strncmp(dataText, base64Prefix, base64PrefixLen));
        ByteStream decoded;
        Utf8CP b64 = dataText + base64PrefixLen;
        Base64Utilities::Decode(decoded, b64, strlen(b64));
        ASSERT_GT((int)decoded.GetSize(), 9);
        // Verify blob header format version matches requested version
        auto bytes = decoded.GetData();
        ASSERT_EQ(1, bytes[4]);
    }

    { // unsupported high version should fail
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Status::SQLiteError, stmt.Prepare(m_ecdb, "PRAGMA schema_view(99)"));
    }

    { // version 0 should fail
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Status::SQLiteError, stmt.Prepare(m_ecdb, "PRAGMA schema_view(0)"));
    }

    { // negative version should fail to parse
        ECSqlStatement stmt;
        // -1 is not a valid pragma_value (no unary minus in grammar), so this fails at parse
        ASSERT_NE(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA schema_view(-1)"));
    }

    { // string argument should fail
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Status::SQLiteError, stmt.Prepare(m_ecdb, "PRAGMA schema_view('2')"));
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_view_is_readonly) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("schema_view_ro.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Status::SQLiteError, stmt.Prepare(m_ecdb, "PRAGMA schema_view=2"));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_view_token_determinism_and_checksum_match) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("schema_view_token.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Foo' modifier='None'>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    // Get schemaToken from schema_view pragma (two calls to verify determinism)
    Utf8String token1, token2;
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA schema_view"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        token1 = stmt.GetValueText(3);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA schema_view"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        token2 = stmt.GetValueText(3);
    }
    ASSERT_STREQ(token1.c_str(), token2.c_str()) << "schema_view token must be deterministic across calls";

    // schema_view's schemaToken column is the cheap schema-identity hash, so it must match
    // PRAGMA checksum(schema_token) (and NOT PRAGMA checksum(ecdb_schema), which hashes full schema contents).
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA checksum(schema_token)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        Utf8CP schemaTokenVal = stmt.GetValueText(0);
        ASSERT_STREQ(token1.c_str(), schemaTokenVal) << "schema_view schemaToken must match PRAGMA checksum(schema_token)";
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_view_token_changes_after_schema_import) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("schema_view_token_change.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Foo' modifier='None'>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    // Capture token before schema change
    Utf8String tokenBefore;
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA schema_view"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        tokenBefore = stmt.GetValueText(3);
    }

    // Import an updated schema (adds a new property)
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Foo' modifier='None'>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "    <ECProperty propertyName='Description' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    // Token after schema change must differ
    Utf8String tokenAfter;
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA schema_view"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        tokenAfter = stmt.GetValueText(3);
    }
    ASSERT_STRNE(tokenBefore.c_str(), tokenAfter.c_str()) << "schema_view token must change after schema import";

    // PRAGMA checksum(schema_token) must still match the new schema_view token (both are the cheap
    // schema-identity hash; the import bumped TestSchema 1.0.0 -> 1.0.1, so the hash changed).
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA checksum(schema_token)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ(tokenAfter.c_str(), stmt.GetValueText(0)) << "PRAGMA checksum(schema_token) must match schema_view token after import";
    }
}

//---------------------------------------------------------------------------------------
// PRAGMA checksum(schema_token) is the cheap cache-invalidation key: a hash of every schema's name
// and version only. It must be deterministic across repeated calls, change whenever a schema is added
// or its version bumped, and be read-only. This walks a small series of imports / minor-version
// bumps, asking for the token repeatedly along the way.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_token_reflects_schema_changes) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("schema_token.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaOne' alias='s1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Foo' modifier='None'>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    // Reads PRAGMA checksum(schema_token), asserting a single well-formed non-empty string row.
    auto readToken = [&]() -> Utf8String {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA checksum(schema_token)"));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        Utf8CP const t = stmt.GetValueText(0);
        EXPECT_TRUE(t != nullptr && *t != '\0') << "schema_token must be a non-empty string";
        Utf8String const token(t == nullptr ? "" : t);
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << "schema_token must return exactly one row";
        return token;
    };

    // Deterministic: two back-to-back reads with no schema change are identical.
    Utf8String const t0 = readToken();
    ASSERT_STREQ(t0.c_str(), readToken().c_str()) << "schema_token must be deterministic across calls";

    // Adding a new schema changes the token.
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaTwo' alias='s2' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Bar' modifier='None'>"
        "    <ECProperty propertyName='Value' typeName='int' />"
        "  </ECEntityClass>"
        "</ECSchema>")));
    Utf8String const t1 = readToken();
    ASSERT_STRNE(t0.c_str(), t1.c_str()) << "schema_token must change when a schema is added";
    ASSERT_STREQ(t1.c_str(), readToken().c_str()) << "schema_token must be stable when nothing changed";

    // Bumping a schema's minor version changes the token.
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaTwo' alias='s2' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Bar' modifier='None'>"
        "    <ECProperty propertyName='Value' typeName='int' />"
        "    <ECProperty propertyName='Extra' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));
    Utf8String const t2 = readToken();
    ASSERT_STRNE(t1.c_str(), t2.c_str()) << "schema_token must change when a schema's version is bumped";

    // Read-only: schema_token is a checksum key (func-style argument), so there is no valid
    // assignment syntax for it - the grammar's '(val)' and '=val' forms are mutually exclusive,
    // making 'checksum(schema_token)=1' a parse error rather than a reachable write.
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Status::InvalidECSql, stmt.Prepare(m_ecdb, "PRAGMA checksum(schema_token)=1"));
    }
}

//---------------------------------------------------------------------------------------
// Note on fragment testing scope: the schema_view / schema_view_fragment blob is an internal
// contract between this C++ writer and the TypeScript reader, and it flows in one direction only.
// Re-implementing the reader here to peek at the blob's internals would duplicate that contract and
// rot. So the native tests stay rough - they assert the pragma produces a well-formed row and that
// bad input is rejected. The content-level guarantees (no-leakage, exactly which schemas/classes
// land in a view, cross-schema reference resolution) are verified in TypeScript against a live
// iModel, where the blob is consumed by its real reader.
//+---------------+---------------+---------------+---------------+---------------+------

//---------------------------------------------------------------------------------------
// Returns the ec_Schema.Id (== meta.ECSchemaDef.ECInstanceId) of a schema by name, or 0 if absent.
//+---------------+---------------+---------------+---------------+---------------+------
static int64_t GetSchemaIdByName(ECDbR ecdb, Utf8CP name) {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, "SELECT ECInstanceId FROM meta.ECSchemaDef WHERE Name=?"))
        return 0;
    stmt.BindText(1, name, IECSqlBinder::MakeCopy::Yes);
    if (BE_SQLITE_ROW != stmt.Step())
        return 0;
    return stmt.GetValueInt64(0);
}

//---------------------------------------------------------------------------------------
// Rough end-to-end check that the fragment pragma emits a single well-formed binary row for a
// requested subset, and that asking for more schemas carries more data. Exactly which
// schemas/classes land in the blob, and the no-leakage guarantee, are verified in TypeScript (see
// the scope note above) - we do not decode the blob here.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_view_fragment_returns_blob) {
    // SchemaA references SchemaB and derives AClass from b:BBase; SchemaB also owns an unreferenced
    // class. The mix gives the two-vs-one size comparison something to measure.
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("schema_view_fragment.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaB' alias='b' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='BBase' modifier='Abstract' />"
        "  <ECEntityClass typeName='BOther' modifier='None'>"
        "    <ECProperty propertyName='BOtherProp' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaA' alias='a' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECSchemaReference name='SchemaB' version='1.0.0' alias='b' />"
        "  <ECEntityClass typeName='AClass' modifier='None'>"
        "    <BaseClass>b:BBase</BaseClass>"
        "    <ECProperty propertyName='AProp' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    int64_t const idA = GetSchemaIdByName(m_ecdb, "SchemaA");
    int64_t const idB = GetSchemaIdByName(m_ecdb, "SchemaB");
    ASSERT_GT(idA, 0);
    ASSERT_GT(idB, 0);

    // Returns the length of the (base64 text) "data" column for a fragment request, asserting it is
    // a single well-formed binary row along the way.
    auto fragmentDataLen = [&](Utf8StringCR idList) -> size_t {
        Utf8String const sql = Utf8PrintfString("PRAGMA schema_view_fragment('%s')", idList.c_str());
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql.c_str())) << sql.c_str();
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("binary", stmt.GetValueText(0));
        EXPECT_EQ(1, stmt.GetValueInt(1)); // formatVersion
        Utf8CP const data = stmt.GetValueText(2);
        EXPECT_TRUE(data != nullptr && *data != '\0') << "fragment blob must be non-empty";
        size_t const len = data == nullptr ? 0 : strlen(data);
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << "fragment pragma must return exactly one row";
        return len;
    };

    // Asking for both schemas must carry more data as asking for one - a coarse proxy
    // that the request set actually drives what the blob holds. Exact contents are a TS concern.
    size_t const lenA = fragmentDataLen(Utf8PrintfString("%lld", (long long)idA));
    size_t const lenAB = fragmentDataLen(Utf8PrintfString("%lld,%lld", (long long)idA, (long long)idB));
    EXPECT_GT(lenAB, lenA);
}

//---------------------------------------------------------------------------------------
// The optional leading 'v<N>;' token selects the blob format version; omitting it means latest.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_view_fragment_version_prefix) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("schema_view_fragment_ver.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Foo' modifier='None'>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    int64_t const id = GetSchemaIdByName(m_ecdb, "TestSchema");
    ASSERT_GT(id, 0);

    { // explicit 'v1;' prefix succeeds and reports format version 1
        Utf8String const sql = Utf8PrintfString("PRAGMA schema_view_fragment('v1;%lld')", (long long)id);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql.c_str()));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("binary", stmt.GetValueText(0));
        ASSERT_EQ(1, stmt.GetValueInt(1)); // formatVersion
        Utf8CP const data = stmt.GetValueText(2);
        ASSERT_TRUE(data != nullptr && *data != '\0') << "fragment blob must be non-empty";
    }

    { // no prefix defaults to latest, also version 1
        Utf8String const sql = Utf8PrintfString("PRAGMA schema_view_fragment('%lld')", (long long)id);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql.c_str()));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(1));
    }
}

//---------------------------------------------------------------------------------------
// Malformed argument strings fail the pragma rather than emitting a partial or wrong blob.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_view_fragment_invalid_arguments) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("schema_view_fragment_invalid.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "  <ECEntityClass typeName='Foo' modifier='None'>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    int64_t const id = GetSchemaIdByName(m_ecdb, "TestSchema");
    ASSERT_GT(id, 0);
    Utf8String const validId = Utf8PrintfString("%lld", (long long)id);

    auto expectPrepareFails = [&](Utf8CP sql) {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Status::SQLiteError, stmt.Prepare(m_ecdb, sql)) << sql;
    };

    expectPrepareFails("PRAGMA schema_view_fragment('')");        // empty list
    expectPrepareFails("PRAGMA schema_view_fragment('abc')");     // non-integer id
    expectPrepareFails("PRAGMA schema_view_fragment('v1;')");     // version present, empty list
    expectPrepareFails("PRAGMA schema_view_fragment('v;1')");     // malformed version token (no digits)
    expectPrepareFails("PRAGMA schema_view_fragment('vx;1')");    // malformed version token (non-digit)
    expectPrepareFails("PRAGMA schema_view_fragment('v99;1')");   // unsupported version
    expectPrepareFails("PRAGMA schema_view_fragment('999999999')"); // non-existent schema id
    expectPrepareFails("PRAGMA schema_view_fragment(1)");         // integer argument, not a string
    expectPrepareFails("PRAGMA schema_view_fragment=2");          // assignment form is read-only
    expectPrepareFails("PRAGMA schema_view_fragment('0')");      // zero is not a positive id

    { // sanity: the valid id alone prepares and returns a row
        Utf8String const sql = Utf8PrintfString("PRAGMA schema_view_fragment('%s')", validId.c_str());
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql.c_str()));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }

    { // duplicate ids are intentionally de-duplicated, not rejected: same id twice still prepares
        Utf8String const sql = Utf8PrintfString("PRAGMA schema_view_fragment('%s,%s')", validId.c_str(), validId.c_str());
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql.c_str())) << sql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
}

//---------------------------------------------------------------------------------------
// Regression test against the 4.0.0.1 benchmark fixture. Profile 4.0.0.1 predates the
// EC3.2 Units/Formats migration (introduced in 4.0.0.2, ~2018). SchemaViewWriter does
// not query any of the tables/columns added in 4.0.0.2, so the pragma must succeed
// without requiring a profile upgrade. KoQ PersistenceUnit/PresentationUnits will pass
// through in legacy FUS format - that is acceptable degradation; the consumer can
// detect it and decide how to handle it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPragmasTestFixture, schema_view_works_on_old_profile_4001) {
    BeFileName benchmarkFilePath;
    BeTest::GetHost().GetDocumentsRoot(benchmarkFilePath);
    benchmarkFilePath.AppendToPath(L"ECDb").AppendToPath(L"fileformatbenchmark").AppendToPath(L"4001").AppendToPath(L"imodel2.ecdb");
    ASSERT_TRUE(benchmarkFilePath.DoesPathExist())
        << "Profile 4.0.0.1 benchmark fixture missing at " << benchmarkFilePath.GetNameUtf8().c_str();

    BeFileName outDir;
    BeTest::GetHost().GetOutputRoot(outDir);
    if (!outDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(outDir));
    BeFileName workingFilePath(outDir);
    workingFilePath.AppendToPath(L"schema_view_4001.ecdb");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(benchmarkFilePath, workingFilePath));

    ECDb oldFile;
    ASSERT_EQ(BE_SQLITE_OK, oldFile.OpenBeSQLiteDb(workingFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)))
        << "Failed to open 4.0.0.1 file readonly";
    ASSERT_EQ(ProfileVersion(4, 0, 0, 1), oldFile.GetECDbProfileVersion())
        << "Fixture is not at expected profile 4.0.0.1";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(oldFile, "PRAGMA schema_view"))
        << "PRAGMA schema_view must prepare against profile 4.0.0.1";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_STREQ("binary", stmt.GetValueText(0));
    ASSERT_EQ(1, stmt.GetValueInt(1));

    Utf8CP dataText = stmt.GetValueText(2);
    ASSERT_NE(nullptr, dataText);
    static constexpr Utf8CP base64Prefix = "encoding=base64;";
    static constexpr size_t base64PrefixLen = 16;
    ASSERT_EQ(0, strncmp(dataText, base64Prefix, base64PrefixLen));
    ByteStream decoded;
    Utf8CP b64 = dataText + base64PrefixLen;
    Base64Utilities::Decode(decoded, b64, strlen(b64));
    ASSERT_GT((int)decoded.GetSize(), 9);

    auto bytes = decoded.GetData();
    uint32_t magic = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    ASSERT_EQ(0x43534348u, magic) << "Expected CSCH magic";
    ASSERT_EQ(1, bytes[4]) << "Expected format version 1";

    Utf8CP token = stmt.GetValueText(3);
    ASSERT_NE(nullptr, token);
    ASSERT_GT(strlen(token), 0u);

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
}

END_ECDBUNITTESTS_NAMESPACE