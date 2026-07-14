/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../../ECDb/FeatureManager.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Tests for the ec_Feature table and FeatureManager string registry.
//=======================================================================================

struct FeatureTests : ECDbTestFixture
    {
protected:
    DbResult InsertRawFeatureRow(Utf8CP name, Utf8CP compat)
        {
        return m_ecdb.ExecuteSql(Utf8PrintfString("INSERT INTO ec_Feature(Name, Description, Compat) VALUES ('%s', '%s', '%s')", name, "A-Feature-From-The-Future", compat).c_str());
        }
    };

//---------------------------------------------------------------------------------------
// ec_Feature table must be freshly created for every new 4.0.0.6 ECDb file, should have
// correct columns and should be empty
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, FeatureTableSetup)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_table_exists.ecdb"));
    ProfileVersion const& actualVersion = m_ecdb.GetECDbProfileVersion();
    EXPECT_EQ(ProfileVersion(4, 0, 0, 6), actualVersion) << "Fresh ECDb must be at profile 4.0.0.6 after Task 2.1";

    EXPECT_TRUE(GetHelper().TableExists("ec_Feature")) << "ec_Feature table must exist in a fresh ECDb file (profile 4.0.0.6)";

    EXPECT_TRUE(GetHelper().ColumnExists("ec_Feature", "Name")) << "ec_Feature must have a Name column";
    EXPECT_TRUE(GetHelper().ColumnExists("ec_Feature", "Description")) << "ec_Feature must have a Description column";
    EXPECT_TRUE(GetHelper().ColumnExists("ec_Feature", "Compat")) << "ec_Feature must have a Compat column";

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature")) << "Failed to prepare SELECT COUNT(*) on ec_Feature";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0)) << "ec_Feature table must be empty on a fresh ECDb file";
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// Opening an ECDb with an empty feature registry must succeed in read-write mode.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, EmptyRegistry_OpenReadWriteSucceeds)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_open_rw.ecdb"))
        << "Opening a fresh ECDb with an empty feature registry must succeed";
    EXPECT_TRUE(m_ecdb.IsDbOpen());
    }

//---------------------------------------------------------------------------------------
// Opening an ECDb with an empty feature registry must succeed in read-only mode.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, EmptyRegistry_OpenReadonlySucceeds)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_open_ro.ecdb"));
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    ASSERT_EQ(BE_SQLITE_OK,
        m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly)))
        << "Read-only open of a fresh ECDb with an empty feature registry must succeed";
    EXPECT_TRUE(m_ecdb.IsDbOpen());
    }

//---------------------------------------------------------------------------------------
// Opening an ECDb with an empty feature registry must not insert any rows
// into ec_Feature (the registry check must be a pure read operation at open-time).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, EmptyRegistry_OpenDoesNotPopulateTable)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_open_nopop.ecdb"));
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0)) << "Opening an ECDb must not insert rows into ec_Feature when the registry is empty";
    }

//---------------------------------------------------------------------------------------
// An unknown feature with Compat=Warn must NOT block the open but MUST issue a warning
// that names the unknown feature.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_UnknownWarn_OpensWithWarning)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_warn.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-unknown-feature", "Warn"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << "A Warn feature must not prevent the file from opening";
    EXPECT_TRUE(m_ecdb.IsDbOpen());

    ASSERT_FALSE(issueListener.IsEmpty());

    const ReportedIssue& issue = issueListener.m_issues.back();
    EXPECT_EQ(IssueSeverity::Warning, issue.severity);
    EXPECT_STREQ(issue.message.c_str(), "ECDb file uses unknown features \"future-unknown-feature\". Some data may not be accessible.");
    }

//---------------------------------------------------------------------------------------
// An unknown feature with Compat=ReadOnly must return BE_SQLITE_READONLY and leave the
// database closed when a read-write open is attempted.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_UnknownReadOnly_BlocksWrite)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_readonly.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("unknown-readonly-feature", "ReadOnly"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BE_SQLITE_READONLY, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << "An unknown ReadOnly feature must return BE_SQLITE_READONLY on a read-write open attempt";
    EXPECT_FALSE(m_ecdb.IsDbOpen()) << "The database must be closed after the failed open";

    ASSERT_FALSE(issueListener.IsEmpty()) << "The failed open must report at least one issue";
    const ReportedIssue& issue = issueListener.m_issues.back();
    EXPECT_EQ(IssueSeverity::Error, issue.severity) << "The issue severity must be Error";
    EXPECT_TRUE(issue.message.EqualsI("ECDb file uses unknown features \"unknown-readonly-feature\". The file can only be opened read-only."));

    issueListener.ClearIssues();
    m_ecdb.AddIssueListener(issueListener);
    // If the ECDb is being opened in read-only mode, it should succeed
    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << "An unknown ReadOnly feature must allow a read-only open";
    EXPECT_TRUE(m_ecdb.IsDbOpen());
    EXPECT_TRUE(m_ecdb.IsReadonly());
    ASSERT_TRUE(issueListener.IsEmpty()) << "The failed open must not report any issues";
    }

//----------------------------------------------------------------------------------------------
// An unknown feature with Compat=NoSchemaImport must open normally but disallow schema imports
// @bsimethod
//----------------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_UnknownNoSchemaImport_BlocksSchemaImports)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_noImports.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("unknown-noImports-feature", "NoSchemaImport"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << "An unknown NoSchemaImport feature must return BE_SQLITE_OK on a read-write open attempt";
    EXPECT_TRUE(m_ecdb.IsDbOpen()) << "The database must be open after the successful open";

    ASSERT_FALSE(issueListener.IsEmpty());
    const ReportedIssue& issue = issueListener.m_issues.back();
    EXPECT_EQ(IssueSeverity::Warning, issue.severity);
    EXPECT_TRUE(issue.message.EqualsI("ECDb file uses unknown features \"unknown-noImports-feature\". The file will restrict all schema imports. However, it can still be written to."));

    // Try to import a schema
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());

    ECSchemaPtr updatedSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(updatedSchema, R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="2.0.0" alias="ecdbmap" />
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="Identifier" typeName="string" />
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", *ctx));

    issueListener.ClearIssues();
    m_ecdb.AddIssueListener(issueListener);

    ASSERT_EQ(ERROR, m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()));
    
    ASSERT_FALSE(issueListener.IsEmpty());
    const ReportedIssue& importIssue = issueListener.m_issues.back();
    EXPECT_EQ(IssueSeverity::Error, importIssue.severity);
    EXPECT_TRUE(importIssue.message.EqualsI("Schema import is not allowed. The ECDb file uses an unknown feature \"unknown-noImports-feature\" that disables schema imports."));
    }

//---------------------------------------------------------------------------------------
// An unknown feature with Compat=Refuse must block ALL opens, both read-write and
// read-only, returning BE_SQLITE_ERROR in both cases.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_UnknownRefuse_BlocksAllOpen)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_refuse.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("unknown-refuse-feature", "Refuse"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    // Try to open the ECDb in read-write mode. Should fail.
    EXPECT_EQ(BE_SQLITE_ERROR, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << "An unknown Refuse feature must block a read-write open";
    EXPECT_FALSE(m_ecdb.IsDbOpen()) << "The database must remain closed after the failed read-write open";

    ASSERT_FALSE(issueListener.IsEmpty()) << "The failed open must report at least one issue";
    const ReportedIssue& issue = issueListener.m_issues.back();
    EXPECT_EQ(IssueSeverity::Error, issue.severity) << "The issue severity must be Error";
    EXPECT_TRUE(issue.message.EqualsI("ECDb file uses unknown feature 'unknown-refuse-feature'. The file cannot be opened by this ECDb runtime."));

    // Try to open the ECDb in read-only mode. Should fail.
    issueListener.ClearIssues();
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BE_SQLITE_ERROR, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << "An unknown Refuse feature must also block a read-only open";
    EXPECT_FALSE(m_ecdb.IsDbOpen()) << "The database must remain closed after the failed read-only open";

    ASSERT_FALSE(issueListener.IsEmpty()) << "The failed open must report at least one issue";
    EXPECT_EQ(IssueSeverity::Error, issue.severity) << "The issue severity must be Error";
    EXPECT_TRUE(issue.message.EqualsI("ECDb file uses unknown feature 'unknown-refuse-feature'. The file cannot be opened by this ECDb runtime."));
    }

TEST_F(FeatureTests, Feature_RefuseShouldTakePrecedence)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_readonly_then_refuse.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-readonly-feature", "ReadOnly"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-warn-feature", "Warn"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-refuse-feature", "Refuse"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-no-schema-import-feature", "NoSchemaImport"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BE_SQLITE_ERROR, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));
    EXPECT_FALSE(m_ecdb.IsDbOpen());

    ASSERT_FALSE(issueListener.IsEmpty());

    ReportedIssue issue = issueListener.m_issues.back();
    EXPECT_EQ(IssueSeverity::Error, issue.severity);
    EXPECT_TRUE(issue.message.EqualsI("ECDb file uses unknown feature 'future-refuse-feature'. The file cannot be opened by this ECDb runtime."));
    }

TEST_F(FeatureTests, Feature_WarningsAndErrorsShouldBeReported)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_readonly_then_refuse.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-readonly-feature1", "ReadOnly"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-readonly-feature2", "ReadOnly"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-warn-feature1", "Warn"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-warn-feature2", "Warn"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-warn-feature3", "Warn"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-no-schema-import-feature", "NoSchemaImport"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BE_SQLITE_READONLY, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));
    EXPECT_FALSE(m_ecdb.IsDbOpen());

    ASSERT_FALSE(issueListener.IsEmpty());
    EXPECT_EQ(3, issueListener.m_issues.size());

    ReportedIssue warning = issueListener.m_issues[0];
    EXPECT_EQ(IssueSeverity::Warning, warning.severity);
    EXPECT_STREQ(warning.message.c_str(), "ECDb file uses unknown features \"future-warn-feature1\", \"future-warn-feature2\", \"future-warn-feature3\". Some data may not be accessible.");

    ReportedIssue noSchemaImport = issueListener.m_issues[1];
    EXPECT_EQ(IssueSeverity::Warning, noSchemaImport.severity);
    EXPECT_STREQ(noSchemaImport.message.c_str(), "ECDb file uses unknown features \"future-no-schema-import-feature\". The file will restrict all schema imports. However, it can still be written to.");

    ReportedIssue readOnly = issueListener.m_issues[2];
    EXPECT_EQ(IssueSeverity::Error, readOnly.severity);
    EXPECT_STREQ(readOnly.message.c_str(), "ECDb file uses unknown features \"future-readonly-feature1\", \"future-readonly-feature2\". The file can only be opened read-only.");
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_CompatToString_AllValues)
    {
    EXPECT_STREQ("Warn",     FeatureManager::FeatureCompatToString(Compat::Warn));
    EXPECT_STREQ("ReadOnly", FeatureManager::FeatureCompatToString(Compat::ReadOnly));
    EXPECT_STREQ("NoSchemaImport", FeatureManager::FeatureCompatToString(Compat::NoSchemaImport));
    EXPECT_STREQ("Refuse",   FeatureManager::FeatureCompatToString(Compat::Refuse));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_InsertFeature_UnknownName_ReturnsError)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("insert_feature_unknown.ecdb"));

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BentleyStatus::ERROR, FeatureManager::InsertFeature(m_ecdb, "completely-unknown-feature"));
    EXPECT_FALSE(issueListener.IsEmpty()) << "An error issue must be reported for the unknown feature";

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0)) << "No row must be inserted for an unknown feature name";
    }

//---------------------------------------------------------------------------------------
// InsertFeature must update the Description and Compat columns of a pre-existing row
// when the runtime's definition of the feature has changed (e.g. compat was tightened
// from Warn to ReadOnly in a newer release).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_InsertFeature_StaleRow_UpdatesDescriptionAndCompat)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("insert_feature_stale.ecdb"));

    // Manually insert a row with an outdated warn compat.
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("INSERT INTO ec_Feature(Name, Description, Compat) VALUES('json-primitive-type', 'OldDescription', 'Warn')"));

    // Verify the stale row is in place before the upsert.
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Description, Compat FROM ec_Feature WHERE Name='json-primitive-type'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("OldDescription", stmt.GetValueText(0));
    EXPECT_STREQ("Warn", stmt.GetValueText(1));
    }

    // Insert should update the feature with the new description and compat value.
    ASSERT_EQ(BentleyStatus::SUCCESS, FeatureManager::InsertFeature(m_ecdb, "json-primitive-type"));

    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Description, Compat FROM ec_Feature WHERE Name='json-primitive-type'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("JSON primitive type with json description Schema Item", stmt.GetValueText(0));
    EXPECT_STREQ("ReadOnly", stmt.GetValueText(1));
    stmt.Finalize();

    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature WHERE Name='json-primitive-type'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(1, stmt.GetValueInt(0));
    }
    }

//---------------------------------------------------------------------------------------
// PRAGMA ecdb_known_features must reject write operations and return BE_SQLITE_READONLY.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_PragmaKnownFeatures_Write_IsReadOnly)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("pragma_known_features_write.ecdb"));

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus(BE_SQLITE_READONLY), stmt.Prepare(m_ecdb, "PRAGMA ecdb_known_features=foo")) << "Writing to ecdb_known_features must fail with BE_SQLITE_READONLY";
    EXPECT_FALSE(issueListener.IsEmpty()) << "An error issue must be reported when attempting to write to a read-only PRAGMA";
    }

//---------------------------------------------------------------------------------------
// PRAGMA ecdb_used_features must return an empty result set when the ec_Feature table
// exists but contains no rows.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_PragmaUsedFeatures_EmptyTable_ReturnsEmpty)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("pragma_used_features_empty.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA ecdb_used_features")) << "PRAGMA ecdb_used_features must prepare successfully even with an empty table";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << "PRAGMA ecdb_used_features must return no rows when the ec_Feature table is empty";
    }

//---------------------------------------------------------------------------------------
// PRAGMA ecdb_used_features must reject write operations and return BE_SQLITE_READONLY.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_PragmaUsedFeatures_Write_IsReadOnly)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("pragma_used_features_write.ecdb"));

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus(BE_SQLITE_READONLY), stmt.Prepare(m_ecdb, "PRAGMA ecdb_used_features=foo")) << "Writing to ecdb_used_features must fail with BE_SQLITE_READONLY";
    EXPECT_FALSE(issueListener.IsEmpty()) << "An error issue must be reported when attempting to write to a read-only PRAGMA";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_PragmaUsedFeatures_PreMigrationFile_ReturnsEmpty)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("pragma_used_features_premigration.ecdb"));

    // Drop the ec_Feature table to simulate a pre-4.0.0.6 file where the table doesn't exist.
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("DROP TABLE ec_Feature")) << "DROP TABLE must succeed on an open connection";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA ecdb_used_features")) << "PRAGMA ecdb_used_features must prepare successfully even without the table";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << "PRAGMA ecdb_used_features must return no rows (not an error) when the ec_Feature table is absent";
    }

TEST_F(FeatureTests, Feature_JSONPrimitiveType_IsRegistered)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_json_primitive_type.ecdb"));

    EXPECT_TRUE(FeatureManager::IsFeatureKnown("json-primitive-type")) << "\"json-primitive-type\" must be registered in the ECDb known-feature registry";
    EXPECT_TRUE(FeatureManager::IsFeatureKnown(Feature::JsonPrimitiveType)) << "\"json-primitive-type\" must be registered in the ECDb known-feature registry";
    }

// Helper: ECSchema XML fragment for a V3.3 schema that uses a json-typed property.
static constexpr Utf8CP s_schemaWithJson = R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <ECEntityClass typeName="JsonHolder">
            <ECProperty propertyName="Data" typeName="json"/>
        </ECEntityClass>
    </ECSchema>)xml";

// Helper: the same schema updated to v1.0.1, with the json property removed.
static constexpr Utf8CP s_schemaWithoutJson = R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <ECEntityClass typeName="JsonHolder">
            <ECProperty propertyName="Data" typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml";

// Helper: a second schema that also uses a json-typed property.
static constexpr Utf8CP s_schemaWithJson2 = R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <ECEntityClass typeName="JsonHolder2">
            <ECProperty propertyName="Payload" typeName="json"/>
        </ECEntityClass>
    </ECSchema>)xml";

//---------------------------------------------------------------------------------------
// Importing a V3.3 schema that contains a json-typed property must insert a
// "json-primitive-type" row into ec_Feature.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, JsonPrimitive_ImportSchema_InsertsFeatureRow)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("json_import_row.ecdb"));

    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, s_schemaWithJson, *ctx));
    ASSERT_EQ(BentleyStatus::SUCCESS, m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Compat FROM ec_Feature WHERE Name='json-primitive-type'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "ec_Feature must have a row for \"json-primitive-type\" after import";
    EXPECT_STREQ("ReadOnly", stmt.GetValueText(0)) << "Compat must be ReadOnly";
    }

//---------------------------------------------------------------------------------------
// Importing a V3.3 schema that has NO json-typed properties must NOT insert any feature row.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, JsonPrimitive_ImportSchema_NoJson_NoFeatureRow)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("json_import_no_json.ecdb"));

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='PlainSchema' alias='ps' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.3'>"
        "  <ECEntityClass typeName='Foo'>"
        "    <ECProperty propertyName='Name' typeName='string'/>"
        "  </ECEntityClass>"
        "</ECSchema>")));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature WHERE Name='json-primitive-type'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0)) << "ec_Feature must have no row for \"json-primitive-type\" when no json properties exist";
    }

//---------------------------------------------------------------------------------------
// Upgrading a schema to remove its last json-typed property must delete the feature row.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, JsonPrimitive_UpgradeSchema_RemovesJsonProperty_ClearsFeatureRow)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("json_upgrade_clear.ecdb"));

    // Import with json property : row inserted.
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, s_schemaWithJson, *ctx));
    ASSERT_EQ(BentleyStatus::SUCCESS, m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature WHERE Name='json-primitive-type'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(1, stmt.GetValueInt(0)) << "Feature row must exist after first import";
    stmt.Finalize();

    // Drop the schema that is using the json primitive property
    const auto dropStatus = m_ecdb.Schemas().DropSchema("TestSchema");
    ASSERT_TRUE(dropStatus.IsSuccess());

    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature WHERE Name='json-primitive-type'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0)) << "Feature row must be deleted when no json properties remain";
    }

//---------------------------------------------------------------------------------------
// When two schemas both use json-typed properties, removing json from one schema must NOT
// delete the feature row as long as the other schema still uses it.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, JsonPrimitive_UpgradeSchema_TwoSchemas_RowRemainsIfOneStillUsesJson)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("json_upgrade_two_schemas.ecdb"));

    // Import both schemas with json properties.
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr schema1, schema2;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema1, s_schemaWithJson, *ctx));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, s_schemaWithJson2, *ctx));
    ASSERT_EQ(BentleyStatus::SUCCESS, m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()));

    // Drop the schema TestSchema
    const auto dropStatus = m_ecdb.Schemas().DropSchema("TestSchema");
    ASSERT_TRUE(dropStatus.IsSuccess());

    // TestSchema2 still has a json property : feature row must remain.
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature WHERE Name='json-primitive-type'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(1, stmt.GetValueInt(0)) << "Feature row must remain while at least one schema still uses json properties";
    }

//---------------------------------------------------------------------------------------
// A json-typed property must be insertable and selectable via ECSql.
// The value round-trips as a text string.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, JsonPrimitive_ECSql_InsertAndSelect)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("json_ecsql_roundtrip.ecdb"));

    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, s_schemaWithJson, *ctx));
    ASSERT_EQ(BentleyStatus::SUCCESS, m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()));

    // Insert a row with a JSON string value.
    static constexpr Utf8CP testJson = R"({"answer":42})";
    {
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO ts.JsonHolder(Data) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, insert.BindText(1, testJson, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    }

    // Select it back and verify the value round-trips.
    {
    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT Data FROM ts.JsonHolder LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ(testJson, select.GetValueText(0)) << "json property value must round-trip through INSERT/SELECT";
    }
    }

END_ECDBUNITTESTS_NAMESPACE