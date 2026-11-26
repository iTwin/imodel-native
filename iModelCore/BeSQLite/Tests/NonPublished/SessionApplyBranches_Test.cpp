/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//=======================================================================================
// Branch-coverage tests for the SQLite session extension's changeset "apply" and
// conflict-handling algorithm (ext/session/sqlite3session.c in iTwin/sqlite), exercised
// through the BeSQLite ChangeSet / ApplyChangesArgs wrappers.
//
// The apply pipeline is:
//   sessionChangesetApply()            -- main per-change loop, schema/filter handling
//     -> sessionApplyOneWithRetry()    -- REPLACE retry orchestration (bRetry / bReplace)
//        -> sessionApplyOneOp()        -- runs the DELETE / UPDATE / INSERT statement
//           -> sessionConflictHandler()-- maps the failure to a ConflictCause and calls back
//     -> sessionRetryConstraints()     -- deferred-constraint buffer (3-step resolution)
//
// ConflictCause mapping (see sessionConflictHandler). The cause depends not just on the
// SQLite result but on whether the caller asked sessionConflictHandler to seek the row
// with the change's primary key (the pbReplace/pbRetry argument):
//
//   DELETE/UPDATE, 0 rows changed, PK row present   -> Data       (seek performed, row found)
//   DELETE/UPDATE, 0 rows changed, PK row absent    -> NotFound   (seek performed, no row)
//   INSERT, SQLITE_CONSTRAINT, PK row present        -> Conflict   (INSERT seeks by new PK)
//   INSERT, SQLITE_CONSTRAINT, PK row absent         -> Constraint (INSERT seeks, no PK row)
//   UPDATE/DELETE, SQLITE_CONSTRAINT                 -> Constraint (no seek; always Constraint,
//                                                                   surfaced via the deferred
//                                                                   constraints buffer)
//   deferred FK count > 0 at end of apply            -> ForeignKey
//
// Note: UPDATE/DELETE constraint violations are first parked in the deferred-constraint
// buffer and only reach the handler (as Constraint) in the final sessionRetryConstraints
// pass, after the automatic delete/reinsert resolution has been attempted.
//
// Each test documents the branch it targets and the behavior asserted alongside it.
//=======================================================================================
#include "BeSQLiteNonPublishedTests.h"
#include <BeSQLite/ChangeSet.h>
#include <vector>

USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// Change tracker used to capture changesets from the "producer" database.
// @bsiclass
//=======================================================================================
struct BranchChangeTracker : ChangeTracker
    {
    explicit BranchChangeTracker(DbR db) { SetDb(&db); }
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Commit; }
    };

//=======================================================================================
// ChangeSet whose default (no-args) conflict handler asserts. Tests that expect
// conflicts drive resolution through ApplyChangesArgs::SetConflictHandler instead.
// @bsiclass
//=======================================================================================
struct BranchChangeSet : ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause cause, Changes::Change iter) override
        {
        BeAssert(false && "unexpected conflict");
        return ConflictResolution::Abort;
        }
    };

//=======================================================================================
// Records every conflict callback invocation so a test can assert exactly which
// ConflictCause values the session extension produced.
//=======================================================================================
struct ConflictRecorder
    {
    struct Entry { ChangeSet::ConflictCause cause; DbOpcode op; Utf8String table; };
    std::vector<Entry> m_entries;
    ChangeSet::ConflictResolution m_resolution = ChangeSet::ConflictResolution::Skip;

    ChangeSet::ConflictResolution Handle(ChangeSet::ConflictCause cause, Changes::Change change)
        {
        m_entries.push_back({cause, change.GetOpcode(), change.GetTableName()});
        return m_resolution;
        }
    int Count() const { return (int)m_entries.size(); }
    int CountOf(ChangeSet::ConflictCause cause) const
        {
        int n = 0;
        for (auto const& e : m_entries) if (e.cause == cause) ++n;
        return n;
        }
    };

//=======================================================================================
// @bsistruct
//=======================================================================================
struct SessionApplyBranchesTests : public ::testing::Test
    {
    // The fixture owns every Db it opens. GoogleTest destroys test-local variables
    // *before* TearDown() runs, so ownership has to live here to guarantee that each
    // writable connection has its pending transaction cleared before it is destroyed
    // (otherwise ~Db() asserts on the auto-commit-with-uncommitted-changes guard).
    std::vector<std::unique_ptr<Db>> m_dbs;

    SessionApplyBranchesTests()
        {
        BeFileName tempDir;
        BeTest::GetHost().GetTempDir(tempDir);
        BeSQLiteLib::Initialize(tempDir);
        }

    void TearDown() override
        {
        for (auto& db : m_dbs)
            {
            if (db != nullptr && db->IsDbOpen() && !db->IsReadonly())
                db->AbandonChanges();
            }
        m_dbs.clear();
        }

    static BeFileName PathFor(Utf8CP name)
        {
        BeFileName p;
        BeTest::GetHost().GetOutputRoot(p);
        p.AppendUtf8(name);
        return p;
        }

    Db* CreateDb(Utf8CP name)
        {
        BeFileName p = PathFor(name);
        if (p.DoesPathExist())
            p.BeDeleteFile();
        std::unique_ptr<Db> db(new Db());
        if (BE_SQLITE_OK != db->CreateNewDb(p))
            return nullptr;
        db->SaveChanges();
        m_dbs.push_back(std::move(db));
        return m_dbs.back().get();
        }

    Db* OpenDb(Utf8CP name)
        {
        BeFileName p = PathFor(name);
        if (!p.DoesPathExist())
            return nullptr;
        std::unique_ptr<Db> db(new Db());
        if (BE_SQLITE_OK != db->OpenBeSQLiteDb(p, Db::OpenParams(Db::OpenMode::ReadWrite)))
            return nullptr;
        m_dbs.push_back(std::move(db));
        return m_dbs.back().get();
        }

    static BeFileNameStatus CloneDb(Utf8CP from, Utf8CP to)
        {
        BeFileName src = PathFor(from);
        BeFileName dst = PathFor(to);
        if (dst.DoesPathExist())
            dst.BeDeleteFile();
        return BeFileName::BeCopyFile(src, dst);
        }

    // Run "task" on "db" while tracking, and return the captured changeset.
    static std::unique_ptr<ChangeSet> Capture(DbR db, std::function<bool(DbR)> task)
        {
        BranchChangeTracker tracker(db);
        tracker.EnableTracking(true);
        if (!task(db))
            return nullptr;
        if (!tracker.HasChanges())
            return nullptr;
        std::unique_ptr<ChangeSet> cs(new BranchChangeSet());
        if (BE_SQLITE_OK != cs->FromChangeTrack(tracker))
            return nullptr;
        return cs;
        }

    static Utf8String GetText(DbR db, Utf8CP sql)
        {
        auto stmt = db.GetCachedStatement(sql);
        if (BE_SQLITE_ROW != stmt->Step())
            return Utf8String();
        return stmt->IsColumnNull(0) ? Utf8String() : Utf8String(stmt->GetValueText(0));
        }

    static int GetInt(DbR db, Utf8CP sql)
        {
        auto stmt = db.GetCachedStatement(sql);
        if (BE_SQLITE_ROW != stmt->Step())
            return -1;
        return stmt->GetValueInt(0);
        }

    static bool RowExists(DbR db, Utf8CP sql) { return GetInt(db, sql) > 0; }

    // Create a base db with the standard t1 schema seeded with rows 1,2,3, then clone
    // it into a "producer" and a "target" db. Returns both open R/W.
    void SetupStd(Db*& producer, Db*& target, Utf8CP extraSchema = nullptr)
        {
        auto base = CreateDb("branch_base.db");
        ASSERT_TRUE(base != nullptr);
        ASSERT_EQ(BE_SQLITE_OK, base->ExecuteSql("CREATE TABLE t1(id INTEGER PRIMARY KEY, val TEXT, num INTEGER)"));
        if (extraSchema)
            ASSERT_EQ(BE_SQLITE_OK, base->ExecuteSql(extraSchema));
        ASSERT_EQ(BE_SQLITE_OK, base->ExecuteSql("INSERT INTO t1 VALUES(1,'one',10),(2,'two',20),(3,'three',30)"));
        base->SaveChanges();
        base->CloseDb();

        ASSERT_EQ(BeFileNameStatus::Success, CloneDb("branch_base.db", "branch_producer.db"));
        ASSERT_EQ(BeFileNameStatus::Success, CloneDb("branch_base.db", "branch_target.db"));
        producer = OpenDb("branch_producer.db");
        target = OpenDb("branch_target.db");
        ASSERT_TRUE(producer != nullptr);
        ASSERT_TRUE(target != nullptr);
        }
    };

//=======================================================================================
// INSERT branches (sessionApplyOneOp, op == SQLITE_INSERT)
//=======================================================================================

// Branch: INSERT with no conflicting row -> plain INSERT succeeds, conflict handler
// is never invoked.
TEST_F(SessionApplyBranchesTests, Insert_Success)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("INSERT INTO t1 VALUES(10,'ten',100)");
        });
    ASSERT_TRUE(cs != nullptr);

    ConflictRecorder rec;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: apply succeeds, the new row is present, and no conflict is reported.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(0, rec.Count());
    EXPECT_TRUE(RowExists(*target, "SELECT count(*) FROM t1 WHERE id=10 AND val='ten'"));
    }

// Branch: INSERT hits a PRIMARY KEY conflict and a row with that PK already exists ->
// ConflictCause::Conflict. Handler returns Skip -> the incoming INSERT is dropped and
// the pre-existing target row is left untouched.
TEST_F(SessionApplyBranchesTests, Insert_PkConflict_Skip)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("INSERT INTO t1 VALUES(10,'ten',100)");
        });
    ASSERT_TRUE(cs != nullptr);

    // Diverge target: it already has a row with the same PK but a different value.
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("INSERT INTO t1 VALUES(10,'other',999)"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: exactly one Conflict callback, apply returns OK, target row unchanged.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::Conflict));
    EXPECT_STREQ("other", GetText(*target, "SELECT val FROM t1 WHERE id=10").c_str());
    }

// Branch: INSERT PRIMARY KEY conflict, handler returns Replace -> sessionApplyOneWithRetry
// takes the bReplace path: it opens a "replace_op" savepoint, DELETEs the conflicting row,
// re-runs the INSERT, then releases the savepoint.
TEST_F(SessionApplyBranchesTests, Insert_PkConflict_Replace)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("INSERT INTO t1 VALUES(10,'ten',100)");
        });
    ASSERT_TRUE(cs != nullptr);

    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("INSERT INTO t1 VALUES(10,'other',999)"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Replace;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: one Conflict callback, apply returns OK, the conflicting row is replaced
    // by the changeset's values.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::Conflict));
    EXPECT_STREQ("ten", GetText(*target, "SELECT val FROM t1 WHERE id=10").c_str());
    EXPECT_EQ(100, GetInt(*target, "SELECT num FROM t1 WHERE id=10"));
    }

// Branch: INSERT violates a UNIQUE constraint on a NON-PK column while NO row with the
// changeset's PK exists -> the "no PK row found" path reports ConflictCause::Constraint.
// Returning Replace here is illegal (there is no conflicting PK row to replace), so
// sessionConflictHandler forces SQLITE_MISUSE.
TEST_F(SessionApplyBranchesTests, Insert_UniqueNonPk_ReplaceIsMisuse)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target, "CREATE UNIQUE INDEX uidx_t1_val ON t1(val)");

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("INSERT INTO t1 VALUES(10,'ten',100)");
        });
    ASSERT_TRUE(cs != nullptr);

    // Target already has 'ten' under a different PK, so the incoming INSERT collides on
    // the UNIQUE(val) index but not on the PK.
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("INSERT INTO t1 VALUES(99,'ten',1)"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Replace;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: cause reported is Constraint, and asking for Replace yields BE_SQLITE_MISUSE.
    DbResult r = cs->ApplyChanges(*target, args);
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::Constraint));
    EXPECT_EQ(BE_SQLITE_MISUSE, r);
    }

//=======================================================================================
// DELETE branches (sessionApplyOneOp, op == SQLITE_DELETE)
//=======================================================================================

// Branch: DELETE of a row that exists with matching values -> deletes cleanly, no conflict.
TEST_F(SessionApplyBranchesTests, Delete_Success)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("DELETE FROM t1 WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ConflictRecorder rec;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: row 2 removed, no conflicts.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(0, rec.Count());
    EXPECT_FALSE(RowExists(*target, "SELECT count(*) FROM t1 WHERE id=2"));
    }

// Branch: DELETE where NO row with the PK exists in the target -> ConflictCause::NotFound.
// Handler Skip -> change omitted.
TEST_F(SessionApplyBranchesTests, Delete_NotFound_Skip)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("DELETE FROM t1 WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    // Remove the row from the target first so the incoming DELETE finds nothing.
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("DELETE FROM t1 WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: one NotFound callback, apply OK.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::NotFound));
    }

// Branch: DELETE where the PK row exists but a non-PK column no longer matches the
// changeset's "old" values -> ConflictCause::Data. Handler Skip -> the target row is kept.
TEST_F(SessionApplyBranchesTests, Delete_Data_Skip)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("DELETE FROM t1 WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    // Diverge the non-PK data on the target row.
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("UPDATE t1 SET val='changed' WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: one Data callback, row kept.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::Data));
    EXPECT_TRUE(RowExists(*target, "SELECT count(*) FROM t1 WHERE id=2"));
    }

// Branch: DELETE Data conflict, handler returns Replace -> sessionApplyOneWithRetry takes
// the bRetry path and re-runs the op ignoring the DATA mismatch, deleting by PK.
TEST_F(SessionApplyBranchesTests, Delete_Data_Replace)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("DELETE FROM t1 WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("UPDATE t1 SET val='changed' WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Replace;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: one Data callback, and the row is deleted despite the mismatch.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::Data));
    EXPECT_FALSE(RowExists(*target, "SELECT count(*) FROM t1 WHERE id=2"));
    }

// Branch: DELETE NotFound, handler illegally returns Replace. There is no PK row to
// replace, so sessionConflictHandler forces SQLITE_MISUSE.
TEST_F(SessionApplyBranchesTests, Delete_NotFound_ReplaceIsMisuse)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("DELETE FROM t1 WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("DELETE FROM t1 WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Replace;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: NotFound reported, Replace turns into BE_SQLITE_MISUSE.
    DbResult r = cs->ApplyChanges(*target, args);
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::NotFound));
    EXPECT_EQ(BE_SQLITE_MISUSE, r);
    }

//=======================================================================================
// UPDATE branches (sessionApplyOneOp, op == SQLITE_UPDATE)
//=======================================================================================

// Branch: UPDATE that matches an existing row -> updates cleanly, no conflict.
TEST_F(SessionApplyBranchesTests, Update_Success)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("UPDATE t1 SET val='TWO' WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ConflictRecorder rec;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: value updated, no conflict.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(0, rec.Count());
    EXPECT_STREQ("TWO", GetText(*target, "SELECT val FROM t1 WHERE id=2").c_str());
    }

// Branch: UPDATE that only changes a subset of columns. Exercises sessionUpdateFind's
// per-change column-mask bookkeeping: only bound columns are written, others untouched.
TEST_F(SessionApplyBranchesTests, Update_PartialColumns_Success)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    // Only "num" changes; "val" is not part of the update.
    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("UPDATE t1 SET num=222 WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ConflictRecorder rec;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: num updated, val preserved, no conflict.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(0, rec.Count());
    EXPECT_EQ(222, GetInt(*target, "SELECT num FROM t1 WHERE id=2"));
    EXPECT_STREQ("two", GetText(*target, "SELECT val FROM t1 WHERE id=2").c_str());
    }

// Branch: UPDATE where no row with the PK exists -> ConflictCause::NotFound.
TEST_F(SessionApplyBranchesTests, Update_NotFound_Skip)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("UPDATE t1 SET val='TWO' WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("DELETE FROM t1 WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: one NotFound callback, apply OK.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::NotFound));
    }

// Branch: UPDATE where the PK row exists but a non-PK "old" value no longer matches ->
// ConflictCause::Data. Handler Skip -> update omitted.
TEST_F(SessionApplyBranchesTests, Update_Data_Skip)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("UPDATE t1 SET val='TWO' WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("UPDATE t1 SET val='divergent' WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: one Data callback, target keeps its divergent value.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::Data));
    EXPECT_STREQ("divergent", GetText(*target, "SELECT val FROM t1 WHERE id=2").c_str());
    }

// Branch: UPDATE Data conflict, handler Replace -> bRetry path re-applies the update by
// PK, overwriting the divergent value.
TEST_F(SessionApplyBranchesTests, Update_Data_Replace)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("UPDATE t1 SET val='TWO' WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("UPDATE t1 SET val='divergent' WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Replace;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: one Data callback, the update is forced through.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::Data));
    EXPECT_STREQ("TWO", GetText(*target, "SELECT val FROM t1 WHERE id=2").c_str());
    }

// Branch: UPDATE whose new value violates a UNIQUE (non-PK) constraint against a
// *different* existing row. Unlike an INSERT PK collision, sessionApplyOneOp passes
// pbReplace==0 for an UPDATE constraint failure, so sessionConflictHandler never seeks a
// conflicting PK row and therefore reports ConflictCause::Constraint (not Conflict). The
// change first lands in the deferred-constraint buffer and only surfaces to the handler
// in the final sessionRetryConstraints pass. Handler Skip -> update omitted.
TEST_F(SessionApplyBranchesTests, Update_UniqueConflict_Skip)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target, "CREATE UNIQUE INDEX uidx_t1_val ON t1(val)");

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("UPDATE t1 SET val='newval' WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    // Target has another row already holding 'newval'.
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("INSERT INTO t1 VALUES(4,'newval',40)"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: exactly one Constraint callback, row 2 keeps its original value.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::Constraint));
    EXPECT_EQ(0, rec.CountOf(ChangeSet::ConflictCause::Conflict));
    EXPECT_STREQ("two", GetText(*target, "SELECT val FROM t1 WHERE id=2").c_str());
    }

//=======================================================================================
// Conflict-resolution return values (sessionConflictHandler switch)
//=======================================================================================

// Branch: any conflict handler returning Abort -> sessionConflictHandler returns
// SQLITE_ABORT, which surfaces as BE_SQLITE_ABORT from ApplyChanges.
TEST_F(SessionApplyBranchesTests, Conflict_Abort)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("DELETE FROM t1 WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("DELETE FROM t1 WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Abort;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: BE_SQLITE_ABORT is returned.
    EXPECT_EQ(BE_SQLITE_ABORT, cs->ApplyChanges(*target, args));
    }

// Branch: ApplyChangesArgs::SetAbortOnAnyConflict short-circuits the callback and returns
// Abort for the first conflict, without ever invoking a user handler.
TEST_F(SessionApplyBranchesTests, Conflict_AbortOnAnyConflictFlag)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("DELETE FROM t1 WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("DELETE FROM t1 WHERE id=2"));
    target->SaveChanges();

    bool handlerCalled = false;
    ApplyChangesArgs args;
    args.SetAbortOnAnyConflict(true);
    args.SetConflictHandler([&](ChangeSet::ConflictCause, Changes::Change){ handlerCalled = true; return ChangeSet::ConflictResolution::Skip; });

    // Expected: BE_SQLITE_ABORT and the user handler is never called.
    EXPECT_EQ(BE_SQLITE_ABORT, cs->ApplyChanges(*target, args));
    EXPECT_FALSE(handlerCalled);
    }

//=======================================================================================
// Deferred-constraint retry (sessionRetryConstraints) + the extended-result-code fix
//=======================================================================================

// Branch: two UPDATEs that swap a unique-index value. Each UPDATE initially fails with
// SQLITE_CONSTRAINT_UNIQUE and is deferred to the constraints buffer; sessionRetryConstraints
// step 2 resolves them (delete one, apply the other, reinsert). This exercises the
// (rc & 0xff) == SQLITE_CONSTRAINT masking fix for extended result codes.
TEST_F(SessionApplyBranchesTests, DeferredConstraint_SwapUnique_ResolvesWithoutConflict)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target, "CREATE UNIQUE INDEX uidx_t1_val ON t1(val)");

    // Net effect: row1.val 'one'->'two', row2.val 'two'->'one' (a circular swap).
    auto cs = Capture(*producer, [](DbR db) {
        if (BE_SQLITE_OK != db.ExecuteSql("UPDATE t1 SET val='__tmp__' WHERE id=1")) return false;
        if (BE_SQLITE_OK != db.ExecuteSql("UPDATE t1 SET val='one' WHERE id=2")) return false;
        if (BE_SQLITE_OK != db.ExecuteSql("UPDATE t1 SET val='two' WHERE id=1")) return false;
        return true;
        });
    ASSERT_TRUE(cs != nullptr);

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Abort;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: the deferred-constraint machinery resolves the swap with ZERO conflict
    // callbacks and the values are swapped in the target.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(0, rec.Count());
    EXPECT_STREQ("two", GetText(*target, "SELECT val FROM t1 WHERE id=1").c_str());
    EXPECT_STREQ("one", GetText(*target, "SELECT val FROM t1 WHERE id=2").c_str());
    }

// Branch: the same unique swap but with SetNoUpdateLoop(true). This disables
// sessionRetryConstraints step 2 (the UPDATE delete/insert loop), so the circular
// constraint can no longer be auto-resolved and the conflict handler is invoked with
// ConflictCause::Constraint in the final pass.
TEST_F(SessionApplyBranchesTests, DeferredConstraint_NoUpdateLoop_InvokesConstraintHandler)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target, "CREATE UNIQUE INDEX uidx_t1_val ON t1(val)");

    auto cs = Capture(*producer, [](DbR db) {
        if (BE_SQLITE_OK != db.ExecuteSql("UPDATE t1 SET val='__tmp__' WHERE id=1")) return false;
        if (BE_SQLITE_OK != db.ExecuteSql("UPDATE t1 SET val='one' WHERE id=2")) return false;
        if (BE_SQLITE_OK != db.ExecuteSql("UPDATE t1 SET val='two' WHERE id=1")) return false;
        return true;
        });
    ASSERT_TRUE(cs != nullptr);

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip; // omit the unresolved change(s)
    ApplyChangesArgs args;
    args.SetNoUpdateLoop(true);
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: with step 2 disabled, at least one Constraint conflict is reported.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_GE(rec.CountOf(ChangeSet::ConflictCause::Constraint), 1);
    }

//=======================================================================================
// Apply flags and filters (sessionChangesetApply)
//=======================================================================================

// Branch: SetIgnoreNoop(true) suppresses a "no-op" UPDATE (the target row already holds
// the changeset's new values). The conflict handler is NOT invoked even though the old
// values would otherwise mismatch (a Data conflict).
TEST_F(SessionApplyBranchesTests, IgnoreNoop_SuppressesNoopUpdate)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("UPDATE t1 SET val='TWO' WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    // Target already has the destination value -> applying is a no-op.
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("UPDATE t1 SET val='TWO' WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip;
    ApplyChangesArgs args;
    args.SetIgnoreNoop(true);
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: no conflict reported, value stays 'TWO'.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(0, rec.Count());
    EXPECT_STREQ("TWO", GetText(*target, "SELECT val FROM t1 WHERE id=2").c_str());
    }

// Branch: without IgnoreNoop the same "already at destination" situation is reported as a
// Data conflict (old values mismatch, 0 rows changed). This contrasts with the test above.
TEST_F(SessionApplyBranchesTests, IgnoreNoop_DisabledReportsDataConflict)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("UPDATE t1 SET val='TWO' WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("UPDATE t1 SET val='TWO' WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip;
    ApplyChangesArgs args; // IgnoreNoop defaults to false
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: exactly one Data conflict reported.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::Data));
    }

// Branch: SetInvert(true) applies the reverse of each change. A captured DELETE is
// applied as an INSERT, restoring a row that was removed from the target.
TEST_F(SessionApplyBranchesTests, Invert_DeleteBecomesInsert)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("DELETE FROM t1 WHERE id=2");
        });
    ASSERT_TRUE(cs != nullptr);

    // Remove row 2 from the target; the inverted DELETE (=INSERT) should bring it back.
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("DELETE FROM t1 WHERE id=2"));
    target->SaveChanges();

    ConflictRecorder rec;
    ApplyChangesArgs args;
    args.SetInvert(true);
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: row 2 reinserted with its original values, no conflict.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(0, rec.Count());
    EXPECT_STREQ("two", GetText(*target, "SELECT val FROM t1 WHERE id=2").c_str());
    }

// Branch: SetFilterChange (the apply_v3 xFilterIter hook) can skip changes for a whole
// table. Here changes to t2 are filtered out while changes to t1 are applied.
TEST_F(SessionApplyBranchesTests, FilterChange_SkipsTable)
    {
    Db *producer = nullptr, *target = nullptr;
    SetupStd(producer, target, "CREATE TABLE t2(id INTEGER PRIMARY KEY, v TEXT)");

    auto cs = Capture(*producer, [](DbR db) {
        if (BE_SQLITE_OK != db.ExecuteSql("INSERT INTO t1 VALUES(10,'ten',100)")) return false;
        if (BE_SQLITE_OK != db.ExecuteSql("INSERT INTO t2 VALUES(1,'keepout')")) return false;
        return true;
        });
    ASSERT_TRUE(cs != nullptr);

    ApplyChangesArgs args;
    args.SetFilterChange([](Changes::Change const& ch) {
        return ch.GetTableName().Equals("t2")
            ? ChangeStream::FilterChangeAction::Skip
            : ChangeStream::FilterChangeAction::Accept;
        });

    // Expected: t1 insert applied, t2 insert skipped.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_TRUE(RowExists(*target, "SELECT count(*) FROM t1 WHERE id=10"));
    EXPECT_FALSE(RowExists(*target, "SELECT count(*) FROM t2 WHERE id=1"));
    }

// Branch: schema mismatch. The target is missing the table referenced by the changeset.
// sessionChangesetApply logs a schema mismatch and skips those changes; ApplyChanges
// still returns BE_SQLITE_OK and the conflict handler is not invoked.
TEST_F(SessionApplyBranchesTests, SchemaMismatch_MissingTableIsSkipped)
    {
    // Producer has an extra table t2; target does not.
    auto producer = CreateDb("branch_producer.db");
    ASSERT_TRUE(producer != nullptr);
    ASSERT_EQ(BE_SQLITE_OK, producer->ExecuteSql("CREATE TABLE t2(id INTEGER PRIMARY KEY, v TEXT)"));
    producer->SaveChanges();

    auto target = CreateDb("branch_target.db");
    ASSERT_TRUE(target != nullptr);
    target->SaveChanges();

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("INSERT INTO t2 VALUES(1,'x')");
        });
    ASSERT_TRUE(cs != nullptr);

    ConflictRecorder rec;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: apply returns OK, change skipped, no conflict callback.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(0, rec.Count());
    }

// Branch: sqlite_stat1 special handling (bStat1). ANALYZE-generated stat rows are applied
// via the stat1-specific INSERT/SELECT path rather than the normal PK machinery.
TEST_F(SessionApplyBranchesTests, Stat1_ApplyGeneratedStats)
    {
    auto producer = CreateDb("branch_producer.db");
    ASSERT_TRUE(producer != nullptr);
    ASSERT_EQ(BE_SQLITE_OK, producer->ExecuteSql("CREATE TABLE s(id INTEGER PRIMARY KEY, v TEXT)"));
    ASSERT_EQ(BE_SQLITE_OK, producer->ExecuteSql("CREATE INDEX idx_s_v ON s(v)"));
    ASSERT_EQ(BE_SQLITE_OK, producer->ExecuteSql("INSERT INTO s VALUES(1,'a'),(2,'b'),(3,'c')"));
    producer->SaveChanges();
    producer->CloseDb();

    ASSERT_EQ(BeFileNameStatus::Success, CloneDb("branch_producer.db", "branch_target.db"));
    producer = OpenDb("branch_producer.db");
    auto target = OpenDb("branch_target.db");
    ASSERT_TRUE(producer != nullptr);
    ASSERT_TRUE(target != nullptr);

    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("ANALYZE");
        });
    ASSERT_TRUE(cs != nullptr);
    producer->SaveChanges();

    int expected = GetInt(*producer, "SELECT count(*) FROM sqlite_stat1");
    ASSERT_GT(expected, 0);

    ConflictRecorder rec;
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: stat rows applied to target with no conflict.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(0, rec.Count());
    EXPECT_EQ(expected, GetInt(*target, "SELECT count(*) FROM sqlite_stat1"));
    }

// Branch: ForeignKey conflict. A changeset that inserts a child row referencing a parent
// that does not exist in the target leaves a deferred FK violation. At the end of the
// apply, sessionChangesetApply invokes the handler with ConflictCause::ForeignKey.
TEST_F(SessionApplyBranchesTests, ForeignKey_DanglingReferenceReported)
    {
    // Build producer with FK on; parent(1) exists so the child insert is legal there.
    auto producer = CreateDb("branch_producer.db");
    ASSERT_TRUE(producer != nullptr);
    ASSERT_EQ(BE_SQLITE_OK, producer->ExecuteSql("PRAGMA foreign_keys=ON"));
    ASSERT_EQ(BE_SQLITE_OK, producer->ExecuteSql("CREATE TABLE parent(id INTEGER PRIMARY KEY)"));
    ASSERT_EQ(BE_SQLITE_OK, producer->ExecuteSql("CREATE TABLE child(id INTEGER PRIMARY KEY, pid INTEGER REFERENCES parent(id))"));
    ASSERT_EQ(BE_SQLITE_OK, producer->ExecuteSql("INSERT INTO parent VALUES(1)"));
    producer->SaveChanges();

    // Target has the same schema but NO parent row -> child insert will dangle.
    auto target = CreateDb("branch_target.db");
    ASSERT_TRUE(target != nullptr);
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("PRAGMA foreign_keys=ON"));
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("CREATE TABLE parent(id INTEGER PRIMARY KEY)"));
    ASSERT_EQ(BE_SQLITE_OK, target->ExecuteSql("CREATE TABLE child(id INTEGER PRIMARY KEY, pid INTEGER REFERENCES parent(id))"));
    target->SaveChanges();

    // Capture only the child insert (parent(1) already existed before tracking).
    auto cs = Capture(*producer, [](DbR db) {
        return BE_SQLITE_OK == db.ExecuteSql("INSERT INTO child VALUES(100,1)");
        });
    ASSERT_TRUE(cs != nullptr);

    ConflictRecorder rec;
    rec.m_resolution = ChangeSet::ConflictResolution::Skip; // OMIT the FK violation
    ApplyChangesArgs args;
    args.SetConflictHandler([&](ChangeSet::ConflictCause c, Changes::Change ch){ return rec.Handle(c, ch); });

    // Expected: a ForeignKey conflict is reported. Returning Skip (OMIT) lets apply
    // complete with BE_SQLITE_OK.
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*target, args));
    EXPECT_EQ(1, rec.CountOf(ChangeSet::ConflictCause::ForeignKey));
    }
