/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include "ECDbPublishedTests.h"
#include <numeric>
#include <Bentley/SHA1.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE




struct DbChangeTracker;
struct ECDbHub;
struct DbChangeset : public BeSQLite::ChangeSet {
    using Ptr = std::unique_ptr<DbChangeset>;
    private:
        Utf8String m_operation;
        Utf8String m_ddl;
        bool m_hasSchemaChanges;
        int m_index;

    private:
        ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override;

    public:
        DbChangeset(int index, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset)
            : m_operation(op), m_ddl(ddl), m_hasSchemaChanges(isSchemaChangeset || !Utf8String::IsNullOrEmpty(ddl)), m_index(index) {}
        int GetIndex() const { return m_index; }
        bool HasSchemaChanges() const { return m_hasSchemaChanges; }
        bvector<Utf8String> GetDDLs() const;
        Ptr Clone() const;

        void  ToSQL(DbCR db, std::function<void(std::string const&, std::string const&)> cb) const{
            for (auto& ddl : GetDDLs()) {
                cb("",  ddl + ";");
            }
            bmap<Utf8String,bvector<Utf8String>> tableColMap;
            auto toString = [](DbValue const& val) -> std::string {
                if (!val.IsValid()) {
                    return "NULL";
                }
                switch (val.GetValueType()) {
                case DbValueType::IntegerVal:
                    return Utf8PrintfString("%" PRId64, val.GetValueInt64());
                case DbValueType::FloatVal:
                    return Utf8PrintfString("%0.17lf", val.GetValueDouble());
                case DbValueType::TextVal:
                    return Utf8PrintfString("'%s'", val.GetValueText());
                case DbValueType::BlobVal: {
                    Utf8String hexStr;
                    std::string valStr = "X'";
                    const auto ptr = (uint8_t const*)(val.GetValueBlob());
                    const auto len = val.GetValueBytes();
                    for (auto i = 0; i < len; ++i) {
                        hexStr.clear();
                        hexStr.Sprintf("%02X", ptr[i]);
                        valStr.append(hexStr);
                    }
                    valStr.append("'");
                    return std::move(valStr);
                }
                case DbValueType::NullVal:
                    return "NULL";
                }
                return "<unknown>";
            };

            std::string sql;
            for (auto const& change : const_cast<DbChangeset*>(this)->GetChanges()) {
                Byte* pkCols;
                int nPkCols;
                Utf8CP tableName;
                DbOpcode opCode;
                int indirect;
                int nCols;
                change.GetOperation(&tableName, &nCols, &opCode, &indirect);
                change.GetPrimaryKeyColumns(&pkCols, &nPkCols);
                auto it = tableColMap.find(tableName);
                if (it == tableColMap.end()) {
                    auto newIt = tableColMap.emplace(tableName, bvector<Utf8String>());
                    it = newIt.first;
                    db.GetColumns(it->second, tableName);
                }
                auto const& columns = it->second;
                auto const& tbl = it->first;
                if (opCode == DbOpcode::Delete) {
                    sql = "DELETE FROM ";
                    sql.append(tableName).append(" WHERE ");
                    bool first = true;
                    for (int i = 0; i < nCols; ++i) {
                        if (pkCols[i]) {
                            auto& columnName = columns[i];
                            auto dbVal = change.GetOldValue(i);
                            if (first) {
                                first = false;
                            } else {
                                sql.append(" AND ");
                            }
                            sql.append("(").append(columnName).append(" IS ").append(toString(dbVal).append(")"));
                        }
                    }
                    sql.append(";");
                    cb(tbl, sql);
                } else if (opCode == DbOpcode::Insert) {
                    sql = "INSERT INTO ";
                    sql.append(tableName).append("(");
                    std::string val = " VALUES(";
                    bool first = true;
                    for (int i = 0; i < nCols; ++i) {
                        auto& columnName = columns[i];
                        auto dbVal = change.GetNewValue(i);
                        if (dbVal.IsNull())
                            continue;

                        if (first) {
                            first = false;
                        } else {
                            sql.append(",");
                            val.append(",");
                        }
                        sql.append(columnName);
                        val.append(toString(dbVal));
                    }
                    sql.append(")").append(val).append(");");
                    cb(tbl, sql);
                } else if (opCode == DbOpcode::Update) {
                    sql = "UPDATE ";
                    sql.append(tableName).append(" SET ");
                    std::string where = " WHERE ";
                    bool firstPk = true;
                    bool firstData = true;
                    for (int i = 0; i < nCols; ++i) {
                        auto& columnName = columns[i];
                        if (pkCols[i]) {
                            auto dbVal = change.GetOldValue(i);
                           if (firstPk) {
                                firstPk = false;
                            } else {
                                sql.append(" AND ");
                            }
                            where.append("(").append(columnName).append(" IS ").append(toString(dbVal).append(")"));
                        } else {
                            auto oldVal = change.GetNewValue(i);
                            if (firstData) {
                                firstData = false;
                            } else {
                                sql.append(",");
                            }
                            sql.append(columnName).append("=").append(toString(oldVal));
                        }
                    }
                    sql.append(where).append(";");
                    cb(tbl, sql);
                }
            }
        }
        static Ptr From(DbChangeTracker& tracker, Utf8CP comment);
        static Ptr Create(int index, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset);
};

struct DbChangeTracker : BeSQLite::ChangeTracker {
    using Ptr = std::unique_ptr<DbChangeTracker>;

   private:
    ECDbR m_mdb;
    std::vector<DbChangeset::Ptr> m_localChangesets;
    bset<std::string> m_tableToIgnore;

private:
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override;
    TrackChangesForTable _FilterTable(Utf8CP tableName) override {
        if (m_tableToIgnore.find(tableName) != m_tableToIgnore.end())
            return TrackChangesForTable::No;
        return TrackChangesForTable::Yes;
    }
   public:
    DbChangeTracker(ECDbR db) : m_mdb(db) {
        AddRef();
        SetDb(&db);
        m_tableToIgnore.insert("ec_cache_ClassHasTables");
        m_tableToIgnore.insert("ec_cache_ClassHierarchy");
    }
    ECDbCR const& GetDb() const { return m_mdb; }
    BeGuid GetDbId() const;
    Utf8String GetDDL() const { return m_ddlChanges.ToString(); }
    DbChangeset::Ptr MakeChangeset(bool deleteLocalChangesets, Utf8CP op);
    std::vector<DbChangeset const*> GetLocalChangesets() const;

    void ToSql(std::function<void(DbChangeset const&, std::string const&, std::string const&)> cb) const {
        for (auto&  cs: m_localChangesets) {
            cs->ToSQL(m_mdb, [&](std::string const& tbl, std::string const& sql) {
                cb(*cs, tbl, sql);
            });
        }
    }
    Ptr Clone(ECDbR) const;
    static Ptr Create(ECDbR db) { return std::make_unique<DbChangeTracker>(db); }
};

struct TrackedECDb : ECDb {
    private:
        std::unique_ptr<DbChangeTracker> m_tracker;
        ECDbHub* m_hub;
        int m_changesetId = 0;
        void SetupTracker(std::unique_ptr<DbChangeTracker> tracker = nullptr);
        virtual void _OnDbClose() override;
        virtual DbResult _OnDbCreated(CreateParams const& params) override {
            auto rc = ECDb::_OnDbCreated(params);
            SetupTracker();
            return rc;
        }
        virtual DbResult _OnDbOpening() override {
            auto rc = ECDb::_OnDbOpening();
            if (!IsReadonly()) {
                SetupTracker();

            }
            return rc;
        }
    public:
        DbChangeTracker* GetTracker() { return m_tracker.get(); }
        void SetHub(ECDbHub& hub) { m_hub = &hub; }
        ECDbHub* GetHub() { return m_hub; }
        DbResult PullMergePush(Utf8CP comment);
        virtual ~TrackedECDb();
};
struct ECDbHub {
    private:
        std::vector<DbChangeset::Ptr> m_changesets;
        BeGuid m_id;
        BeFileName m_basePath;
        BeFileName m_seedFile;
        BeBriefcaseId m_briefcaseid;

    private:
        DbResult CreateSeedFile();
        BeFileName BuildECDbPath(Utf8CP name) const;
        BeBriefcaseId GetNextBriefcaseId() {
            return (m_briefcaseid = m_briefcaseid.GetNextBriefcaseId());
        }

    public:
        ECDbHub();
        std::unique_ptr<TrackedECDb> CreateBriefcase();
        std::vector<DbChangeset*> Query(int afterChangesetId = 1) {
            std::vector<DbChangeset*> results;
            const auto cid = afterChangesetId - 1;
            if (cid < 0 || cid > (int)m_changesets.size() - 1) {
                return results;
            }
            for (auto i = cid; i < m_changesets.size(); ++i) {
                results.push_back(m_changesets[i].get());
            }
            return results;
        }
        int PushNewChangeset(DbChangeset::Ptr changeset) {
            m_changesets.push_back(std::move(changeset));
            return (int)(m_changesets.size()) - 1;
        }
};
struct SchemaSyncTestFixture : public ECDbTestFixture {
	static std::unique_ptr<TrackedECDb> CreateECDb(Utf8CP asFileName) {
        auto fileName = BuildECDbPath(asFileName);
        if (fileName.DoesPathExist()) {
			if (fileName.BeDeleteFile() != BeFileNameStatus::Success) {
                throw std::runtime_error("unable to delete file");
            }
        }
        auto ecdb = std::make_unique<TrackedECDb>();
		if (BE_SQLITE_OK != ecdb->CreateNewDb(fileName)) {
            return nullptr;
        }
        ecdb->SaveChanges();
        return std::move(ecdb);
    }

	static std::unique_ptr<TrackedECDb> CopyAs(ECDbR fromDb, Utf8CP asFileName) {
        fromDb.SaveChanges();
        auto fileName = BuildECDbPath(asFileName);
        if (fileName.DoesPathExist()) {
			if (fileName.BeDeleteFile() != BeFileNameStatus::Success) {
                throw std::runtime_error("unable to delete file");
            }
        }
        BeFileName::BeCopyFile( BeFileName(fromDb.GetDbFileName(), true), fileName);
        auto ecdb = std::make_unique<TrackedECDb>();
        if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(fileName, Db::OpenParams(Db::OpenMode::ReadWrite))) {
            return nullptr;
        }
        ecdb->SaveChanges();
        return std::move(ecdb);
    }
    static SchemaImportResult ImportSchemas(ECDbR ecdb, std::vector<SchemaItem> items, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None) {
        auto schemaReadContext = ECSchemaReadContext::CreateContext();
		schemaReadContext->AddSchemaLocater(ecdb.GetSchemaLocater());
		bvector<ECSchemaCP> importSchemas;
		for(auto& item: items) {
			ECSchemaPtr schema;
			ECSchema::ReadFromXmlString(schema, item.GetXmlString().c_str(), *schemaReadContext);
			if (!schema.IsValid()) {
                return SchemaImportResult::ERROR;
            }
            importSchemas.push_back(schema.get());
        }
        return ecdb.Schemas().ImportSchemas(importSchemas, opts);
    }
	static SchemaImportResult ImportSchema(ECDbR ecdb, SchemaItem item, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None) {
        return ImportSchemas(ecdb, std::vector<SchemaItem>{item}, opts);
    }
	static std::unique_ptr<TrackedECDb> OpenECDb(Utf8CP asFileNam) {
        auto ecdb = std::make_unique<TrackedECDb>();
        if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(asFileNam, Db::OpenParams(Db::OpenMode::ReadWrite))) {
            return nullptr;
        }
        return std::move(ecdb);
    }
    static Utf8String GetSchemaHash(ECDbCR db) {
        ECSqlStatement stmt;
        if (stmt.Prepare(db, "PRAGMA checksum(ec_schema)") != ECSqlStatus::Success) {
            return "";
        }
        if (stmt.Step() == BE_SQLITE_ROW) {
            return stmt.GetValueText(0);
        }
        return "";
    }
    static Utf8String GetMapHash(ECDbCR db) {
        ECSqlStatement stmt;
        if (stmt.Prepare(db, "PRAGMA checksum(ec_map)") != ECSqlStatus::Success) {
            return "";
        }
        if (stmt.Step() == BE_SQLITE_ROW) {
            return stmt.GetValueText(0);
        }
        return "";
    }
    static Utf8String GetDbSchemaHash(ECDbCR db) {
        ECSqlStatement stmt;
        if (stmt.Prepare(db, "PRAGMA checksum(db_schema)") != ECSqlStatus::Success) {
            return "";
        }
        if (stmt.Step() == BE_SQLITE_ROW) {
            return stmt.GetValueText(0);
        }
        return "";
    }
    static bool ForeignkeyCheck(ECDbCR db) {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "PRAGMA foreign_key_check"));
        auto rc = stmt.Step();
        if (rc == BE_SQLITE_DONE) {
            return true;
        }
        while(rc == BE_SQLITE_ROW) {
            printf("%s\n",
                   SqlPrintfString("[table=%s], [rowid=%lld], [parent=%s], [fkid=%d]",
                                   stmt.GetValueText(0),
                                   stmt.GetValueInt64(1),
                                   stmt.GetValueText(2),
                                   stmt.GetValueInt(3))
                       .GetUtf8CP());

            rc = stmt.Step();
        }
        return false;
    }
    static std::string GetLastChangesetAsSql(TrackedECDb& db) {
        std::vector<std::string> sqlList;
        const auto changesets = db.GetTracker()->GetLocalChangesets();
        if (changesets.size() == 0) {
            return "";
        }
        changesets.back()->ToSQL(db, [&](std::string const& tbl, std::string const& sql) {
            sqlList.push_back(sql);
        });
        std::sort(sqlList.begin(), sqlList.end());
        return std::accumulate(
            std::next(sqlList.begin()),
            std::end(sqlList),
            std::string{sqlList.front()},
            [&](std::string const& acc, const std::string& piece) {
                return acc + "\n" + piece;
            }
        );
    }
};
void TrackedECDb::SetupTracker(std::unique_ptr<DbChangeTracker> tracker) {
    if (m_tracker != nullptr) {
        m_tracker->EndTracking();
        this->SetChangeTracker(nullptr);
        m_tracker = nullptr;
    }
    m_tracker = tracker != nullptr ? std::move(tracker) : std::make_unique<DbChangeTracker>(*this);
    this->SetChangeTracker(m_tracker.get());
    m_tracker->EnableTracking(true);
}

TrackedECDb::~TrackedECDb() {
    if (IsDbOpen())
        CloseDb();
}
void TrackedECDb::_OnDbClose() {
    SaveChanges();
    this->SetChangeTracker(nullptr);
    if (m_tracker != nullptr) {
        m_tracker->EndTracking();
        m_tracker = nullptr;
    }
    ECDb::_OnDbClose();
}
std::vector<DbChangeset const*> DbChangeTracker::GetLocalChangesets() const {
    std::vector<DbChangeset const*>  list;
    for(auto& r : m_localChangesets) {
        list.push_back(r.get());
    }
    return list;
}
DbChangeset::Ptr DbChangeTracker::MakeChangeset(bool deleteLocalChangesets, Utf8CP op) {
    bvector<Utf8String> ddlChanges;
    ChangeGroup group;
    bool hasSchemaChanges = false;

    m_mdb.SaveChanges(op);

    for (auto& changeset : m_localChangesets) {
        const auto rc = changeset->AddToChangeGroup(group);
        if (rc != BE_SQLITE_OK) {
            return nullptr;
        }
        for (auto& ddl : changeset->GetDDLs()) {
            ddlChanges.push_back(ddl);
        }
        if (!hasSchemaChanges) {
            hasSchemaChanges = changeset->HasSchemaChanges();
        }
    }

    auto changeset = DbChangeset::Create((int)(m_localChangesets.size() + 1), op, BeStringUtilities::Join(ddlChanges, ";").c_str(), hasSchemaChanges);
    if (BE_SQLITE_OK != changeset->FromChangeGroup(group)) {
        return nullptr;
    }
    if (deleteLocalChangesets) {
        m_localChangesets.clear();
    }
    return std::move(changeset);
}
ChangeTracker::OnCommitStatus DbChangeTracker::_OnCommit(bool isCommit, Utf8CP operation) {
    if (isCommit) {
        auto changeset = DbChangeset::From(*this, operation);
        if (changeset != nullptr) {
            m_localChangesets.push_back(std::move(changeset));
        }
    }
    EndTracking();
    CreateSession();
    return OnCommitStatus::Commit;
}
DbChangeTracker::Ptr DbChangeTracker::Clone(ECDb& db) const {
    auto tracker= Create(db);
    for (auto& changeset : m_localChangesets) {
        tracker->m_localChangesets.push_back(changeset->Clone());
    }
    return std::move(tracker);

}


bvector<Utf8String> DbChangeset::GetDDLs() const {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(m_ddl.c_str(), ";", tokens);
    return tokens;
}
DbChangeset::Ptr DbChangeset::Clone() const {
    auto changeset = std::make_unique<DbChangeset>(0, m_operation.c_str(), m_ddl.c_str(), m_hasSchemaChanges);
    for (auto& chunk : this->m_data.m_chunks) {
        changeset->m_data.Append((Byte const*)&chunk[0], (int)chunk.size());
    }
    return std::move(changeset);
}
DbChangeset::Ptr DbChangeset::From(DbChangeTracker& tracker, Utf8CP comment) {
    if (!tracker.HasChanges() && !tracker.HasDdlChanges()) {
        return nullptr;
    }
    auto changeset = std::make_unique<DbChangeset>( (int)(tracker.GetLocalChangesets().size() + 1), comment, tracker.GetDDL().c_str(), tracker.HasEcSchemaChanges());
    if (tracker.HasChanges()) {
        auto rc = changeset->FromChangeTrack(tracker);
        if (rc != BE_SQLITE_OK) {
            return nullptr;
        }
    }
    return std::move(changeset);
}
DbChangeset::Ptr DbChangeset::Create(int index, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset) {
    return std::make_unique<DbChangeset>(index, op, ddl, isSchemaChangeset);
}
ChangeStream::ConflictResolution DbChangeset::_OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) {

    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    BeAssert(result == BE_SQLITE_OK);

    if (cause == ChangeSet::ConflictCause::Conflict) {
        return ChangeSet::ConflictResolution::Replace;
    }
    if (cause == ChangeSet::ConflictCause::ForeignKey) {
        // Note: No current or conflicting row information is provided if it's a FKey conflict
        // Since we abort on FKey conflicts, always try and provide details about the error
        int nConflicts = 0;
        result = iter.GetFKeyConflicts(&nConflicts);
        BeAssert(result == BE_SQLITE_OK);
        LOG.errorv("Detected %d foreign key conflicts in ChangeSet. Aborting merge.", nConflicts);
        return ChangeSet::ConflictResolution::Abort ;
    }
    if(cause == ChangeSet::ConflictCause::NotFound) {
        if (opcode == DbOpcode::Delete) {
            // Caused by CASCADE DELETE on a foreign key, and is usually not a problem.
            return ChangeSet::ConflictResolution::Skip;
        }
        if (opcode == DbOpcode::Update && 0 == ::strncmp(tableName, "ec_", 3)) {
            // Caused by a ON DELETE SET NULL constraint on a foreign key - this is known to happen with "ec_" tables, but needs investigation if it happens otherwise
            return ChangeSet::ConflictResolution::Skip;
        }
        // Refer to comment below
        return opcode == DbOpcode::Update ? ChangeSet::ConflictResolution::Skip : ChangeSet::ConflictResolution::Replace;
    }
    if (ChangeSet::ConflictCause::Constraint == cause) {
        return ChangeSet::ConflictResolution::Skip;
    }
    return ConflictResolution::Replace;
}



BeFileName ECDbHub::BuildECDbPath(Utf8CP name) const {
    BeFileName outPath = m_basePath;
    outPath.AppendToPath(WPrintfString(L"%s.ecdb", name).GetWCharCP());
    return outPath;
}

DbResult ECDbHub::CreateSeedFile() {
    m_seedFile = BuildECDbPath("seed");
    if (m_seedFile.DoesPathExist()) {
        if (m_seedFile.BeDeleteFile() != BeFileNameStatus::Success) {
            throw std::runtime_error("unable to delete file");
        }
    }
    auto ecdb = std::make_unique<TrackedECDb>();
    if (BE_SQLITE_OK != ecdb->CreateNewDb(m_seedFile)) {
        return BE_SQLITE_ERROR;
    }
    ecdb->SaveChanges();
    ecdb->CloseDb();
    return BE_SQLITE_OK;
}
ECDbHub::ECDbHub():m_id(true), m_briefcaseid(10) {
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    outPath.AppendToPath(WString(m_id.ToString().c_str(), true).c_str());
    if (!outPath.DoesPathExist()) {
        BeFileName::CreateNewDirectory(outPath.GetName());
    }
    m_basePath = outPath;
    CreateSeedFile();
}
std::unique_ptr<TrackedECDb> ECDbHub::CreateBriefcase() {
    const auto briefcaseId = GetNextBriefcaseId();
    const auto fileName = BuildECDbPath(SqlPrintfString("%d", briefcaseId.GetValue()).GetUtf8CP());
    BeFileName::BeCopyFile(m_seedFile, fileName);
    auto ecdb = std::make_unique<TrackedECDb>();
    if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(fileName, Db::OpenParams(Db::OpenMode::ReadWrite))) {
        fileName.BeDeleteFile();
        return nullptr;
    }
    ecdb->ResetBriefcaseId(briefcaseId);
    ecdb->SaveChanges();
    ecdb->SetHub(*this);
    return std::move(ecdb);
}
DbResult TrackedECDb::PullMergePush(Utf8CP comment) {
    if (m_hub == nullptr || m_tracker == nullptr) {
        return BE_SQLITE_ERROR;
    }
    auto changesetsToApply = m_hub->Query(m_changesetId + 1);
    m_tracker->EnableTracking(false);
    for (auto& changesetToApply : changesetsToApply) {
        for (auto& ddl : changesetToApply->GetDDLs()) {
            auto rc = TryExecuteSql(ddl.c_str());
            if (rc != BE_SQLITE_OK && false) {
                m_tracker->EnableTracking(true);
                LOG.errorv("PullAndMergeChangesFrom(): %s", GetLastError().c_str());
                return rc;
            }
        }
        auto rc = changesetToApply->ApplyChanges(*this);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("PullAndMergeChangesFrom(): %s", GetLastError().c_str());
            return rc;
        }
    }

    m_tracker->EnableTracking(true);
    if (!m_tracker->GetLocalChangesets().empty()){
        auto changeset = m_tracker->MakeChangeset(true, comment);
        if (changeset == nullptr) {
            m_tracker->EnableTracking(true);
            return BE_SQLITE_ERROR;
        }
        m_changesetId = m_hub->PushNewChangeset(std::move(changeset));
    }
    return BE_SQLITE_OK;

}
//=============================================================================================================================
//=============================================================================================================================
//=============================================================================================================================
//=============================================================================================================================

TEST_F(SchemaSyncTestFixture, Test) {
    auto printHash = [&](ECDbR ecdb, Utf8CP desc) {
        printf("=====%s======\n", desc);
        printf("\tSchema: SHA1-%s\n", GetSchemaHash(ecdb).c_str());
        printf("\t   Map: SHA1-%s\n", GetMapHash(ecdb).c_str());
        printf("\t    Db: SHA1-%s\n", GetDbSchemaHash(ecdb).c_str());
    };

    ECDbHub hub;

    std::string synDbFile;
    if ("create syncdb") {
        auto syncDb = CreateECDb("sync.db");
        synDbFile = std::string{syncDb->GetDbFileName()};
    }

    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    auto b3 = hub.CreateBriefcase();

    ASSERT_EQ(b1->GetBriefcaseId().GetValue(), 11);
    ASSERT_EQ(b2->GetBriefcaseId().GetValue(), 12);
    ASSERT_EQ(b3->GetBriefcaseId().GetValue(), 13);
    ASSERT_EQ(BE_SQLITE_OK, b1->Schemas().InitSharedSchemaDb(synDbFile));

    if ("check syn db hash") {
        auto syncDb = OpenECDb(synDbFile.c_str());
        EXPECT_STREQ("9a0c2041a498d674fc0045cb0209e3dd8951b904", GetSchemaHash(*syncDb).c_str());
        EXPECT_STREQ("49675fee5c0f1264853df55142364516b5ef5202", GetMapHash(*syncDb).c_str());
        EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*syncDb).c_str());
    }

    EXPECT_STREQ("9a0c2041a498d674fc0045cb0209e3dd8951b904", GetSchemaHash(*b1).c_str());
    EXPECT_STREQ("49675fee5c0f1264853df55142364516b5ef5202", GetMapHash(*b1).c_str());
    EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*b1).c_str());


    EXPECT_STREQ("9a0c2041a498d674fc0045cb0209e3dd8951b904", GetSchemaHash(*b2).c_str());
    EXPECT_STREQ("49675fee5c0f1264853df55142364516b5ef5202", GetMapHash(*b2).c_str());
    EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*b2).c_str());


    auto pushChangesToSyncDb = [&](ECDbR ecdb) {
        ASSERT_EQ(BE_SQLITE_OK, ecdb.Schemas().SyncSchemas(synDbFile, SchemaManager::SyncAction::Push));
        ASSERT_TRUE(ForeignkeyCheck(ecdb));
    };
    auto pullChangesFromSyncDb = [&](ECDbR ecdb) {
        ASSERT_EQ(BE_SQLITE_OK, ecdb.Schemas().SyncSchemas(synDbFile, SchemaManager::SyncAction::Pull));
        ASSERT_EQ(BE_SQLITE_OK, ecdb.SaveChanges());
        ASSERT_TRUE(ForeignkeyCheck(ecdb));
    };

    auto getIndexDDL = [&](ECDbCR ecdb, Utf8CP indexName) -> std::string {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "select sql from sqlite_master where name=?"));
        stmt.BindText(1, indexName, Statement::MakeCopy::Yes);
        if (stmt.Step() == BE_SQLITE_ROW) {
            return stmt.GetValueText(0);
        }
        return "";
    };

    pushChangesToSyncDb(*b1);
    if ("import schema into b1") {
        auto schema1 = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Pipe1">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <DbIndexList xmlns="ECDbMap.02.00.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>idx_pipe1_p1</Name>
                                    <IsUnique>False</IsUnique>
                                    <Properties>
                                        <string>p1</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="p1" typeName="int" />
                    <ECProperty propertyName="p2" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml");
        ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b1, schema1));
        b1->SaveChanges();
        ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(getIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");
        pushChangesToSyncDb(*b1);
        EXPECT_STREQ("58378b2ff3c9c4ad9664395274173d2c9dee1970", GetSchemaHash(*b1).c_str());
        EXPECT_STREQ("fe4dfa5552f7a28b49e003ba2d2989ef3380538b", GetMapHash(*b1).c_str());
        EXPECT_STREQ("c7c7a104e2197c9aeb95e76534318a1c6ecc68d7", GetDbSchemaHash(*b1).c_str());
        ASSERT_TRUE(ForeignkeyCheck(*b1));
        // printf("======================\n");
        // printf("%s\n", GetLastChangesetAsSql(*b1).c_str());


    }

    if("check if sync-db has changes but not tables and index") {
        auto syncDb = OpenECDb(synDbFile.c_str());
        auto pipe1 = syncDb->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 2);
        ASSERT_FALSE(syncDb->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(getIndexDDL(*syncDb, "idx_pipe1_p1").c_str(), "");
        EXPECT_STREQ("58378b2ff3c9c4ad9664395274173d2c9dee1970", GetSchemaHash(*syncDb).c_str());
        EXPECT_STREQ("fe4dfa5552f7a28b49e003ba2d2989ef3380538b", GetMapHash(*syncDb).c_str());
        EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*syncDb).c_str()); // It should not change
        ASSERT_TRUE(ForeignkeyCheck(*syncDb));
    }

	if("pull changes from sync-db into b2 and verify class, table and index exists") {
        pullChangesFromSyncDb(*b2);
        auto pipe1 = b2->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 2);
        ASSERT_TRUE(b2->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(getIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");
        EXPECT_STREQ("58378b2ff3c9c4ad9664395274173d2c9dee1970", GetSchemaHash(*b2).c_str());
        EXPECT_STREQ("fe4dfa5552f7a28b49e003ba2d2989ef3380538b", GetMapHash(*b2).c_str());
        EXPECT_STREQ("c7c7a104e2197c9aeb95e76534318a1c6ecc68d7", GetDbSchemaHash(*b2).c_str());
        ASSERT_TRUE(ForeignkeyCheck(*b2));
        // printf("======================\n");
        // printf("%s\n", GetLastChangesetAsSql(*b2).c_str());

    }

    if ("update schema by adding more properties and expand index in b2") {
        auto schema2 = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Pipe1">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <DbIndexList xmlns="ECDbMap.02.00.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>idx_pipe1_p1</Name>
                                    <IsUnique>False</IsUnique>
                                    <Properties>
                                        <string>p1</string>
                                        <string>p2</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="p1" typeName="int" />
                    <ECProperty propertyName="p2" typeName="int" />
                    <ECProperty propertyName="p3" typeName="int" />
                    <ECProperty propertyName="p4" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml");
        ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b2, schema2));
        b2->SaveChanges();
        ASSERT_STRCASEEQ(getIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
        pushChangesToSyncDb(*b2);
        EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*b2).c_str());   // CORRECT
        EXPECT_STREQ("5775dbd6f8956d6cb25e5f55c57948eaeefd7a42", GetMapHash(*b2).c_str());      // CORRECT
        EXPECT_STREQ("a0ecfd5f3845cc756be16b9649b1a3d5d8be3325", GetDbSchemaHash(*b2).c_str()); // CORRECT
        ASSERT_TRUE(ForeignkeyCheck(*b2));
    }

	if("check if sync-db has changes but not tables and index") {
        auto syncDb = OpenECDb(synDbFile.c_str());
        auto pipe1 = syncDb->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 4);
        ASSERT_STRCASEEQ(getIndexDDL(*syncDb, "idx_pipe1_p1").c_str(), "");
        EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*syncDb).c_str());   // CORRECT
        EXPECT_STREQ("5775dbd6f8956d6cb25e5f55c57948eaeefd7a42", GetMapHash(*syncDb).c_str());      // CORRECT
        EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*syncDb).c_str()); // CORRECT It should not change
        ASSERT_TRUE(ForeignkeyCheck(*syncDb));
    }

	if("pull changes from sync db into master db and check if schema changes was there and valid") {
        pullChangesFromSyncDb(*b1);
        auto pipe1 = b1->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 4);
        ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(getIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
        EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*b1).c_str());   // CORRECT
        EXPECT_STREQ("5775dbd6f8956d6cb25e5f55c57948eaeefd7a42", GetMapHash(*b1).c_str());      // CORRECT
        EXPECT_STREQ("a0ecfd5f3845cc756be16b9649b1a3d5d8be3325", GetDbSchemaHash(*b1).c_str()); // CORRECT
        ASSERT_TRUE(ForeignkeyCheck(*b1));
    }

    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("b1 import schema"));
    if("b3 pull changes from hub") {
        ASSERT_EQ(BE_SQLITE_OK, b3->PullMergePush("add new schema"));
        auto pipe1 = b3->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 4);
        ASSERT_TRUE(b3->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(getIndexDDL(*b3, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
        EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*b3).c_str());   // CORRECT
        EXPECT_STREQ("5775dbd6f8956d6cb25e5f55c57948eaeefd7a42", GetMapHash(*b3).c_str());      // CORRECT
        EXPECT_STREQ("a0ecfd5f3845cc756be16b9649b1a3d5d8be3325", GetDbSchemaHash(*b3).c_str()); // CORRECT
    }


}

END_ECDBUNITTESTS_NAMESPACE
