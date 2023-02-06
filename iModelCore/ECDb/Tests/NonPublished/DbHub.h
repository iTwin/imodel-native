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
struct DbTracker;
struct DbHub;
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DbChangeset final : public BeSQLite::ChangeSet {
    using Ptr = std::unique_ptr<DbChangeset>;

   private:
    Utf8String m_operation;
    Utf8String m_ddl;
    BeGuid m_id;
    BeGuid m_parentId;
    BeGuid m_dbId;
    bool m_hasSchemaChanges;
    int m_index;

   public:
       DbChangeset(BeGuid dbId, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset, BeGuid parentId)
        : m_dbId(dbId), m_id(true), m_operation(op), m_ddl(ddl), m_hasSchemaChanges(isSchemaChangeset || !Utf8String::IsNullOrEmpty(ddl)), m_index(-1), m_parentId(parentId) {}

    int GetIndex() const { return m_index; }
    bool HasSchemaChanges() const { return m_hasSchemaChanges; }
    BeGuid GetId() const { return m_id; }
    BeGuid GetParentId() const { return m_parentId; }
    void SetParentId(BeGuid guid) { m_parentId = guid; }
    void SetIndex(int index) { m_index = index; }
    BeGuid GetDbId() const { return m_dbId; }
    bvector<Utf8String> GetDDLs() const;
    Ptr Clone() const;
    static Ptr From(DbTracker& tracker, Utf8CP comment, BeGuid parent = BeGuid());
    static Ptr Create(BeGuid dbId, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset, BeGuid parentId);
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override;
    // void ToJson(bool includeVal) {
    //     BeJsDocument doc;
    //     doc.toObject();
    //     auto schemaChangeVal = doc["schema_change"];
    //     schemaChangeVal.toArray();
    //     for (auto& ddl, GetDDLs()) {
    //         schemaChangeVal.appendValue() = ddl;
    //     }
    //     auto dataChangeVal = doc["changes"];
    //     for(auto& change: Changes()) {


    //     }
    // }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct InMemoryECDb : public BentleyApi::BeSQLite::EC::ECDb {
    using Ptr = std::unique_ptr<InMemoryECDb>;

   private:
    std::unique_ptr<DbTracker> m_tracker;
    BeGuid m_parentChangesetId;
    void SetupTracker(std::unique_ptr<DbTracker> tracker = nullptr);
    virtual void _OnDbClose() override;

   public:
    explicit InMemoryECDb(bool enableTracker = true);
    BeGuid const& GetParentChangesetId() const { return m_parentChangesetId; }
    InMemoryECDb::Ptr CreateSnapshot(DbResult* outRc = nullptr);
    InMemoryECDb::Ptr CreateBriefcase(BeBriefcaseId id, DbResult* outRc = nullptr);
    DbTracker* GetTracker() { return m_tracker.get(); }
    DbResult PullMergePush(DbHub& hub, Utf8CP comment);
    SchemaImportResult ImportSchema(SchemaItem const& si);
    bool WriteToDisk(Utf8CP fileName, const char* zSchema = nullptr, bool overrideFile = true) const;
    virtual ~InMemoryECDb();
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DbTracker : BeSQLite::ChangeTracker {
    using Ptr = std::unique_ptr<DbTracker>;

   private:
    InMemoryECDb& m_mdb;
    std::vector<DbChangeset::Ptr> m_localChangesets;

   private:
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override;

   public:
    DbTracker(InMemoryECDb& db) : m_mdb(db) {
        AddRef();
        SetDb(&m_mdb);
    }
    InMemoryECDb const& GetECDb() const { return m_mdb; }
    BeGuid GetDbId() const;
    Utf8String GetDDL() const { return m_ddlChanges.ToString(); }
    DbChangeset::Ptr MakeChangeset(bool deleteLocalChangesets, Utf8CP op);
    std::vector<DbChangeset const*> GetLocalChangesets() const {
        std::vector<DbChangeset const*>  list;
        for(auto& r : m_localChangesets) {
            list.push_back(r.get());
        }
        return list;
    }


   Ptr Clone(InMemoryECDb&) const;



    static Ptr Create(InMemoryECDb& db) { return std::make_unique<DbTracker>(db); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DbHub final {
    using const_iterator = std::vector<DbChangeset::Ptr>::const_iterator;
    using guard = std::lock_guard<std::recursive_mutex>;
    struct Lock {
        BeBriefcaseId m_id;
    };
   private:
    BeGuid m_id;
    InMemoryECDb m_seed;
    std::vector<DbChangeset::Ptr> m_changesets;
    mutable std::recursive_mutex m_mutex;
    mutable BeBriefcaseId m_briefcaseId;
    Utf8String m_name;
    Lock m_schemaLock;
    Lock m_iModelLock;
    void ClearLocks(BeBriefcaseId id) {
        if (m_schemaLock.m_id == id) {
            m_schemaLock.m_id = BeBriefcaseId();
        }
        if (m_iModelLock.m_id == id) {
            m_iModelLock.m_id = BeBriefcaseId();
        }
    }
   public:
    bool AcquireSchemaLock(BeBriefcaseId id) {
        guard lk(GetMutex());
        if (m_schemaLock.m_id.IsValid()) {
            if (m_schemaLock.m_id != id) {
                return false;
            }
        } else {
            m_schemaLock.m_id = id;
        }
        return true;
    }
    bool AcquireIModelLock(BeBriefcaseId id) {
        guard lk(GetMutex());
        if (m_iModelLock.m_id.IsValid()) {
            if (m_iModelLock.m_id != id) {
                return false;
            }
        } else {
            m_iModelLock.m_id = id;
        }
        return true;
    }
     bool HasIModelLock(BeBriefcaseId id) {
          guard lk(GetMutex());
          return id == m_iModelLock.m_id;
     }
     bool HasSchemaLock(BeBriefcaseId id) {
          guard lk(GetMutex());
          return id == m_schemaLock.m_id;
     }

    explicit DbHub(Utf8CP name = "test") : m_name(name),m_briefcaseId(10) { m_id = m_seed.GetDbGuid(); }
    std::recursive_mutex& GetMutex() const { return m_mutex; }
    BeGuid GetDbId() const { return m_id; }
    BentleyStatus Push(BeBriefcaseId id, DbChangeset::Ptr changeset);
    BeBriefcaseId GetNextBriefcaseId() const {
        m_briefcaseId = m_briefcaseId.GetNextBriefcaseId();
        return m_briefcaseId;
        }
    std::vector<DbChangeset const*> QueryAfter(BeGuid id) const;
    std::vector<DbChangeset const*> QueryAll() const;
    InMemoryECDb::Ptr AcquireNewBriefcase();
    BeGuid GetTipChangesetId() const { return m_changesets.empty() ? BeGuid() : m_changesets.back()->GetId(); }

};


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SqlTypeDetector {
    private:
        std::unique_ptr<RE2> m_alterTableRegEx;
        std::unique_ptr<RE2> m_dropTableRegEx;
        std::unique_ptr<RE2> m_dropIndexRegEx;
        std::unique_ptr<RE2> m_createTableRegEx;
        std::unique_ptr<RE2> m_createIndexRegEx;
        std::unique_ptr<RE2> m_createViewRegEx;
        std::unique_ptr<RE2> m_pragmaRegEx;
        std::unique_ptr<RE2> m_dataInsertRegEx;
        std::unique_ptr<RE2> m_dataUpdateRegEx;
        std::unique_ptr<RE2> m_dataDeleteRegEx;
        std::unique_ptr<RE2> m_sysInsertRegEx;
        std::unique_ptr<RE2> m_sysUpdateRegEx;
        std::unique_ptr<RE2> m_sysDeleteRegEx;
        std::unique_ptr<RE2> m_insertRegEx;
        std::unique_ptr<RE2> m_updateRegEx;
        std::unique_ptr<RE2> m_deleteRegEx;

        static const std::vector<std::string> GetDataTables (DbCR conn);
        static const std::vector<std::string> GetSystemTables (DbCR conn);
        static std::string Join(std::vector<std::string> const& v, const std::string sep);
        static void Validate(RE2 const& re);
        void SetupRegex(DbCR conn, bool useDataCRUD);
    public:
        explicit SqlTypeDetector(DbCR conn, bool useDataCRUD) {
            SetupRegex(conn, useDataCRUD);
        }
        bool IsInsert(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_insertRegEx); }
        bool IsUpdate(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_updateRegEx); }
        bool IsDelete(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_deleteRegEx); }
        bool IsDataInsert(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dataInsertRegEx); }
        bool IsDataUpdate(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dataUpdateRegEx); }
        bool IsDataDelete(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dataDeleteRegEx); }
        bool IsSysInsert(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_sysInsertRegEx); }
        bool IsSysUpdate(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_sysUpdateRegEx); }
        bool IsSysDelete(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_sysDeleteRegEx); }
        bool IsPragma(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_pragmaRegEx); }
        bool IsAlterTable(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_alterTableRegEx); }
        bool IsDropTable(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dropTableRegEx); }
        bool IsDropIndex(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dropIndexRegEx); }
        bool IsCreateTable(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_createTableRegEx); }
        bool IsCreateIndex(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_createIndexRegEx); }
        bool IsCreateView(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_createViewRegEx); }
        bool IsDdl(Utf8CP sql) const { return IsAlterTable(sql) || IsDropTable(sql) || IsDropIndex(sql) || IsCreateIndex(sql) || IsCreateTable(sql) || IsCreateView(sql); }
        bool IsDml(Utf8CP sql) const { return IsInsert(sql) || IsUpdate(sql) || IsDelete(sql); }
        bool IsDataDml(Utf8CP sql) const { return IsDataInsert(sql) || IsDataUpdate(sql) || IsDataDelete(sql); }
        bool IsSysDml(Utf8CP sql) const { return IsSysInsert(sql) || IsSysUpdate(sql) || IsSysDelete(sql); }
};