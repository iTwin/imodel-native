#include "DbHub.h"
#include <numeric>

//***************************************************************************************
// InMemoryECDb
//***************************************************************************************
USING_NAMESPACE_BENTLEY_SQLITE_EC;

BeGuid DbTracker::GetDbId() const { return GetECDb().GetDbGuid(); }
//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
void InMemoryECDb::SetupTracker(std::unique_ptr<DbTracker> tracker) {
    if (m_tracker != nullptr) {
        m_tracker->EndTracking();
        this->SetChangeTracker(nullptr);
        m_tracker = nullptr;
    }
    m_tracker = tracker != nullptr ? std::move(tracker) : std::make_unique<DbTracker>(*this);
    this->SetChangeTracker(m_tracker.get());
    m_tracker->EnableTracking(true);
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
bool InMemoryECDb::WriteToDisk(Utf8CP fileName, const char *zSchema, bool overrideFile) const {
    BeFileName filePath(fileName);
    if (filePath.DoesPathExist()) {
        if (overrideFile) {
            if (filePath.BeDeleteFile() != BeFileNameStatus::Success) {
                return false;
            }
        } else {
            return false;
        }
    }
    DbBuffer buf = Serialize(zSchema);
    if (buf.Empty()) {
        return false;
    }
    BeFile outFile;
    if (BeFileStatus::Success != outFile.Create(filePath, true)) {
        return false;
    }
    if (BeFileStatus::Success != outFile.Write(nullptr, buf.Data(), (uint32_t)buf.Size())) {
        return false;
    }
    if (BeFileStatus::Success != outFile.Flush() ){
        return false;
    }
    return BeFileStatus::Success == outFile.Close();
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
InMemoryECDb::Ptr InMemoryECDb::CreateSnapshot(DbResult* outRc) {
    return CreateBriefcase(BeBriefcaseId(0), outRc);
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
InMemoryECDb::Ptr InMemoryECDb::CreateBriefcase(BeBriefcaseId id, DbResult* outRc) {
    DbResult ALLOW_NULL_OUTPUT(rc, outRc);
    //SaveChanges("create snapshot");
    auto buff = Serialize();
    auto dbPtr = std::make_unique<InMemoryECDb>(false);
    dbPtr->CloseDb();
    rc = Db::Deserialize(buff, *dbPtr, DbDeserializeOptions::FreeOnClose | DbDeserializeOptions::Resizable, nullptr, [&](DbR db) {
        db.ResetBriefcaseId(id);
    });
    if (rc == BE_SQLITE_OK) {

        dbPtr->ChangeDbGuid(GetDbGuid());
        auto clonedTracker = m_tracker->Clone(*dbPtr);
        dbPtr->SetupTracker(std::move(clonedTracker));
        return std::move(dbPtr);
    }
    return nullptr;
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
SchemaImportResult InMemoryECDb::ImportSchema(SchemaItem const& si) {
    auto ctx = ECSchemaReadContextPtr();
    if (ECDbTestFixture::ReadECSchema(ctx, *this, si) != SUCCESS)
        return SchemaImportResult::ERROR;

    bvector<ECN::ECSchemaP> schemas;
    ctx->GetCache().GetSchemas(schemas);
    bvector<ECN::ECSchemaCP> schemasIn(schemas.begin(), schemas.end());
   // if (m_tracker != nullptr) m_tracker->SetHasEcSchemaChanges(true);
    return Schemas().ImportSchemas(schemasIn, nullptr);
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
InMemoryECDb::InMemoryECDb(bool enableTracker) {
    if (CreateNewDb(BEDB_MemoryDb) != BE_SQLITE_OK) {
        throw std::runtime_error("unable to created in memory ecdb");
    }
    if (enableTracker) {
        SetupTracker();
    }
};

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
InMemoryECDb::~InMemoryECDb() {
    if (IsDbOpen())
        CloseDb();
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
void InMemoryECDb::_OnDbClose() {
    SaveChanges();
    ECDb::_OnDbClose();
    this->SetChangeTracker(nullptr);
    if (m_tracker != nullptr) {
        m_tracker->EndTracking();
        m_tracker = nullptr;
    }
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
DbResult InMemoryECDb::PullMergePush(DbHub& hub, Utf8CP comment) {
    DbHub::guard lk(hub.GetMutex());
    SqlTypeDetector detector(*this, false);
    if (hub.GetDbId() != m_tracker->GetDbId()) {
        return BE_SQLITE_ERROR;
    }

    m_tracker->EnableTracking(false);
    auto changesetsToApply = hub.QueryAfter(m_parentChangesetId);
    for (auto& changesetToApply : changesetsToApply) {
        for (auto& ddl : changesetToApply->GetDDLs()) {
            auto rc = TryExecuteSql(ddl.c_str());
            if (rc != BE_SQLITE_OK) {
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
        if (changesetToApply->HasSchemaChanges()) {
            AfterSchemaChangeSetApplied();
        } else {
            AfterDataChangeSetApplied();
        }
    }

    m_tracker->EnableTracking(true);
    if (!m_tracker->GetLocalChangesets().empty()){
        auto changeset = m_tracker->MakeChangeset(true, comment);
        if (changeset == nullptr) {
            m_tracker->EnableTracking(true);
            return BE_SQLITE_ERROR;
        }

        if (hub.Push(GetBriefcaseId(), std::move(changeset)) != SUCCESS) {
            // retry
            m_tracker->EnableTracking(true);
            return BE_SQLITE_ERROR;
        }
    }
    m_parentChangesetId = hub.GetTipChangesetId();
    return BE_SQLITE_OK;
}

//***************************************************************************************
// DbTracker
//***************************************************************************************
//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
DbChangeset::Ptr DbTracker::MakeChangeset(bool deleteLocalChangesets, Utf8CP op) {
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

    auto changeset = DbChangeset::Create(
        m_mdb.GetDbGuid(), op, BeStringUtilities::Join(ddlChanges, ";").c_str(), hasSchemaChanges, BeGuid());

    if (BE_SQLITE_OK != changeset->FromChangeGroup(group)) {
        return nullptr;
    }
    if (deleteLocalChangesets) {
        m_localChangesets.clear();
    }
    return std::move(changeset);
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
ChangeTracker::OnCommitStatus DbTracker::_OnCommit(bool isCommit, Utf8CP operation) {
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

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
DbTracker::Ptr DbTracker::Clone(InMemoryECDb& db) const {
    auto tracker= Create(db);
    for (auto& changeset : m_localChangesets) {
        tracker->m_localChangesets.push_back(changeset->Clone());
    }
    return std::move(tracker);
}

//***************************************************************************************
// DbHub
//***************************************************************************************
//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
BentleyStatus DbHub::Push(BeBriefcaseId id, DbChangeset::Ptr changeset) {
    DbHub::guard lck(m_mutex);
    if (changeset == nullptr) {
        return ERROR;
    }
    if (changeset->GetDbId() != m_id) {
        return ERROR;
    }
    ClearLocks(id);

    changeset->SetIndex((int)m_changesets.size());
    changeset->SetParentId(GetTipChangesetId());
    m_changesets.push_back(std::move(changeset));
    return SUCCESS;
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
std::vector<DbChangeset const*> DbHub::QueryAfter(BeGuid id) const {
    DbHub::guard lck(m_mutex);
    if (!id.IsValid()) {
        return QueryAll();
    }

    std::vector<DbChangeset const*> results;
    bool capture = false;
    for (auto& cs : m_changesets) {
        if (capture) {
            results.push_back(cs.get());
        } else if (cs->GetId() == id) {
            capture = true;
        }
    }
    return results;
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
std::vector<DbChangeset const*> DbHub::QueryAll() const {
    DbHub::guard lck(m_mutex);
    std::vector<DbChangeset const*> results;
    for (auto& cs : m_changesets) {
        results.push_back(cs.get());
    }
    return results;
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
std::unique_ptr<InMemoryECDb> DbHub::AcquireNewBriefcase()  {
    DbHub::guard lck(m_mutex);
    auto bc = m_seed.CreateBriefcase(GetNextBriefcaseId());
    if (bc->PullMergePush(*this,"")) {
        return nullptr;
    }
    return std::move(bc);
}

//***************************************************************************************
// DbChangeset
//***************************************************************************************
//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
bvector<Utf8String> DbChangeset::GetDDLs() const {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(m_ddl.c_str(), ";", tokens);
    return tokens;
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
DbChangeset::Ptr DbChangeset::Clone() const {
    auto changeset = std::make_unique<DbChangeset>(m_dbId, m_operation.c_str(), m_ddl.c_str(), m_hasSchemaChanges, m_parentId);
    for (auto& chunk : this->m_data.m_chunks) {
        changeset->m_data.Append((Byte const*)&chunk[0], (int)chunk.size());
    }
    return std::move(changeset);
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
DbChangeset::Ptr DbChangeset::From(DbTracker& tracker, Utf8CP comment, BeGuid parent) {
    auto& db = tracker.GetECDb();
    if (db.IsReadonly()) {
        return nullptr;
    }
    if (!tracker.HasChanges() && !tracker.HasDdlChanges()) {
        return nullptr;
    }
    auto changeset = std::make_unique<DbChangeset>(tracker.GetDbId(), comment, tracker.GetDDL().c_str(), tracker.HasEcSchemaChanges(), parent);
    if (tracker.HasChanges()) {
        auto rc = changeset->FromChangeTrack(tracker);
        if (rc != BE_SQLITE_OK) {
            return nullptr;
        }
    }
    return std::move(changeset);
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
DbChangeset::Ptr DbChangeset::Create(BeGuid dbId, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset, BeGuid parentId) {
    return std::make_unique<DbChangeset>(dbId, op, ddl, isSchemaChangeset, parentId);
}

//=======================================================================================
// @bsimethod
//+===============+===============+===============+===============+===============+======
ChangeStream ::ConflictResolution DbChangeset::_OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) {

    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    BeAssert(result == BE_SQLITE_OK);

    if (cause == ChangeSet::ConflictCause::Conflict) {
        return ChangeSet::ConflictResolution::Abort;
    }
    if (cause == ChangeSet::ConflictCause::ForeignKey) {
        return ChangeSet::ConflictResolution::Abort;
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

//***************************************************************************************
// SqlTypeDetector
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
const std::vector<std::string> SqlTypeDetector::GetDataTables(DbCR conn) {
    std::vector<std::string> tables;
    auto sql = R"sql(
        SELECT [tbl_name]
        FROM   [sqlite_master]
        WHERE   [type] = 'table'
                AND NOT [tbl_name] LIKE 'ec\_%'     ESCAPE '\'
                AND NOT [tbl_name] LIKE 'dgn\_%'    ESCAPE '\'
                AND NOT [tbl_name] LIKE 'be\_%'     ESCAPE '\'
                AND NOT [tbl_name] LIKE 'sqlite\_%' ESCAPE '\')sql";
    auto stmt = conn.GetCachedStatement(sql);
    while (stmt->Step() == BE_SQLITE_ROW) {
        tables.push_back(stmt->GetValueText(0));
    }
    std::sort(tables.begin(), tables.end());
    return tables;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
const std::vector<std::string> SqlTypeDetector::GetSystemTables(DbCR conn) {
    std::vector<std::string> tables;
    auto sql = R"sql(
        SELECT tbl_name
        FROM   [sqlite_master]
        WHERE  [type] = 'table'
                AND     [tbl_name] LIKE 'ec\_%' ESCAPE '\')sql";
    auto stmt = conn.GetCachedStatement(sql);
    while (stmt->Step() == BE_SQLITE_ROW) {
        tables.push_back(stmt->GetValueText(0));
    }
    std::sort(tables.begin(), tables.end());
    return tables;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string SqlTypeDetector::Join(std::vector<std::string> const& v, const std::string sep) {
    std::string init = v.front();
    return std::accumulate(v.begin() + 1, v.end(), init,
                           [&](std::string& s, const std::string& piece) -> decltype(auto) {
                               return s.append(sep).append(piece);
                           });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SqlTypeDetector::Validate(RE2 const& re) {
    if (!re.ok()) {
        LOG.errorv("REGEX: %s\n", re.pattern().c_str());
        LOG.errorv("ERROR: %s\n", re.error().c_str());
    }
    BeAssert(re.ok());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SqlTypeDetector::SetupRegex(DbCR conn, bool useDataCRUD) {
    RE2::Options opts;
    opts.set_never_capture(true);
    opts.set_case_sensitive(false);
    opts.set_one_line(false);
    opts.set_log_errors(true);

    m_alterTableRegEx = std::make_unique<RE2>(
        R"(^\s*ALTER\s+TABLE\s+(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_dropTableRegEx = std::make_unique<RE2>(
        R"(^\s*DROP\s+TABLE\s+(IF\s+EXISTS)?\s+(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_dropIndexRegEx = std::make_unique<RE2>(
        R"(^\s*DROP\s+INDEX\s+(IF\s+EXISTS)?\s+(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_createTableRegEx = std::make_unique<RE2>(
        R"(^\s*CREATE\s+((TEMP|TEMPORARY)\s*)?TABLE\s+(IF\s+NOT\s+EXISTS)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_createIndexRegEx = std::make_unique<RE2>(
        R"(^\s*CREATE\s+((UNIQUE)\s*)?INDEX\s+(IF\s+NOT\s+EXISTS)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_createViewRegEx = std::make_unique<RE2>(
        R"(^\s*CREATE\s+((TEMP|TEMPORARY)\s*)?VIEW\s+(IF\s+NOT\s+EXISTS)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_pragmaRegEx = std::make_unique<RE2>(
        R"(^\s*PRAGMA\s+(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_insertRegEx = std::make_unique<RE2>(
        R"(^\s*INSERT\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_deleteRegEx = std::make_unique<RE2>(
        R"(^\s*DELETE\s+(FROM)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_updateRegEx = std::make_unique<RE2>(
        R"(^\s*UPDATE\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);

    const auto dataTables = Join(GetDataTables(conn), "|");
    m_dataInsertRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*INSERT\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", dataTables.c_str()).GetUtf8CP(), opts);
    m_dataDeleteRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*DELETE\s+(FROM)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", dataTables.c_str()).GetUtf8CP(), opts);
    m_dataUpdateRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*UPDATE\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", dataTables.c_str()).GetUtf8CP(), opts);

    const auto sysTables = Join(GetSystemTables(conn), "|");
    m_sysInsertRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*INSERT\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", sysTables.c_str()).GetUtf8CP(), opts);
    m_sysDeleteRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*DELETE\s+(FROM)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", sysTables.c_str()).GetUtf8CP(), opts);
    m_sysUpdateRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*UPDATE\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", sysTables.c_str()).GetUtf8CP(), opts);

    Validate(*m_alterTableRegEx);
    Validate(*m_dropTableRegEx);
    Validate(*m_dropIndexRegEx);
    Validate(*m_createTableRegEx);
    Validate(*m_createIndexRegEx);
    Validate(*m_alterTableRegEx);
    Validate(*m_createViewRegEx);
    Validate(*m_pragmaRegEx);
    Validate(*m_insertRegEx);
    Validate(*m_deleteRegEx);
    Validate(*m_updateRegEx);
    Validate(*m_dataInsertRegEx);
    Validate(*m_dataDeleteRegEx);
    Validate(*m_dataUpdateRegEx);
    Validate(*m_sysInsertRegEx);
    Validate(*m_sysDeleteRegEx);
    Validate(*m_sysUpdateRegEx);
}
