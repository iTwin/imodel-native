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
    EXPECT_TRUE(issue.message.EqualsI("ECDb file uses unknown feature 'future-unknown-feature'. Some data may not be accessible."));
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
    EXPECT_TRUE(issue.message.EqualsI("ECDb file uses unknown feature 'unknown-readonly-feature'. The file can only be opened read-only."));

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

    ASSERT_TRUE(issueListener.IsEmpty());

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
    ASSERT_FALSE(issueListener.IsEmpty()) << "The failed open must report at least one issue";
    const ReportedIssue& issue = issueListener.m_issues.back();
    EXPECT_EQ(IssueSeverity::Error, issue.severity) << "The issue severity must be Error";
    EXPECT_TRUE(issue.message.EqualsI("Schema import is not allowed. The ECDb file uses an unknown feature 'unknown-noImports-feature' that disables schema imports."));
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

END_ECDBUNITTESTS_NAMESPACE