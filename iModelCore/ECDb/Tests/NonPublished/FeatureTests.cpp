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
    //! Inserts a raw feature row. This simulates a file written by a FUTURE ECDb runtime that uses a
    //! feature the current runtime does not know.
    //! @param compat the (possibly future/unrecognized) precise compat mode
    //! @param fallback how this runtime must degrade when it does not recognize @p compat.
    //!        Defaults to @p compat, which is the normal case for features using existing modes.
    DbResult InsertRawFeatureRow(Utf8CP name, Utf8CP compat, Utf8CP fallback = nullptr)
        {
        return m_ecdb.TryExecuteSql(Utf8PrintfString("INSERT INTO ec_Feature(Name, Description, Compat, Fallback) VALUES ('%s', '%s', '%s', '%s')",
            name, "A-Feature-From-The-Future", compat, fallback == nullptr ? compat : fallback).c_str());
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
    EXPECT_TRUE(GetHelper().ColumnExists("ec_Feature", "Fallback")) << "ec_Feature must have a Fallback column";

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
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_unknown_refuse_blocks_open.ecdb"));
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
    EXPECT_EQ(IssueSeverity::Error, issueListener.m_issues.back().severity) << "The issue severity must be Error";
    EXPECT_STREQ("ECDb file uses unknown features \"unknown-refuse-feature\". The file cannot be opened.", issueListener.m_issues.back().message.c_str());

    // Try to open the ECDb in read-only mode. Should fail.
    issueListener.ClearIssues();
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BE_SQLITE_ERROR, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << "An unknown Refuse feature must also block a read-only open";
    EXPECT_FALSE(m_ecdb.IsDbOpen()) << "The database must remain closed after the failed read-only open";

    ASSERT_FALSE(issueListener.IsEmpty()) << "The failed open must report at least one issue";
    EXPECT_EQ(IssueSeverity::Error, issueListener.m_issues.back().severity) << "The issue severity must be Error";
    EXPECT_STREQ("ECDb file uses unknown features \"unknown-refuse-feature\". The file cannot be opened.", issueListener.m_issues.back().message.c_str());
    }

//---------------------------------------------------------------------------------------
// Smoke test: when all four compat modes are present at once, Refuse (the most severe)
// must determine the outcome. See Feature_RefuseTakesPrecedenceOverReadOnly for the
// focused Refuse-vs-ReadOnly ordering regression guard.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_RefuseWinsOverAllOtherModes)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_all_modes_refuse_wins.ecdb"));
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
    EXPECT_STREQ("ECDb file uses unknown features \"future-refuse-feature\". The file cannot be opened.", issue.message.c_str());
    }

//---------------------------------------------------------------------------------------
// Regression guard: when a file carries BOTH an unknown ReadOnly feature and an unknown
// Refuse feature, the more severe mode must win. Refuse must be evaluated before ReadOnly,
// otherwise a read-write open degrades to BE_SQLITE_READONLY and the file is opened
// read-only when it must not be opened at all.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_RefuseTakesPrecedenceOverReadOnly)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_refuse_beats_readonly.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-readonly-feature", "ReadOnly"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-refuse-feature", "Refuse"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    // Read-write: Refuse must win over ReadOnly.
        {
        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        EXPECT_EQ(BE_SQLITE_ERROR, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)))
            << "Refuse must take precedence over ReadOnly on a read-write open";
        EXPECT_FALSE(m_ecdb.IsDbOpen()) << "The database must remain closed, not fall back to read-only";

        ASSERT_FALSE(issueListener.IsEmpty());
        EXPECT_EQ(IssueSeverity::Error, issueListener.m_issues.back().severity);
        EXPECT_STREQ("ECDb file uses unknown features \"future-refuse-feature\". The file cannot be opened.", issueListener.m_issues.back().message.c_str())
            << "The reported issue must be the Refuse issue, not the ReadOnly one";

        for (ReportedIssue const& reported : issueListener.m_issues)
            EXPECT_EQ(Utf8String::npos, reported.message.find("can only be opened read-only"))
                << "The ReadOnly issue must not be reported once a Refuse feature has already blocked the open";
        }

    // Read-only: Refuse must still block, even though ReadOnly alone would have been satisfied.
        {
        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        EXPECT_EQ(BE_SQLITE_ERROR, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly)))
            << "Refuse must block a read-only open as well";
        EXPECT_FALSE(m_ecdb.IsDbOpen());

        ASSERT_FALSE(issueListener.IsEmpty());
        EXPECT_EQ(IssueSeverity::Error, issueListener.m_issues.back().severity);
        EXPECT_STREQ("ECDb file uses unknown features \"future-refuse-feature\". The file cannot be opened.", issueListener.m_issues.back().message.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// When a file carries both an unknown NoSchemaImport feature and an unknown Refuse
// feature, Refuse must win: the file must not open at all, in either mode.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_RefuseTakesPrecedenceOverNoSchemaImport)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_refuse_beats_noschemaimport.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-no-schema-import-feature", "NoSchemaImport"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-refuse-feature", "Refuse"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    for (Db::OpenMode openMode : {Db::OpenMode::ReadWrite, Db::OpenMode::Readonly})
        {
        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        EXPECT_EQ(BE_SQLITE_ERROR, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(openMode)))
            << "Refuse must take precedence over NoSchemaImport";
        EXPECT_FALSE(m_ecdb.IsDbOpen());

        ASSERT_FALSE(issueListener.IsEmpty());
        EXPECT_EQ(IssueSeverity::Error, issueListener.m_issues.back().severity);
        EXPECT_STREQ("ECDb file uses unknown features \"future-refuse-feature\". The file cannot be opened.", issueListener.m_issues.back().message.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// Feature names are matched case-insensitively against the known-feature registry and
// therefore must also be reported verbatim as written in the file.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_UnknownName_IsReportedVerbatimRegardlessOfCasing)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_name_casing.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("FUTURE-Warn-FEATURE", "Warn"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_FALSE(issueListener.IsEmpty());
    EXPECT_STREQ("ECDb file uses unknown features \"FUTURE-Warn-FEATURE\". Some data may not be accessible.", issueListener.m_issues.back().message.c_str())
        << "The unknown feature name must be echoed exactly as stored in the file";
    }

TEST_F(FeatureTests, Feature_WarningsAndErrorsShouldBeReported)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_warn_and_error_reporting.ecdb"));
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
    ASSERT_EQ(3, issueListener.m_issues.size());

    auto findIssue = [&](Utf8CP expectedMessage) -> ReportedIssue const*
        {
        for (ReportedIssue const& reported : issueListener.m_issues)
            {
            if (reported.message.Equals(expectedMessage))
                return &reported;
            }
        return nullptr;
        };

    ReportedIssue const* warning = findIssue("ECDb file uses unknown features \"future-warn-feature1\", \"future-warn-feature2\", \"future-warn-feature3\". Some data may not be accessible.");
    ASSERT_NE(nullptr, warning) << "The aggregated Warn issue was not reported";
    EXPECT_EQ(IssueSeverity::Warning, warning->severity);

    ReportedIssue const* noSchemaImport = findIssue("ECDb file uses unknown features \"future-no-schema-import-feature\". The file will restrict all schema imports. However, it can still be written to.");
    ASSERT_NE(nullptr, noSchemaImport) << "The NoSchemaImport issue was not reported";
    EXPECT_EQ(IssueSeverity::Warning, noSchemaImport->severity);

    ReportedIssue const* readOnly = findIssue("ECDb file uses unknown features \"future-readonly-feature1\", \"future-readonly-feature2\". The file can only be opened read-only.");
    ASSERT_NE(nullptr, readOnly) << "The aggregated ReadOnly issue was not reported";
    EXPECT_EQ(IssueSeverity::Error, readOnly->severity);
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_ResolveEffectiveCompat)
    {
    Compat compat;

    EXPECT_TRUE(FeatureManager::TryParseCompat("Warn", compat));
    EXPECT_EQ(Compat::Warn, compat);
    EXPECT_TRUE(FeatureManager::TryParseCompat("noschemaimport", compat));
    EXPECT_EQ(Compat::NoSchemaImport, compat);
    EXPECT_FALSE(FeatureManager::TryParseCompat("SomeFutureMode", compat));

    // Recognized Compat wins.
    EXPECT_EQ(Compat::ReadOnly, FeatureManager::ResolveEffectiveCompat("ReadOnly", "Warn"));
    // Unrecognized Compat degrades to the declared Fallback.
    EXPECT_EQ(Compat::Warn, FeatureManager::ResolveEffectiveCompat("SomeFutureMode", "Warn"));
    // Neither recognized -> fail closed.
    EXPECT_EQ(Compat::Refuse, FeatureManager::ResolveEffectiveCompat("SomeFutureMode", "AlsoUnknown"));
    EXPECT_EQ(Compat::Refuse, FeatureManager::ResolveEffectiveCompat("", ""));
    }

//---------------------------------------------------------------------------------------
// A future runtime may write a Compat mode this runtime has never heard of. In that case the
// writer-declared Fallback must drive the behavior.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_UnrecognizedCompat_UsesFallback_Warn)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_fallback_warn.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-benign-feature", "SomeFutureBenignMode", "Warn"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));
    EXPECT_TRUE(m_ecdb.IsDbOpen());
    EXPECT_FALSE(m_ecdb.IsReadonly());
    ASSERT_FALSE(issueListener.IsEmpty());
    EXPECT_EQ(IssueSeverity::Warning, issueListener.m_issues.back().severity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_UnrecognizedCompat_UsesFallback_ReadOnly)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_fallback_readonly.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("future-risky-feature", "SomeFutureRiskyMode", "ReadOnly"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    EXPECT_EQ(BE_SQLITE_READONLY, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));
    EXPECT_FALSE(m_ecdb.IsDbOpen());

    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly)));
    EXPECT_TRUE(m_ecdb.IsDbOpen());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, Feature_FallbackColumn_RejectsUnknownValues)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_fallback_check.ecdb"));

    // Compat is intentionally unconstrained: it must accept future modes.
    EXPECT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("f1", "AnyFutureMode", "Warn")) << "Compat can accept values unknown to this runtime";

    // Fallback is constrained to the modes that exist as of 4.0.0.6.
    EXPECT_NE(BE_SQLITE_OK, InsertRawFeatureRow("f2", "AnyFutureMode", "AnyFutureFallback")) << "Fallback must reject values outside the closed compat set";
    }

//---------------------------------------------------------------------------------------
// A changeset from a newer runtime can add feature rows to an already-open file.
// RevalidateFeatures must notice rows that appear after open.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, RevalidatePicksUpFeaturesAddedAfterOpen)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_revalidate_added.ecdb"));

    // Nothing unknown yet.
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.RevalidateFeatures());
    ASSERT_TRUE(m_ecdb.GetFeaturesBlockingSchemaImport().empty());

    // Simulate a pulled changeset introducing a feature this runtime has never heard of.
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("post-open-feature", "NoSchemaImport"));

    // Until we revalidate, the cached state is stale - this is exactly the hole being closed.
    ASSERT_TRUE(m_ecdb.GetFeaturesBlockingSchemaImport().empty()) << "Cached state is only refreshed on demand";

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.RevalidateFeatures()) << "NoSchemaImport keeps the file usable";
    ASSERT_EQ(1, m_ecdb.GetFeaturesBlockingSchemaImport().size());
    EXPECT_STREQ("post-open-feature", m_ecdb.GetFeaturesBlockingSchemaImport().front().c_str());
    }

//---------------------------------------------------------------------------------------
// Revalidation must report the severe modes through its return value so the caller can abandon.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, RevalidateReportsReadOnlyAndRefuse)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_revalidate_severe.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.RevalidateFeatures());

    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("post-open-readonly-feature", "ReadOnly"));
    EXPECT_EQ(BE_SQLITE_READONLY, m_ecdb.RevalidateFeatures()) << "A ReadOnly feature appearing on a read-write connection must be reported";

    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("post-open-refuse-feature", "Refuse"));
    EXPECT_EQ(BE_SQLITE_ERROR, m_ecdb.RevalidateFeatures()) << "Refuse must outrank ReadOnly";
    }

//---------------------------------------------------------------------------------------
// Revalidation recomputes from scratch. It must not accumulate duplicates when called repeatedly,
// and it must drop features that are no longer present (e.g. after a changeset is reversed).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, RevalidateIsIdempotent)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_revalidate_idempotent.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("no-schema-feature", "NoSchemaImport"));

    for (int i = 0; i < 3; ++i)
        {
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.RevalidateFeatures());
        ASSERT_EQ(1, m_ecdb.GetFeaturesBlockingSchemaImport().size()) << "Repeated revalidation must not accumulate duplicates";
        }

    // Reversing a changeset can remove feature rows again - the restriction must be lifted.
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.TryExecuteSql("DELETE FROM ec_Feature"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.RevalidateFeatures());
    EXPECT_TRUE(m_ecdb.GetFeaturesBlockingSchemaImport().empty()) << "Stale restrictions must be cleared";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, TryGetBlockingFeatures_DiagnosesARefusedFile)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_refuse.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("feature_refuse", "Refuse"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    // Confirm the file is indeed unopenable by this runtime
    ASSERT_EQ(BE_SQLITE_ERROR, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_FALSE(m_ecdb.IsDbOpen());

    std::vector<Utf8String> blockingFeatureNames;
    ASSERT_EQ(SUCCESS, ECDb::TryGetBlockingFeatures(blockingFeatureNames, filePath));
    ASSERT_EQ(1, blockingFeatureNames.size());
    EXPECT_STREQ("feature_refuse", blockingFeatureNames[0].c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, TryGetBlockingFeatures_EmptyForCleanFile)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_clean.ecdb"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    std::vector<Utf8String> blockingFeatureNames;
    ASSERT_EQ(SUCCESS, ECDb::TryGetBlockingFeatures(blockingFeatureNames, filePath));
    EXPECT_TRUE(blockingFeatureNames.empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, TryGetBlockingFeatures_FailsForNonexistentFile)
    {
    std::vector<Utf8String> blockingFeatureNames;
    EXPECT_EQ(ERROR, ECDb::TryGetBlockingFeatures(blockingFeatureNames, BeFileName(L"ThisFileDoesNotExist.ecdb")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, TryGetBlockingFeatures_OnlyReportsRefuseLevelEntries)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_mixed.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("feature_warn", "Warn"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("feature_readonly", "ReadOnly"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("feature_noschemaimport", "NoSchemaImport"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("feature_refuse", "Refuse"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    std::vector<Utf8String> blockingFeatureNames;
    ASSERT_EQ(SUCCESS, ECDb::TryGetBlockingFeatures(blockingFeatureNames, filePath));
    ASSERT_EQ(1, blockingFeatureNames.size()) << "only the Refuse-level row should be reported";
    EXPECT_STREQ("feature_refuse", blockingFeatureNames[0].c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, TryGetBlockingFeatures_RecognizedCompatWinsOverRefuseFallback)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_recognized_wins.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, InsertRawFeatureRow("feature_readonly_with_refuse_fallback", "ReadOnly", "Refuse"));
    m_ecdb.SaveChanges();
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    // The file must still be openable read-only, confirming this is NOT actually a Refuse situation.
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly)));
    m_ecdb.CloseDb();

    std::vector<Utf8String> blockingFeatureNames;
    ASSERT_EQ(SUCCESS, ECDb::TryGetBlockingFeatures(blockingFeatureNames, filePath));
    EXPECT_TRUE(blockingFeatureNames.empty()) << "Compat=ReadOnly is recognized and must win over a Fallback of Refuse";
    }

END_ECDBUNITTESTS_NAMESPACE