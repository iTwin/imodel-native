/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See COPYRIGHT.md in the repository root for full copyright notice.
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

    private:
        ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override;

    public:
        ECDbChangeSet(int index, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset)
            : m_operation(op), m_ddl(ddl), m_hasSchemaChanges(isSchemaChangeset || !Utf8String::IsNullOrEmpty(ddl)), m_index(index), m_ecdb(nullptr) {}
        int GetIndex() const { return m_index; }
        bool HasSchemaChanges() const { return m_hasSchemaChanges; }
        bvector<Utf8String> GetDDLs() const;
        void SetECDb(ECDbCR ecdb) { m_ecdb = &ecdb; }
        ECDb const* GetECDb() const { return m_ecdb; }
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
        int m_changesetId = 0;
        void SetupTracker(std::unique_ptr<ECDbChangeTracker> tracker = nullptr);
        virtual void _OnDbClose() override;
        virtual DbResult _OnDbCreated(CreateParams const& params) override;
        virtual DbResult _OnDbOpening() override;

    public:
        ECDbChangeTracker* GetTracker() { return m_tracker.get(); }
        void SetHub(ECDbHub& hub) { m_hub = &hub; }
        ECDbHub* GetHub() { return m_hub; }
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
        DbResult CreateSeedFile();
        BeFileName BuildECDbPath(Utf8CP name) const;
        BeBriefcaseId GetNextBriefcaseId() { return (m_briefcaseid = m_briefcaseid.GetNextBriefcaseId()); }

    public:
        ECDbHub();
        std::unique_ptr<TrackedECDb> CreateBriefcase();
        std::vector<ECDbChangeSet*> Query(int afterChangesetId = 1);
        int PushNewChangeset(ECDbChangeSet::Ptr changeset);
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
struct SharedSchemaDb final {
    private:
        BeFileName m_fileName;

    public:
        explicit SharedSchemaDb(Utf8CP name);
        BeFileName GetFileName() const;
        std::unique_ptr<ECDb> OpenReadOnly(DefaultTxn mode = DefaultTxn::Yes);
        std::unique_ptr<ECDb> OpenReadWrite(DefaultTxn mode = DefaultTxn::Yes);
        void WithReadOnly(std::function<void(ECDbR)> cb, DefaultTxn mode = DefaultTxn::Yes);
        void WithReadWrite(std::function<void(ECDbR)> cb, DefaultTxn mode = DefaultTxn::Yes);
        //SharedSchemaChannel::Status Push(ECDbR ecdb, std::function<void()> cb = nullptr);
        SharedSchemaChannel::Status Pull(ECDbR ecdb, std::function<void()> cb = nullptr);
        SharedSchemaChannel::ChannelUri GetChannelUri() const { return SharedSchemaChannel::ChannelUri(m_fileName.GetNameUtf8().c_str()); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaSyncTestFixture : public ECDbTestFixture {
    static SchemaImportResult ImportSchemas(ECDbR ecdb, std::vector<SchemaItem> items, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None);
    static SchemaImportResult ImportSchema(ECDbR ecdb, SchemaItem item, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None);
    static std::unique_ptr<TrackedECDb> OpenECDb(Utf8CP asFileNam);
    static Utf8String GetSchemaHash(ECDbCR db);
    static Utf8String GetMapHash(ECDbCR db);
    static Utf8String GetDbSchemaHash(ECDbCR db);
    static bool ForeignkeyCheck(ECDbCR db);
    static void PrintHash(ECDbR ecdb, Utf8CP desc);
    static std::string GetIndexDDL(ECDbCR ecdb, Utf8CP indexName);
    static void Test(Utf8CP name, std::function<void()> test);
};