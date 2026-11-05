/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//=======================================================================================
// Tests for the "indirect flag is preserved while applying a changeset" behavior.
//
// Background (iTwin/itwinjs-backlog issue #2346, SQLite check-in 9d36067e76):
//   Every change recorded by a session is flagged as either "direct" (made
//   explicitly by the user) or "indirect" (a side effect, e.g. produced by a
//   trigger/foreign-key action, or - with this fix - produced by applying a
//   changeset entry that was itself flagged indirect).
//
//   Before the fix, when a changeset was applied to a database that had an
//   active ChangeTracker (session) recording, EVERY applied row change was
//   recorded in that tracker as DIRECT, regardless of how the row change was
//   flagged in the source changeset. In OpenSite+ this caused a whole site to
//   be flagged as "directly modified" when only a pond (whose trigger
//   indirectly touched the site row) had actually been edited, which broke the
//   Blame feature.
//
//   The fix makes sqlite3changeset_apply() temporarily set the session
//   "indirect" flag while stepping a row change that was flagged indirect in
//   the changeset. As a result the per-change direct/indirect classification is
//   preserved end-to-end: a direct source change is recorded direct, an
//   indirect source change is recorded indirect.
//
// These tests assert exactly that per-change propagation.
//=======================================================================================
#include "BeSQLiteNonPublishedTests.h"
#include <BeSQLite/ChangeSet.h>
#include <memory>

USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// A changeset that fails the test on any unexpected conflict. None of the apply
// operations in these tests should conflict (the target already matches the
// changeset's "before" state), so a conflict indicates a broken test setup.
// @bsiclass
//=======================================================================================
struct IndirectTestChangeSet : ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause cause, Changes::Change iter) override
        {
        BeAssert(false && "Unexpected conflict while applying changeset");
        return ConflictResolution::Abort;
        }
    };

//=======================================================================================
// Minimal ChangeTracker used to capture/record changes for a Db.
// @bsiclass
//=======================================================================================
struct IndirectTestChangeTracker : ChangeTracker
    {
    explicit IndirectTestChangeTracker(DbR db) { SetDb(&db); }
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Commit; }
    };

//=======================================================================================
// @bsistruct
//=======================================================================================
struct SessionApplyIndirectTests : public ::testing::Test
    {
protected:
    SessionApplyIndirectTests()
        {
        // Initialize BeSQLite with a temp dir (required before creating/opening Dbs).
        BeFileName tempDir;
        BeTest::GetHost().GetTempDir(tempDir);
        BeSQLiteLib::Initialize(tempDir);
        }

    // The fixture owns every Db it hands out. GTest destroys test-local objects
    // only after the test body returns; by owning the Dbs here and abandoning
    // any pending writes in TearDown we avoid the BeSQLite assertion that fires
    // when a Db is destroyed with uncommitted changes.
    std::vector<std::unique_ptr<Db>> m_dbs;

    void TearDown() override
        {
        for (auto& db : m_dbs)
            {
            if (db->IsDbOpen())
                db->AbandonChanges();
            }
        m_dbs.clear();
        }

    BeFileName OutputPath(Utf8CP fileName)
        {
        BeFileName path;
        BeTest::GetHost().GetOutputRoot(path);
        path.AppendUtf8(fileName);
        return path;
        }

    // Create a fresh, empty BeSQLite Db at the given file name.
    Db* CreateDb(Utf8CP fileName)
        {
        BeFileName path = OutputPath(fileName);
        if (path.DoesPathExist())
            path.BeDeleteFile();

        auto db = std::make_unique<Db>();
        if (BE_SQLITE_OK != db->CreateNewDb(path))
            return nullptr;
        db->SaveChanges();
        m_dbs.push_back(std::move(db));
        return m_dbs.back().get();
        }

    // Copy an existing (closed) Db file to a new file name.
    BeFileNameStatus CloneFile(Utf8CP existingFile, Utf8CP newFile)
        {
        BeFileName existing = OutputPath(existingFile);
        BeFileName dest = OutputPath(newFile);
        if (dest.DoesPathExist())
            dest.BeDeleteFile();
        return BeFileName::BeCopyFile(existing, dest);
        }

    // Open an existing Db file read/write.
    Db* OpenDb(Utf8CP fileName)
        {
        BeFileName path = OutputPath(fileName);
        auto db = std::make_unique<Db>();
        if (BE_SQLITE_OK != db->OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::ReadWrite)))
            return nullptr;
        m_dbs.push_back(std::move(db));
        return m_dbs.back().get();
        }

    // Count the direct/indirect changes of a given opcode for a given table in a changeset.
    static void CountChanges(ChangeSet& cs, Utf8CP table, DbOpcode opcode, int& directCount, int& indirectCount)
        {
        directCount = 0;
        indirectCount = 0;
        for (auto const& change : cs.GetChanges())
            {
            if (!change.GetTableName().Equals(table) || change.GetOpcode() != opcode)
                continue;
            if (change.IsIndirect())
                ++indirectCount;
            else
                ++directCount;
            }
        }
    };

//---------------------------------------------------------------------------------------
// Behavior: Applying a changeset whose entries carry mixed direct/indirect flags
// to a database that is itself recording a session preserves the per-change
// classification. The source change explicitly flagged indirect (via
// ChangeTracker::Mode::Indirect) must be recorded as indirect in the target's
// session, while the direct source change must be recorded as direct.
//
// This is the core guarantee added by the fix. Before the fix BOTH recorded
// changes would have been flagged direct, because the target session was in
// Direct mode and the source's per-change indirect flag was ignored during apply.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SessionApplyIndirectTests, MixedDirectAndIndirect_FlagsPreservedThroughApply)
    {
    // Build a baseline Db with a single row, then clone it so source and target
    // start from the same "before" state (apply will see no conflicts).
    Db* source = CreateDb("indirect_mixed_source.db");
    ASSERT_NE(nullptr, source);
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql("CREATE TABLE t1(id INTEGER PRIMARY KEY, val TEXT)"));
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql("INSERT INTO t1(id,val) VALUES(1,'one')"));
    ASSERT_EQ(BE_SQLITE_OK, source->SaveChanges());
    source->CloseDb();
    ASSERT_EQ(BeFileNameStatus::Success, CloneFile("indirect_mixed_source.db", "indirect_mixed_target.db"));
    source = OpenDb("indirect_mixed_source.db");
    ASSERT_NE(nullptr, source);

    // Record a changeset on the source that contains one DIRECT change (insert
    // id=2) and one INDIRECT change (update id=1). The ChangeTracker mode drives
    // the per-change flag that is stored in the changeset.
    IndirectTestChangeTracker srcTracker(*source);
    srcTracker.EnableTracking(true);

    srcTracker.SetMode(ChangeTracker::Mode::Direct);
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql("INSERT INTO t1(id,val) VALUES(2,'two')"));

    srcTracker.SetMode(ChangeTracker::Mode::Indirect);
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql("UPDATE t1 SET val='one-indirect' WHERE id=1"));

    IndirectTestChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(srcTracker));
    ASSERT_EQ(BE_SQLITE_OK, source->SaveChanges());

    // Sanity check the source changeset: the insert is direct, the update is indirect.
    int directInserts = 0, indirectInserts = 0, directUpdates = 0, indirectUpdates = 0;
    CountChanges(cs, "t1", DbOpcode::Insert, directInserts, indirectInserts);
    CountChanges(cs, "t1", DbOpcode::Update, directUpdates, indirectUpdates);
    ASSERT_EQ(1, directInserts) << "insert made in Direct mode must be flagged direct in the source changeset";
    ASSERT_EQ(0, indirectInserts);
    ASSERT_EQ(1, indirectUpdates) << "update made in Indirect mode must be flagged indirect in the source changeset";
    ASSERT_EQ(0, directUpdates);

    // Open the target (clone of the baseline) and start recording a session on
    // it in the DEFAULT (Direct) mode. The target has not had SetMode called, so
    // any recorded change would, before the fix, be classified purely by this
    // Direct mode.
    Db* target = OpenDb("indirect_mixed_target.db");
    ASSERT_NE(nullptr, target);
    IndirectTestChangeTracker tgtTracker(*target);
    tgtTracker.EnableTracking(true);
    ASSERT_EQ(ChangeTracker::Mode::Direct, tgtTracker.GetMode());

    // Apply the source changeset to the target. This writes both rows, which the
    // target's active session records through the pre-update hook.
    ASSERT_EQ(BE_SQLITE_OK, cs.ApplyChanges(*target));

    // Capture what the target's session recorded and verify the flags survived.
    IndirectTestChangeSet recorded;
    ASSERT_EQ(BE_SQLITE_OK, recorded.FromChangeTrack(tgtTracker));

    int recDirectInserts = 0, recIndirectInserts = 0, recDirectUpdates = 0, recIndirectUpdates = 0;
    CountChanges(recorded, "t1", DbOpcode::Insert, recDirectInserts, recIndirectInserts);
    CountChanges(recorded, "t1", DbOpcode::Update, recDirectUpdates, recIndirectUpdates);

    // The direct source change stays direct after apply...
    EXPECT_EQ(1, recDirectInserts) << "a direct source change must be recorded direct by the target session";
    EXPECT_EQ(0, recIndirectInserts);
    // ...and the indirect source change stays indirect after apply. This is the
    // assertion that fails without the fix (it would be recorded as direct).
    EXPECT_EQ(1, recIndirectUpdates) << "an indirect source change must be recorded indirect by the target session";
    EXPECT_EQ(0, recDirectUpdates);

    ASSERT_EQ(BE_SQLITE_OK, target->SaveChanges());
    }

//---------------------------------------------------------------------------------------
// Behavior: Reproduction of the real-world #2346 scenario. On the source, a
// single DIRECT edit to a "pond" row fires a trigger that touches a "site" row;
// the trigger-driven site change is recorded as INDIRECT in the source
// changeset. When that changeset is applied to another briefcase that is
// recording its own session, the site change must remain INDIRECT (so the site
// is not falsely reported as directly modified), while the pond change remains
// DIRECT.
//
// Note: SQLite applies changeset row-changes directly and does not re-fire
// triggers during apply, so the target intentionally has no trigger; both rows
// are written (and recorded) purely from the changeset, exercising flag
// propagation rather than re-running the trigger.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SessionApplyIndirectTests, TriggerGeneratedIndirectChange_StaysIndirectThroughApply)
    {
    // Source has a trigger: editing a pond indirectly bumps the site counter.
    Db* source = CreateDb("indirect_trigger_source.db");
    ASSERT_NE(nullptr, source);
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql("CREATE TABLE site(id INTEGER PRIMARY KEY, edits INTEGER)"));
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql("CREATE TABLE pond(id INTEGER PRIMARY KEY, name TEXT)"));
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql(
        "CREATE TRIGGER pond_touches_site AFTER UPDATE ON pond BEGIN "
        "UPDATE site SET edits = edits + 1 WHERE id = 1; END"));
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql("INSERT INTO site(id,edits) VALUES(1,0)"));
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql("INSERT INTO pond(id,name) VALUES(1,'pond-a')"));
    ASSERT_EQ(BE_SQLITE_OK, source->SaveChanges());
    source->CloseDb();

    // Target starts from the same baseline but WITHOUT the trigger, so applying
    // the changeset writes the site row directly (no re-trigger).
    ASSERT_EQ(BeFileNameStatus::Success, CloneFile("indirect_trigger_source.db", "indirect_trigger_target.db"));
    source = OpenDb("indirect_trigger_source.db");
    ASSERT_NE(nullptr, source);

    // Record ONE direct edit to the pond. The trigger fires and records the site
    // row change as indirect in the same changeset.
    IndirectTestChangeTracker srcTracker(*source);
    srcTracker.EnableTracking(true);
    // Default Direct mode: the user's pond edit is direct; trigger side effects
    // are automatically flagged indirect by the session module.
    ASSERT_EQ(BE_SQLITE_OK, source->ExecuteSql("UPDATE pond SET name='pond-a-edited' WHERE id=1"));

    IndirectTestChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(srcTracker));
    ASSERT_EQ(BE_SQLITE_OK, source->SaveChanges());

    // Sanity: pond update is direct, site update (from the trigger) is indirect.
    int pondDirect = 0, pondIndirect = 0, siteDirect = 0, siteIndirect = 0;
    CountChanges(cs, "pond", DbOpcode::Update, pondDirect, pondIndirect);
    CountChanges(cs, "site", DbOpcode::Update, siteDirect, siteIndirect);
    ASSERT_EQ(1, pondDirect) << "the user's pond edit must be flagged direct";
    ASSERT_EQ(0, pondIndirect);
    ASSERT_EQ(1, siteIndirect) << "the trigger-driven site change must be flagged indirect";
    ASSERT_EQ(0, siteDirect);

    // Apply to the target briefcase while it records its own session (Direct mode).
    Db* target = OpenDb("indirect_trigger_target.db");
    ASSERT_NE(nullptr, target);
    // Drop the trigger on the target so apply does not double-touch the site row.
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("DROP TRIGGER pond_touches_site"));
    ASSERT_EQ(BE_SQLITE_OK, target->SaveChanges());

    IndirectTestChangeTracker tgtTracker(*target);
    tgtTracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_OK, cs.ApplyChanges(*target));

    IndirectTestChangeSet recorded;
    ASSERT_EQ(BE_SQLITE_OK, recorded.FromChangeTrack(tgtTracker));

    int recPondDirect = 0, recPondIndirect = 0, recSiteDirect = 0, recSiteIndirect = 0;
    CountChanges(recorded, "pond", DbOpcode::Update, recPondDirect, recPondIndirect);
    CountChanges(recorded, "site", DbOpcode::Update, recSiteDirect, recSiteIndirect);

    // The pond edit remains direct after apply...
    EXPECT_EQ(1, recPondDirect) << "pond edit must be recorded direct on the target";
    EXPECT_EQ(0, recPondIndirect);
    // ...and, crucially, the site change remains indirect after apply, so the
    // site is not falsely reported as directly modified (the #2346 regression).
    EXPECT_EQ(1, recSiteIndirect) << "trigger-driven site change must stay indirect on the target (issue #2346)";
    EXPECT_EQ(0, recSiteDirect);

    ASSERT_EQ(BE_SQLITE_OK, target->SaveChanges());
    }
