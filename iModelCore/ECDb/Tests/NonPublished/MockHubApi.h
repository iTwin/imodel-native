/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbPublishedTests.h"
#include <ECDb/ECDbApi.h>
#include <re2/re2.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
using namespace BentleyApi::BeSQLite::EC::Tests;
using namespace BentleyApi::BeSQLite::EC;

struct ECDbChangeTracker;
struct ECDbHub;


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbChangeSet : public BeSQLite::ChangeSet {
    using Ptr = std::unique_ptr<ECDbChangeSet>;
    private:
        Utf8String m_operation;
        Utf8String m_ddl;
        bool m_hasSchemaChanges;
        int m_index;
        ECDb const* m_ecdb;
        bool m_ecChangesSupersedeBriefcase = false;
        BeSQLite::ChangeGroup m_supersedingRows;
        bool m_hasSupersedingRows = false;

    private:
        ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override;

    public:
        ECDbChangeSet(int index, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset)
            : m_operation(op), m_ddl(ddl), m_hasSchemaChanges(isSchemaChangeset || !Utf8String::IsNullOrEmpty(ddl)), m_index(index), m_ecdb(nullptr) {}
        int GetIndex() const { return m_index; }
        bool HasSchemaChanges() const { return m_hasSchemaChanges; }
        Utf8StringCR GetOperation() const { return m_operation; }
        Utf8StringCR GetDDL() const { return m_ddl; }
        void PrependDDL(Utf8StringCR ddl);
        bvector<Utf8String> GetDDLs() const;
        void SetECDb(ECDbCR ecdb) { m_ecdb = &ecdb; }
        ECDb const* GetECDb() const { return m_ecdb; }
        //! Work out, before replaying this as a local changeset, whether its ec_ rows supersede the
        //! ones the briefcase now holds. Mirrors LocalChangeSet::DetermineSchemaSyncPrecedence.
        void DetermineSchemaSyncPrecedence();
        //! Write the superseding rows an insert conflict had to leave alone. Call after ApplyChanges.
        DbResult ApplySupersedingRows(ECDbR db);
        Ptr Clone() const;
        void ToSQL(DbCR db, std::function<void(bool, std::string const&)> cb) const;
        static Ptr From(ECDbChangeTracker& tracker, Utf8CP comment);
        static Ptr Create(int index, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset);
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbChangeTracker : BeSQLite::ChangeTracker {
    using Ptr = std::unique_ptr<ECDbChangeTracker>;

private:
    ECDbR m_mdb;
    std::vector<ECDbChangeSet::Ptr> m_localChangesets;
    Utf8String m_carriedDdl;
    bset<std::string> m_tableToIgnore;

private:
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override;
    TrackChangesForTable _FilterTable(Utf8CP tableName) override {
        if (m_tableToIgnore.find(tableName) != m_tableToIgnore.end())
            return TrackChangesForTable::No;
        return TrackChangesForTable::Yes;
    }
public:
    ECDbChangeTracker(ECDbR db) : m_mdb(db) {
        AddRef();
        SetDb(&db);
        m_tableToIgnore.insert("ec_cache_ClassHasTables");
        m_tableToIgnore.insert("ec_cache_ClassHierarchy");
    }
    ECDbCR const& GetDb() const { return m_mdb; }
    BeGuid GetDbId() const;
    Utf8String GetDDL() const { return m_ddlChanges.ToString(); }
    ECDbChangeSet::Ptr MakeChangeset(bool deleteLocalChangesets, Utf8CP op);
    std::vector<ECDbChangeSet const*> GetLocalChangesets() const;
    std::vector<ECDbChangeSet::Ptr> TakeLocalChangesets();
    //! DDL cannot be replayed onto a file that already has it, so a rebased changeset is handed the text to ship.
    void CarryDdlIntoNextChangeset(Utf8StringCR ddl) { m_carriedDdl = ddl; }

    void ToSql(std::function<void(ECDbChangeSet const&, bool, std::string const&)> cb) const {
        for (auto&  cs: m_localChangesets) {
            cs->ToSQL(m_mdb, [&](bool isDDL, std::string const& sql) {
                cb(*cs, isDDL, sql);
            });
        }
    }
    Ptr Clone(ECDbR) const;
    static Ptr Create(ECDbR db) { return std::make_unique<ECDbChangeTracker>(db); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct TrackedECDb : ECDb {
    private:
        std::unique_ptr<ECDbChangeTracker> m_tracker;
        ECDbHub* m_hub;
        int m_changesetId = -1;
        bool m_replayingLocalChangesets = false;
        void SetupTracker(std::unique_ptr<ECDbChangeTracker> tracker = nullptr);
        DbResult RebaseOntoIncoming(std::vector<ECDbChangeSet*> const& incoming);
        virtual void _OnDbClose() override;
        virtual DbResult _OnDbCreated(CreateParams const& params) override;
        virtual DbResult _OnDbOpening() override;
        virtual bool _IsLevelWithTimeline() override;

    public:
        ECDbChangeTracker* GetTracker() { return m_tracker.get(); }
        void SetHub(ECDbHub& hub) { m_hub = &hub; }
        ECDbHub* GetHub() { return m_hub; }
        //! Stands in for TxnManager::HasPendingTxns(). True while local changesets are being replayed too:
        //! a rebase reinstates them one at a time, so the txns are pending throughout.
        bool HasLocalChangesets() const { return m_replayingLocalChangesets || (m_tracker != nullptr && !m_tracker->GetLocalChangesets().empty()); }
        //! True during the rebase phase that production runs through LocalChangeSet rather than
        //! ChangesetFileReader, which resolve some conflicts differently.
        bool IsReplayingLocalChangesets() const { return m_replayingLocalChangesets; }
        DbResult PullMergePush(Utf8CP comment);
        virtual ~TrackedECDb();
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbHub {
    private:
        std::vector<ECDbChangeSet::Ptr> m_changesets;
        BeGuid m_id;
        BeFileName m_basePath;
        BeFileName m_seedFile;
        BeBriefcaseId m_briefcaseid;

    private:
        DbResult FindSeedFile();
        BeFileName BuildECDbPath(Utf8CP name) const;
        BeBriefcaseId GetNextBriefcaseId() { return (m_briefcaseid = m_briefcaseid.GetNextBriefcaseId()); }

    public:
        ECDbHub();
        std::unique_ptr<TrackedECDb> CreateBriefcase();
        std::vector<ECDbChangeSet*> Query(int afterChangesetId = 1);
        int PushNewChangeset(ECDbChangeSet::Ptr changeset);
        //! Index of the newest changeset, or -1 while nothing has been pushed.
        int GetTipChangesetId() const { return (int)m_changesets.size() - 1; }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct InMemoryECDb final: public BentleyApi::BeSQLite::EC::ECDb {
    using Ptr = std::unique_ptr<InMemoryECDb>;

private:
    virtual void _OnDbClose() override;
    InMemoryECDb();

public:
    InMemoryECDb::Ptr CreateSnapshot(DbResult* outRc = nullptr);
    SchemaImportResult ImportSchema(SchemaItem const& si);
    bool WriteToDisk(Utf8CP fileName, const char* zSchema = nullptr, bool overrideFile = true) const;
    virtual ~InMemoryECDb();
    static Ptr Create();
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaSyncDb final {
    private:
        BeFileName m_fileName;

    public:
        explicit SchemaSyncDb(Utf8CP name);
        BeFileName GetFileName() const;
        std::unique_ptr<ECDb> OpenReadOnly(DefaultTxn mode = DefaultTxn::Yes);
        std::unique_ptr<ECDb> OpenReadWrite(DefaultTxn mode = DefaultTxn::Yes);
        void WithReadOnly(std::function<void(ECDbR)> cb, DefaultTxn mode = DefaultTxn::Yes);
        void WithReadWrite(std::function<void(ECDbR)> cb, DefaultTxn mode = DefaultTxn::Yes);
        //SchemaSync::Status Push(ECDbR ecdb, std::function<void()> cb = nullptr);
        SchemaSync::Status Pull(ECDbR ecdb, std::function<void()> cb = nullptr);
        SchemaSync::SyncDbUri GetSyncDbUri() const { return SchemaSync::SyncDbUri(m_fileName.GetNameUtf8().c_str()); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaSyncTestFixture : public ECDbTestFixture
    {
    static const char* DEFAULT_SHA3_256_ECDB_SCHEMA;
    static const char* DEFAULT_SHA3_256_ECDB_MAP;
    static const char* DEFAULT_SHA3_256_SQLITE_SCHEMA;
    static const char* DEFAULT_SHA3_256_CHANNEL_SQLITE_SCHEMA;
    std::unique_ptr<ECDbHub> m_hub;
    std::unique_ptr<TrackedECDb> m_briefcase;
    std::unique_ptr<SchemaSyncDb> m_schemaChannel;

    static SchemaImportResult ImportSchemas(ECDbR ecdb, std::vector<SchemaItem> items, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None, SchemaSync::SyncDbUri uri = SchemaSync::SyncDbUri());
    static SchemaImportResult ImportSchema(ECDbR ecdb, SchemaItem item, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None, SchemaSync::SyncDbUri uri = SchemaSync::SyncDbUri());
    static std::unique_ptr<TrackedECDb> OpenECDb(Utf8CP asFileName);
    DbResult ReopenECDb();
    void CloseECDb();
    SchemaImportResult SetupECDb(Utf8CP ecdbName);
    SchemaImportResult SetupECDb(Utf8CP ecdbName, SchemaItem const& schema, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None);
    SchemaImportResult ImportSchema(SchemaItem item, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None);
    DropSchemaResult DropSchema(Utf8CP schemaName);
    static Utf8String GetSchemaHash(ECDbCR db);
    static Utf8String GetMapHash(ECDbCR db);
    static Utf8String GetDbSchemaHash(ECDbCR db);
    static bool ForeignkeyCheck(ECDbCR db);
    static void PrintHash(ECDbR ecdb, Utf8CP desc);
    static void CheckHashes(ECDbR ecdb, Utf8CP schemaHash = DEFAULT_SHA3_256_ECDB_SCHEMA, Utf8CP mapHash = DEFAULT_SHA3_256_ECDB_MAP, Utf8CP dbSchemaHash = DEFAULT_SHA3_256_SQLITE_SCHEMA, bool strictCheck = false, int lineNo = -1);
    static void CheckSyncHashes(ECDbR ecdb, Utf8CP schemaHash = DEFAULT_SHA3_256_ECDB_SCHEMA, Utf8CP mapHash = DEFAULT_SHA3_256_ECDB_MAP, Utf8CP dbSchemaHash = DEFAULT_SHA3_256_CHANNEL_SQLITE_SCHEMA, bool strictCheck = false, int lineNo = -1) {
        CheckHashes(ecdb, schemaHash, mapHash, dbSchemaHash, strictCheck, lineNo);
    }

    static std::string GetIndexDDL(ECDbCR ecdb, Utf8CP indexName);
    static void Test(Utf8CP name, std::function<void()> test);

    //! The sync db holds ec_ tables and the handful of profile tables Init leaves behind. Anything
    //! else means a data table leaked into it.
    static void VerifySyncDbHoldsOnlyMetadata(ECDbR syncDb, Utf8CP context);
    //! Every ec_ row the briefcase holds has to exist in the sync db with the same values. The sync
    //! db may hold more - it is the record, the briefcase is a filtered view of it. Only valid
    //! while the briefcase is level with the sync db; VerifySchemaSyncRules checks that first.
    //! Attaches the sync db, which commits, so this belongs at the end of a test.
    static void VerifyBriefcaseRowsExistInSyncDb(ECDbR briefcase, SchemaSyncDb& syncDb, Utf8CP context);
    //! PRAGMA integrity_check + foreign_key_check, to catch corruption a targeted assertion misses.
    static void VerifyFileIsSound(ECDbCR db, Utf8CP context);
    //! All of the above. SchemaSyncTestFixture::TearDown runs it for m_briefcase; tests driving
    //! their own briefcases call it themselves.
    static void VerifySchemaSyncRules(SchemaSyncDb& syncDb, std::vector<ECDb*> const& briefcases, Utf8CP context);

    //! Compares every ec_ table between two files and names the rows that differ, rather than only
    //! reporting that a hash did not match. ec_cache_* is skipped - it is derived and rebuilt locally.
    static void ExpectECTablesIdentical(ECDbR actual, ECDbR expected, Utf8CP context);
    //! Compares the physical schema - every table, index and trigger, by the DDL SQLite kept for it.
    //! ExpectECTablesIdentical cannot see foreign keys or triggers: they are never persisted in ec_,
    //! only text inside a CREATE statement that DerivedDbStructures works out again per file.
    static void ExpectPhysicalSchemaIdentical(ECDbR actual, ECDbR expected, Utf8CP context);
    //! Checks the number and actions of every physical foreign key on one table. This pins the
    //! definitions DerivedDbStructures should produce instead of only comparing two possibly-wrong files.
    static void ExpectForeignKeys(ECDbR db, Utf8CP tableName, int expectedCount, Utf8CP expectedOnDelete, Utf8CP expectedOnUpdate, Utf8CP context);
    //! Foreign keys are deferred while rows are copied, so a violation otherwise surfaces at
    //! SaveChanges with nothing to say about its cause. This names the offending row first.
    static void ExpectNoForeignKeyViolations(ECDbR db, Utf8CP context);

    //! Every instance in a file, as JSON, keyed by class and instance id.
    //!
    //! Take one before a schema change and one after, then ExpectCensusPreserved. The oracles above
    //! all read metadata or DDL; this is the only one that reads the data itself, which is what the
    //! update path claims never to touch.
    struct InstanceCensus final {
        //! class full name -> instance id (hex) -> the row as JSON
        bmap<Utf8String, bmap<Utf8String, Utf8String>> m_rowsByClass;

        //! Read every mapped entity and relationship class in the file. Classes with no instances
        //! contribute nothing, so the standard schemas cost a prepare and no more.
        static InstanceCensus Take(ECDbCR db);
        size_t GetInstanceCount() const;
        size_t GetClassCount() const { return m_rowsByClass.size(); }
    };

    //! Every instance in @p before must still be in @p after with the same values.
    //!
    //! Properties that appeared since read null and are ignored, so a test that adds a property does
    //! not have to describe what it added. Properties that disappeared are reported unless named in
    //! @p removedProperties as "ClassName.PropertyName". Reports the class, instance id and property
    //! of each divergence.
    static void ExpectCensusPreserved(InstanceCensus const& before, InstanceCensus const& after, Utf8CP context,
                                      std::vector<Utf8String> const& removedProperties = {});

    //! Whether the running test has already reported a failure, for the checks that only make sense
    //! against a file nothing else complained about. gtest's failure state does not exist in the
    //! non-gtest BeTest configuration the mobile builds use, where this always reads false.
    static bool CurrentTestHasFailed()
        {
#if defined (USE_GTEST)
        return ::testing::Test::HasFailure();
#else
        return false;
#endif
        }

    void TearDown() override;

    SchemaSync::SyncDbUri GetSyncDbUri()
        {
        return m_schemaChannel != nullptr ? m_schemaChannel->GetSyncDbUri() : SchemaSync::SyncDbUri();
        }
    int GetColumnCount(Utf8CP dbTableName) const
        {
        bvector<Utf8String> cols;
        m_briefcase->GetColumns(cols, dbTableName);
        return (int) cols.size();
        }
    };

#define ASSERT_ECDB_SCHEMA_HASH(ECDB, HASH)   ASSERT_STREQ(HASH, SchemaSyncTestFixture::GetSchemaHash(ECDB).c_str())   << "File: " << (ECDB).GetDbFileName();
#define ASSERT_ECDB_MAP_HASH(ECDB, HASH)      ASSERT_STREQ(HASH, SchemaSyncTestFixture::GetMapHash(ECDB).c_str())      << "File: " << (ECDB).GetDbFileName();
#define ASSERT_SQLITE_SCHEMA_HASH(ECDB, HASH) ASSERT_STREQ(HASH, SchemaSyncTestFixture::GetDbSchemaHash(ECDB).c_str()) << "File: " << (ECDB).GetDbFileName();