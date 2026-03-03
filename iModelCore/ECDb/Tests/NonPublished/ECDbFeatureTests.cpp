/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <BeSQLite/ChangeSet.h>
USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Minimal helpers so we can create and apply changesets inside tests
//=======================================================================================

//! Concrete ChangeSet subclass that aborts on any conflict (should not occur in our tests).
struct TestChangeSet : BeSQLite::ChangeSet
    {
    virtual ConflictResolution _OnConflict(ConflictCause, BeSQLite::Changes::Change) override
        { return ConflictResolution::Abort; }
    };

//! Concrete ChangeTracker that tracks all tables.
struct TestChangeTracker : BeSQLite::ChangeTracker
    {
    explicit TestChangeTracker(BeSQLite::DbR db)
        {
        SetDb(&db);
        AddRef();
        }

    virtual OnCommitStatus _OnCommit(bool, Utf8CP) override
        { return OnCommitStatus::Commit; }

    virtual TrackChangesForTable _FilterTable(Utf8CP) override
        { return TrackChangesForTable::Yes; }
    };

//=======================================================================================
// Test fixture
//=======================================================================================

struct FeatureTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, Registry_ContainsStrictSchemaLoading)
    {
    ECDbFeatureRegistry const& registry = ECDbFeatureRegistry::GetInstance();

    EXPECT_NE(nullptr, registry.Find(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));
    if (auto* desc = registry.Find(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING))
        {
        EXPECT_STREQ("strict-schema-loading", desc->GetName().c_str());
        EXPECT_EQ(ECDbFeatureStatus::Experimental, desc->GetStatus());
        EXPECT_FALSE(desc->GetLabel().empty());
        EXPECT_FALSE(desc->GetDescription().empty());
        }

    // Contains() convenience method
    EXPECT_TRUE(registry.Contains(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));
    EXPECT_FALSE(registry.Contains("no-such-feature"));
    EXPECT_EQ(nullptr, registry.Find("no-such-feature"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, StatusToString)
    {
    EXPECT_STREQ("Experimental",  ECDbFeatureDescriptor::StatusToString(ECDbFeatureStatus::Experimental));
    EXPECT_STREQ("Stable",        ECDbFeatureDescriptor::StatusToString(ECDbFeatureStatus::Stable));
    EXPECT_STREQ("Deprecated",    ECDbFeatureDescriptor::StatusToString(ECDbFeatureStatus::Deprecated));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, NewFile_HasNoEnabledFeatures)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feature_newfile.ecdb"));

    ECDbFeatureSet const& features = m_ecdb.GetEnabledFeatures();
    EXPECT_EQ(0u, features.Count()) << "New file should have zero enabled features (none are Stable+enabledByDefault yet).";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, EnableFeature_Persist)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_persist.ecdb"));

    // No features on a fresh file
    EXPECT_EQ(0u, m_ecdb.GetEnabledFeatures().Count());

    // Enable inside the implicit write transaction
    ASSERT_EQ(SUCCESS, m_ecdb.EnableFeature(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));

    // Visible within the current transaction
    EXPECT_TRUE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));

    // Commit and verify it survived
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    EXPECT_TRUE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));

    // Close and reopen – must survive persistence
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_TRUE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));
    EXPECT_TRUE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, EnableFeature_Idempotent)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_idempotent.ecdb"));

    // Enable twice – both calls must return SUCCESS
    EXPECT_EQ(SUCCESS, m_ecdb.EnableFeature(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));
    EXPECT_EQ(SUCCESS, m_ecdb.EnableFeature(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));

    // Still exactly one entry
    EXPECT_EQ(1u, m_ecdb.GetEnabledFeatures().Count());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, EnableFeature_UnknownName_ReturnsError)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_unknown.ecdb"));

    TestIssueListener issues;
    m_ecdb.AddIssueListener(issues);

    BentleyStatus stat = m_ecdb.EnableFeature("no-such-feature");

    m_ecdb.RemoveIssueListener();
    EXPECT_EQ(ERROR, stat);
    EXPECT_FALSE(issues.IsEmpty()) << "An error issue must have been reported.";

    // No feature must be recorded
    EXPECT_EQ(0u, m_ecdb.GetEnabledFeatures().Count());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, EnableFeature_NullName_ReturnsError)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_null_name.ecdb"));

    TestIssueListener issues;
    m_ecdb.AddIssueListener(issues);

    EXPECT_EQ(ERROR, m_ecdb.EnableFeature(nullptr));
    EXPECT_EQ(ERROR, m_ecdb.EnableFeature(""));

    m_ecdb.RemoveIssueListener();
    EXPECT_FALSE(issues.IsEmpty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, EnableFeature_ReadOnly_ReturnsError)
    {
    // Create a file first, then reopen read-only
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_readonly.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    TestIssueListener issues;
    m_ecdb.AddIssueListener(issues);

    BentleyStatus stat = m_ecdb.EnableFeature(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING);

    m_ecdb.RemoveIssueListener();
    EXPECT_EQ(ERROR, stat);
    EXPECT_FALSE(issues.IsEmpty());
    }

//---------------------------------------------------------------------------------------
//! KEY TEST: AbandonChanges must undo an in-flight EnableFeature.
//!
//! Because EnableFeature writes to be_prop inside a SQLite transaction,
//! a rollback (AbandonChanges) must bring the file state back to the pre-enable
//! condition.  The next access to GetEnabledFeatures() will re-read from
//! SQLite and reflect the reverted state.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, EnableFeature_TxnRevert_AbandonChanges)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_txn_revert.ecdb"));

    // Verify clean slate
    EXPECT_EQ(0u, m_ecdb.GetEnabledFeatures().Count());

    // Enable feature within the transaction
    ASSERT_EQ(SUCCESS, m_ecdb.EnableFeature(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));

    // Visible within the current (uncommitted) transaction
    EXPECT_TRUE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));

    // *** ABANDON – must roll back the be_prop write ***
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());

    // After abandon the cache is marked stale; the next read comes from SQLite.
    // SQLite has reverted the be_prop row → feature must NOT be reported as enabled.
    EXPECT_FALSE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING))
        << "Feature must not be enabled after AbandonChanges.";
    EXPECT_EQ(0u, m_ecdb.GetEnabledFeatures().Count());

    // Double-check: close and reopen – still not persisted
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_FALSE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));
    }

//---------------------------------------------------------------------------------------
//! KEY TEST: Reversing a changeset must undo a previously committed EnableFeature.
//!
//! After applying a reversed changeset (one that undoes the be_prop INSERT),
//! the feature system must reflect the reverted state.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, EnableFeature_ChangesetReversal)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_cs_reversal.ecdb"));

    // --- Step 1: Enable feature under a ChangeTracker and commit ---
    TestChangeSet changeSet;
    {
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ASSERT_EQ(SUCCESS, m_ecdb.EnableFeature(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    // Capture the changeset before stopping tracking
    ASSERT_EQ(BE_SQLITE_OK, changeSet.FromChangeTrack(tracker));
    tracker.EndTracking();
    }

    // Verify the feature is now enabled
    ASSERT_TRUE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING))
        << "Feature must be enabled after SaveChanges.";

    // --- Step 2: Apply the changeset in reverse ---
    // This undoes the be_prop INSERT (effectively deletes the row).
    ASSERT_EQ(BE_SQLITE_OK, changeSet.ApplyChanges(m_ecdb, /*invert=*/true));

    // Notify the ECDb layer that data has changed so it refreshes its cache.
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AfterDataChangeSetApplied(false));

    // --- Step 3: Verify the feature is gone ---
    EXPECT_FALSE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING))
        << "Feature must not be reported as enabled after the reversed changeset.";
    EXPECT_EQ(0u, m_ecdb.GetEnabledFeatures().Count());

    // Commit the reversal and verify on reopen
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_FALSE(m_ecdb.GetEnabledFeatures().IsEnabled(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, Pragma_ecdb_features_ReturnsRows)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_pragma_list.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA ecdb_features"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    // Column layout: name(0), label(1), description(2), status(3), enabled(4)
    EXPECT_FALSE(Utf8String::IsNullOrEmpty(stmt.GetValueText(0)));   // name
    EXPECT_FALSE(Utf8String::IsNullOrEmpty(stmt.GetValueText(1)));   // label
    EXPECT_FALSE(Utf8String::IsNullOrEmpty(stmt.GetValueText(2)));   // description
    EXPECT_FALSE(Utf8String::IsNullOrEmpty(stmt.GetValueText(3)));   // status
    EXPECT_FALSE(stmt.GetValueBoolean(4));                            // enabled=false on fresh file

    // The strict-schema-loading feature must appear
    bool foundStrictSchema = false;
    stmt.Reset();
    do
        {
        DbResult rc = stmt.Step();
        if (rc == BE_SQLITE_DONE)
            break;
        ASSERT_EQ(BE_SQLITE_ROW, rc);
        if (Utf8String(stmt.GetValueText(0)).EqualsIAscii(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING))
            {
            foundStrictSchema = true;
            EXPECT_STREQ("Experimental", stmt.GetValueText(3));
            }
        } while (true);

    EXPECT_TRUE(foundStrictSchema) << "ecdb_features PRAGMA must list strict-schema-loading.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, Pragma_ecdb_features_ShowsEnabled)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_pragma_enabled.ecdb"));

    ASSERT_EQ(SUCCESS, m_ecdb.EnableFeature(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA ecdb_features"));

    bool foundEnabled = false;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        if (Utf8String(stmt.GetValueText(0)).EqualsIAscii(ECDbFeatureRegistry::STRICT_SCHEMA_LOADING))
            {
            EXPECT_TRUE(stmt.GetValueBoolean(4)) << "strict-schema-loading must show as enabled.";
            foundEnabled = true;
            }
        }
    EXPECT_TRUE(foundEnabled);

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FeatureTestFixture, Pragma_ecdb_features_IsReadOnly)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("feat_pragma_readonly.ecdb"));

    ECSqlStatement stmt;
    // Writing to the read-only pragma must fail at Prepare time or return READONLY on Step
    ECSqlStatus stat = stmt.Prepare(m_ecdb, "PRAGMA ecdb_features=1");
    if (stat == ECSqlStatus::Success)
        EXPECT_NE(BE_SQLITE_OK, stmt.Step());       // must fail at runtime
    // Either prepare or step failure is acceptable
    }

END_ECDBUNITTESTS_NAMESPACE
