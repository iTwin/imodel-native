/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeSQLite/Profiler.h>

BEGIN_UNNAMED_NAMESPACE

typedef bvector<TxnMonitorP> TxnMonitors;
static TxnMonitors s_monitors;
static T_OnCommitCallback s_onCommitCallback = nullptr;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct PullMergeConf final {
    static constexpr int VER = 0x1;

private:
    static constexpr auto JVersion = "version";
    static constexpr auto JStartTxnId = "start_txn_id";
    static constexpr auto JEndTxnId = "end_txn_id";
    static constexpr auto JMergeStage = "merge_stage";
    static constexpr auto JKey = "pull_merge_conf";
    static constexpr auto JInProgressRebaseTxnId = "inprogress_rebase_txn_id";

    TxnManager::TxnId _endTxnId;
    TxnManager::TxnId _startTxnId;
    TxnManager::TxnId _inProgressRebaseTxnId;
    TxnManager::PullMergeStage _mergeStage = TxnManager::PullMergeStage::None;

public:
    TxnManager::TxnId GetEndTxnId() const { return _endTxnId; }
    TxnManager::TxnId GetStartTxnId() const { return _startTxnId; }
    TxnManager::TxnId GetInProgressRebaseTxnId() const {
        return _mergeStage == TxnManager::PullMergeStage::Rebasing ? _inProgressRebaseTxnId : TxnManager::TxnId(); }
    TxnManager::PullMergeStage GetMergeStage() const { return _mergeStage; }
    bool IsMergingRemoteChanges() const { return _mergeStage == TxnManager::PullMergeStage::Merging; }
    bool IsRebasingLocalChanges() const { return _mergeStage == TxnManager::PullMergeStage::Rebasing; }
    bool InProgress() const {return _mergeStage != TxnManager::PullMergeStage::None; }
    DbResult Save(DbR db) {
        BeJsDocument doc;
        doc.SetEmptyObject();
        doc[PullMergeConf::JVersion] = VER;
        doc[PullMergeConf::JMergeStage] = static_cast<int>(_mergeStage);
        doc[PullMergeConf::JEndTxnId] = static_cast<int64_t>(_endTxnId.GetValue());
        doc[PullMergeConf::JInProgressRebaseTxnId] = static_cast<int64_t>(_inProgressRebaseTxnId.GetValue());
        return db.SaveBriefcaseLocalValue(PullMergeConf::JKey, doc.Stringify());
    }
    PullMergeConf& SetInProgressRebaseTxnId(TxnManager::TxnId id) { 
        if (id.IsValid() && _mergeStage == TxnManager::PullMergeStage::Rebasing )
            _inProgressRebaseTxnId = id;
        else
            _inProgressRebaseTxnId = TxnManager::TxnId();
        return *this;
    }
    PullMergeConf& ResetInProgressRebaseTxnId() {
        _inProgressRebaseTxnId = TxnManager::TxnId();
        return *this;
    }
    PullMergeConf& SetEndTxnId(TxnManager::TxnId id) { _endTxnId = id; return *this; }
    PullMergeConf& SetStartTxnId(TxnManager::TxnId id) { _startTxnId = id; return *this; }
    PullMergeConf& SetMergeStage(TxnManager::PullMergeStage stage) { _mergeStage = stage; return *this; }

    static DbResult Remove(DbR db) { return db.DeleteBriefcaseLocalValue(PullMergeConf::JKey); }
    static PullMergeConf Load(DbCR db) {
        Utf8String val;
        if (db.QueryBriefcaseLocalValue(val, PullMergeConf::JKey) != BE_SQLITE_ROW) {
            return PullMergeConf();
        }

        BeJsDocument doc(val.c_str());
        if (doc.hasParseError()){
            return PullMergeConf();
        }

        PullMergeConf info;
        auto ver = doc[PullMergeConf::JVersion].GetInt(PullMergeConf::VER);
        if (ver >= 0x1) {
            info._mergeStage = static_cast<TxnManager::PullMergeStage>(doc[PullMergeConf::JMergeStage].GetInt(static_cast<int>(TxnManager::PullMergeStage::None)));
            info._endTxnId  = TxnManager::TxnId(doc[PullMergeConf::JEndTxnId].GetUInt64(0));
            info._startTxnId  = TxnManager::TxnId(doc[PullMergeConf::JStartTxnId].GetUInt64(0));
            info._inProgressRebaseTxnId = TxnManager::TxnId(doc[PullMergeConf::JInProgressRebaseTxnId].GetUInt64(0));
        }
        return info;
    }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangesBlobHeader {
    enum { DB_Signature06 = 0x0600 };
    uint32_t m_signature; // write this so we can detect errors on read
    uint32_t m_size;

    ChangesBlobHeader(uint32_t size) {m_signature = DB_Signature06; m_size = size;}
    ChangesBlobHeader(SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*)this, sizeof(*this), actuallyRead);}
};

enum : uint64_t {
     K = 1024,
     MEG = K * K,
     GIG = K * MEG,
     MAX_REASONABLE_TXN_SIZE = 200 * MEG,
     MAX_TXN_SIZE = (3 * GIG) - 100, // uncompressed
     MAX_TXN_ROW_LENGTH = (2 * GIG) - 1, // compressed max (this is max limit of SQLite)
};


//=======================================================================================
// Undo/redo, save/abandon changes, and merging revisions all end up calling
// TxnManager::_OnCommit. From there, if everything goes well we want to notify of
// changes to geometry guids and geometric elements; we want to clear accumulated changes
// regardless of the outcome.
// @bsistruct
//=======================================================================================
struct ModelChangesScope {
    TxnManager& m_mgr;
    explicit ModelChangesScope(TxnManager& mgr) : m_mgr(mgr) {}
    ~ModelChangesScope() { m_mgr.ClearModelChanges(); }
};

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
uint64_t TxnManager::GetMaxReasonableTxnSize() { return MAX_REASONABLE_TXN_SIZE; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::UndoChangeSet::ConflictResolution TxnManager::UndoChangeSet::_OnConflict(ConflictCause cause, BeSQLite::Changes::Change change) {
    const auto opcode = change.GetOpcode();

    if (cause == ConflictCause::NotFound) {
        if (opcode == DbOpcode::Delete)      // a delete that is already gone.
            return ConflictResolution::Skip; // This is caused by propagate delete on a foreign key. It is not a problem.
        if (opcode == DbOpcode::Update)
            return ConflictResolution::Skip; // caused by inserting row and then updating it in the same txn and then undoing the txn. It's not a problem.
    } else if (ConflictCause::Data == cause) {
        return ConflictResolution::Skip; // caused by inserting row and then updating it in the same txn and then undoing the txn. It's not a problem.
    }

    BeAssert(false);
    return ConflictResolution::Skip;
}

/*---------------------------------------------------------------------------------**//**
* We keep a statement cache just for TxnManager statements
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr TxnManager::GetTxnStatement(Utf8CP sql) const {
    CachedStatementPtr ptr;
    m_stmts.GetPreparedStatement(ptr, *m_dgndb.GetDbFile(), sql);
    return ptr;
}

/*---------------------------------------------------------------------------------**//**
* Save an array of bytes representing a change for the current Txn into the DGN_TABLE_Txns table
* in the DgnDb. This also compresses the changes.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::SaveTxn(ChangeSetCR changeSet, Utf8CP operation, TxnType txnType) {
    if (0 == changeSet.GetSize()) {
        BeAssert(false);
        LOG.error("called SaveChangeSet for empty changeset");
        return BE_SQLITE_ERROR;
    }

    if (changeSet.GetSize() > MAX_REASONABLE_TXN_SIZE) {
        //BeAssert(changeSet.GetSize() < MAX_REASONABLE_TXN_SIZE); // if you hit this, something is wrong in your application. You should call commit more frequently.
        LOG.warningv("changeset size %" PRIu64 " exceeds recommended limit. Please investigate.", changeSet.GetSize());
    }

    if (changeSet.GetSize() > MAX_TXN_SIZE) {
        LOG.fatalv("changeset size %" PRIu64 " exceeds maximum. Panic stop to avoid loss! You must now abandon this briefcase.", changeSet.GetSize());
        // return error so besqlite StopSavepoint() can deal with it cleanly and throw exception.
        return BE_SQLITE_ERROR;
    }

    // Note: column in Db is named "IsSchemaChange", but we store TxnType there.
    enum Column : int {Id=1,Deleted=2,Grouped=3,Operation=4,TxnType=5,Change=6};
    CachedStatementPtr stmt = GetTxnStatement("INSERT INTO " DGN_TABLE_Txns "(Id,Deleted,Grouped,Operation,IsSchemaChange,Change) VALUES(?,?,?,?,?,?)");
    stmt->BindInt64(Column::Id, m_curr.GetValue());
    stmt->BindInt(Column::Deleted,  false);
    if (nullptr != operation)
        stmt->BindText(Column::Operation, operation, Statement::MakeCopy::No);

    if (txnType == TxnType::EcSchema)
        stmt->BindNull(Column::TxnType); // for backwards compatibility, since it used to be a Boolean and we don't want old versions to interpret this as DDL
    else
        stmt->BindInt(Column::TxnType, (int) txnType);

    // if we're in a multi-txn operation, and if the current TxnId is greater than the first txn, mark it as "grouped"
    stmt->BindInt(Column::Grouped, !m_multiTxnOp.empty() && (m_curr > m_multiTxnOp.back()));

    m_snappyTo.Init();
    uint32_t csetSize = (uint32_t) changeSet.GetSize();
    ChangesBlobHeader header(csetSize);
    m_snappyTo.Write((Byte const*) &header, sizeof(header));
    for (auto const& chunk : changeSet.m_data.m_chunks)
        m_snappyTo.Write(chunk.data(), (uint32_t) chunk.size());

    uint32_t zipSize = m_snappyTo.GetCompressedSize();
    DbResult rc;
    if (0 < zipSize)
        {
        if (1 == m_snappyTo.GetCurrChunk())
            rc = stmt->BindBlob(Column::Change, m_snappyTo.GetChunkData(0), zipSize, Statement::MakeCopy::No);
        else
            rc = stmt->BindZeroBlob(Column::Change, zipSize); // more than one chunk

        if (BE_SQLITE_OK != rc)
            {
            BeAssert(false);
            return rc;
            }
        }

    if (changeSet.GetSize() > MAX_REASONABLE_TXN_SIZE/2)
        LOG.infov("Saving large changeset. Size=%" PRIuPTR ", Compressed size=%" PRIu32, changeSet.GetSize(), m_snappyTo.GetCompressedSize());

    rc = stmt->Step();
    if (BE_SQLITE_DONE != rc)
        {
        LOG.errorv("SaveChangeSet failed: %s", BeSQLiteLib::GetErrorName(rc));
        BeAssert(false);
        return rc;
        }

    m_curr.Increment();
    if (1 == m_snappyTo.GetCurrChunk())
        return BE_SQLITE_OK;

    rc = m_snappyTo.SaveToRow(m_dgndb, DGN_TABLE_Txns, "Change", m_dgndb.GetLastInsertRowId());
    if (BE_SQLITE_OK != rc)
        {
        LOG.errorv("SaveChangeSet failed to save to row: %s", BeSQLiteLib::GetErrorName(rc));
        BeAssert(false);
        return rc;
        }

    if (txnType != TxnType::Data)
        Initialize(); // schema changes are never undoable, initialize the TxnManager to start a new session

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* Read the description of a Txn
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetTxnDescription(TxnId rowid) const {
    Statement stmt(m_dgndb, "SELECT Operation FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt.BindInt64(1, rowid.GetValue());
    const auto hasRow = stmt.Step() == BE_SQLITE_ROW;
    if (!hasRow || stmt.IsColumnNull(0))
        return "";

    BeJsDocument doc(stmt.GetValueText(0));
    if (doc.hasParseError() || doc.isNull() || !doc.isObject() || !doc.isStringMember("description") )
        return stmt.GetValueText(0);
        
    return doc["description"].asString();
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnType TxnManager::GetTxnType(TxnId rowid) const {
    Statement stmt(m_dgndb, "SELECT IsSchemaChange FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt.BindInt64(1, rowid.GetValue());
    if (stmt.Step() != BE_SQLITE_ROW)
        return TxnType::Data;
    // for backwards compatibility, Null means EcSchema. Otherwise old versions will interpret it as DDL changes.
    return stmt.IsColumnNull(0) ? TxnType::EcSchema : (TxnType) stmt.GetValueInt(0);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::GetTxnProps(TxnId id, BeJsValue obj) const {
    Statement stmt(m_dgndb, "SELECT [Id], [Operation], [IsSchemaChange], [Deleted], [Grouped], strftime('%Y-%m-%dT%H:%M:%S', [Time]) FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt.BindInt64(1, id.GetValue());
    if (stmt.Step() != BE_SQLITE_ROW)
        return false;

    obj.SetEmptyObject();
    obj["id"] = stmt.GetValueId<BeInt64Id>(0).ToHexStr();
    auto nextId = QueryNextTxnId(id);
    if (nextId.IsValid())
        obj["nextId"] = BeInt64Id(nextId.GetValue()).ToHexStr();

    auto previousId = QueryPreviousTxnId(id);
    if (previousId.IsValid())
        obj["prevId"] = BeInt64Id(previousId.GetValue()).ToHexStr();

    auto props = obj["props"];
    props.SetEmptyObject();        
    if (!stmt.IsColumnNull(1)){
        auto jsonOrStr = stmt.GetValueText(1);
        BeJsDocument doc(jsonOrStr);
        if (doc.hasParseError() || doc.isNull() || !doc.isObject())
            props["description"] = jsonOrStr;
        else
            doc.SaveTo(props);
    }

    if (stmt.IsColumnNull(2))
        obj["type"] = "Schema";
    else {
        const auto type = static_cast<TxnType>(stmt.GetValueInt(2));
        if (type == TxnType::Data)
            obj["type"] = "Data";
        else if (type == TxnType::Ddl)
            obj["type"] = "Ddl";
        else if (type == TxnType::EcSchema)
            obj["type"] = "EcSchema";
    }
    obj["reversed"] = stmt.GetValueBoolean(3);
    obj["grouped"] = stmt.GetValueBoolean(4);
    obj["timestamp"] = stmt.GetValueText(5);
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::IsMultiTxnMember(TxnId rowid) const {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Grouped FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt->BindInt64(1, rowid.GetValue());
    return stmt->Step() != BE_SQLITE_ROW ? false : stmt->GetValueBoolean(0);
}

/*---------------------------------------------------------------------------------**//**
 return true if there are any (non-reversed) txns in the db
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::HasPendingTxns() const {
    Statement stmt(m_dgndb, "SELECT Id FROM " DGN_TABLE_Txns " WHERE Deleted=0");
    return stmt.Step() == BE_SQLITE_ROW;
}

/** Get the highest used (not reversed) TxnId in the Txns table */
TxnManager::TxnId TxnManager::GetLastTxnId() {
    Statement stmt(m_dgndb, "SELECT MAX(Id) FROM " DGN_TABLE_Txns " WHERE Deleted=0");
    DbResult result = stmt.Step();
    UNUSED_VARIABLE(result);
    BeAssert(result == BE_SQLITE_ROW);

    return stmt.GetValueInt64(0);
}

void TxnManager::Initialize() {
    m_action = TxnAction::None;
    TxnId last = GetLastTxnId(); // this is where we left off last session
    m_curr = TxnId(SessionId(last.GetSession().GetValue()+1), 0); // increment the session id, reset to index to 0.
    m_reversedTxn.clear();
}

/**
 * Increment the current SessionId by 1, so that all current Txns will no longer be undoable.
 */
void TxnManager::StartNewSession() {
    m_curr = TxnId(SessionId(m_curr.GetSession().GetValue()+1), 0);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnManager(DgnDbR dgndb) : m_dgndb(dgndb), m_stmts(20), m_rlt(*this), m_initTableHandlers(false), m_modelChanges(*this) {
    m_dgndb.SetChangeTracker(this);
    Initialize();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::InitializeTableHandlers() {
    if (m_initTableHandlers)
        return BE_SQLITE_OK;

    for (auto table : m_tables)
        table->_Initialize();

    m_initTableHandlers = true;

    DbResult result = m_dgndb.SaveChanges(); // "Commit" the creation of temp tables, so that a subsequent call to AbandonChanges will not un-create them.
    if (result != BE_SQLITE_OK) {
        BeAssert(false);
        return result;
    }

    return BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::BeginTrackingRelationship(ECN::ECClassCR relClass) {
    if (m_dgndb.IsReadonly() || !relClass.IsRelationshipClass())
        return;

    Utf8CP tableName;
    bool isTablePerHierarchy;
    if (SUCCESS != ChangeSummary::GetMappedPrimaryTable(tableName, isTablePerHierarchy, relClass, m_dgndb))
        return;

    if (nullptr != FindTxnTable(tableName))
        return; //  already tracking this table

    auto handler = new dgn_TxnTable::UniqueRelationshipLinkTable(*this, tableName);
    m_tablesByName.Insert(handler->_GetTableName(), handler); // takes ownership of handler, NOTE: use tableName from Utf8String in handler, not local variable!
    m_tables.push_back(handler);
}

/*---------------------------------------------------------------------------------**//**
* add a new TxnTable to this TxnManager. TxnTables are responsible for reacting to changes to a given SQLite table, by name.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::AddTxnTable(DgnDomain::TableHandler* tableHandler) {
    // we can get called multiple times with the same tableHandler. Ignore all but the first one.
    TxnTablePtr table = tableHandler->_Create(*this);
    if (m_tablesByName.Insert(table->_GetTableName(), table).second)
        m_tables.push_back(table.get()); // order matters for the calling sequence, so we have to store it both sorted by name and insertion order
}

/*---------------------------------------------------------------------------------**/ /**
* Find a TxnTable by name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnTable* TxnManager::FindTxnTable(Utf8CP tableName) const {
    auto it = m_tablesByName.find(tableName);
    return it != m_tablesByName.end() ? it->second.get() : NULL;
}

/*---------------------------------------------------------------------------------**/ /**
* Get the TxnTable that handles changes to the dgn_Element table
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Element& TxnManager::Elements() const {
    BeAssert(m_initTableHandlers);
    return *(dgn_TxnTable::Element*)FindTxnTable(dgn_TxnTable::Element::MyTableName());
}

/*---------------------------------------------------------------------------------**/ /**
* Get the TxnTable that handles changes to the dgn_ElementDrivesElement table
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::ElementDep& TxnManager::ElementDependencies() const {
    BeAssert(m_initTableHandlers);
    return *(dgn_TxnTable::ElementDep*)FindTxnTable(dgn_TxnTable::ElementDep::MyTableName());
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Model& TxnManager::Models() const {
    BeAssert(m_initTableHandlers);
    return *(dgn_TxnTable::Model*)FindTxnTable(dgn_TxnTable::Model::MyTableName());
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::QueryPreviousTxnId(TxnId curr) const {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Id FROM " DGN_TABLE_Txns " WHERE Id<? ORDER BY Id DESC LIMIT 1");
    stmt->BindInt64(1, curr.GetValue());

    auto rc = stmt->Step();
    return (rc != BE_SQLITE_ROW) ? TxnId() : TxnId(stmt->GetValueInt64(0));
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::QueryNextTxnId(TxnId curr) const {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Id FROM " DGN_TABLE_Txns " WHERE Id>? ORDER BY Id LIMIT 1");
    stmt->BindInt64(1, curr.GetValue());

    auto rc = stmt->Step();
    return (rc != BE_SQLITE_ROW) ? TxnId() : TxnId(stmt->GetValueInt64(0));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::BeginMultiTxnOperation()
    {
    m_multiTxnOp.push_back(GetCurrentTxnId());
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::EndMultiTxnOperation()
    {
    if (m_multiTxnOp.empty())
        {
        BeAssert(0);
        return DgnDbStatus::NoMultiTxnOperation;
        }

    m_multiTxnOp.pop_back();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::GetMultiTxnOperationStart()
    {
    return m_multiTxnOp.empty() ? GetSessionStartId() : m_multiTxnOp.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TxnManager::DoPropagateChanges(ChangeTracker& tracker) {    
    SetandRestoreIndirectChanges _v(tracker);
    for (auto table : m_tables) {
        table->_PropagateChanges();
        if (HasFatalError()) {
            LOG.error("fatal propagation error");
            break;
        }
    }

    return HasFatalError() ? BSIERROR : BSISUCCESS;
}

#define TABLE_NAME_STARTS_WITH(NAME) (0==strncmp(NAME, tableName, sizeof(NAME)-1))
/*---------------------------------------------------------------------------------**//**
* When journaling changes, SQLite calls this method to determine whether changes to a specific table are eligible or not.
* @note tables with no primary key are skipped automatically.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TrackChangesForTable TxnManager::_FilterTable(Utf8CP tableName) {
    // Skip these tables - they hold redundant data that will be automatically updated when the changeset is applied
    return (
               TABLE_NAME_STARTS_WITH(BEDB_TABLE_Local) ||
               TABLE_NAME_STARTS_WITH(DGN_TABLE_Txns) ||
               TABLE_NAME_STARTS_WITH(DGN_VTABLE_SpatialIndex) ||
               TABLE_NAME_STARTS_WITH(DGN_TABLE_Rebase) ||
               DgnSearchableText::IsUntrackedFts5Table(tableName)
            ) ? TrackChangesForTable::No : TrackChangesForTable::Yes;
}
#undef TABLE_NAME_STARTS_WITH

/*---------------------------------------------------------------------------------**//**
* Add all changes to the TxnTables. TxnTables store information about the changes in their own state
* if they need to hold on to them so they can react after the changeset is applied.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnValidateChanges(ChangeStreamCR changeStream) {
    BeAssert(!m_dgndb.IsReadonly());

    Changes changes(changeStream, false);
    TxnTable* txnTable = 0;
    Utf8String currTable;

    // Walk through each changed row in the changeset. They are ordered by table, so we know that all changes to one table will be seen
    // before we see any changes to another table.
    for (auto change : changes) {
        Utf8CP tableName;
        int nCols, indirect;
        DbOpcode opcode;

        DbResult rc = change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        if (rc != BE_SQLITE_OK) {
            LOG.error("invalid change in changeset");
            BeAssert(false && "invalid change in changeset");
            continue;
        }

        if (0 != BeStringUtilities::StricmpAscii(currTable.c_str(), tableName)) { // changes within a changeset are grouped by table
            currTable = tableName;
            txnTable = FindTxnTable(tableName);
        }

        if (nullptr == txnTable)
            continue; // this table does not have a TxnTable for it, skip it

        switch (opcode) {
        case DbOpcode::Delete:
            txnTable->_OnValidateDelete(change);
            break;
        case DbOpcode::Insert:
            txnTable->_OnValidateAdd(change);
            break;
        case DbOpcode::Update:
            txnTable->_OnValidateUpdate(change);
            break;
        default:
            BeAssert(false);
        }
    }

    ProcessModelChanges();
}

/*---------------------------------------------------------------------------------**/ /**
* The supplied changeset was just applied to the database. That means the the database now potentially reflects a different
* state than the in-memory objects for the affected tables. Use the changeset to send _OnAppliedxxx events to the TxnTables for each changed row,
* so they can update in-memory state as necessary.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnChangeSetApplied(ChangeStreamCR changeStream, bool invert) {
    if (!m_initTableHandlers) // won't do anything if we don't have table handlers
        return;

    Changes changes(changeStream, invert);
    Utf8String currTable;
    TxnTable* txnTable = 0;

    // Walk through each changed row in the changeset. They are ordered by table, so we know that all changes to one table will be seen
    // before we see any changes to another table.
    for (auto change : changes) {
        Utf8CP tableName;
        int nCols, indirect;
        DbOpcode opcode;

        DbResult rc = change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        BeAssert(rc == BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);

        if (0 != BeStringUtilities::StricmpAscii(currTable.c_str(), tableName)) { // changes within a changeset are grouped by table
            currTable = tableName;
            txnTable = FindTxnTable(tableName);
        }

        if (nullptr == txnTable)
            continue; // this table does not have a TxnTable for it, skip it

        switch (opcode) {
            case DbOpcode::Delete:
                txnTable->_OnAppliedDelete(change);
                break;
            case DbOpcode::Insert:
                txnTable->_OnAppliedAdd(change);
                break;
            case DbOpcode::Update:
                txnTable->_OnAppliedUpdate(change);
                break;
            default:
                BeAssert(false);
        }
    }

    ProcessModelChanges();
}

/*---------------------------------------------------------------------------------**//**
* A changeset is about to be committed (or, in the case of "what if" testing, look like it is committed). Let each
* TxnTable prepare its state
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnBeginValidate() {
    if (!m_initTableHandlers) {
        BeAssert(false); // validation cannot happen without table handlers initialized.
        return;
    }
    m_fatalValidationError = false;
    m_txnErrors = 0;
    m_action = TxnAction::Commit;
    for (auto table : m_tables)
        table->_OnValidate();
    CallJsTxnManager("_onBeginValidate");
}

/*---------------------------------------------------------------------------------**/ /**
* A changeset was just be committed. Let each TxnTable clean up its state. Also clear the validation errors.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnEndValidate() {
    if (!m_initTableHandlers) {
        BeAssert(false); // validation cannot happen without table handlers initialized.
        return;
    }
    NotifyModelChanges();

    CallJsTxnManager("_onEndValidate");
    for (auto table : m_tables)
        table->_OnValidated();

    m_fatalValidationError = false;
    m_txnErrors = 0;
    m_action = TxnAction::None;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::SetOnCommitCallback(T_OnCommitCallback cb) {
    LOG.error("setting commit callback - For tests only!");
    s_onCommitCallback = cb;
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnRollback(ChangeStreamCR changeSet) {
    m_dgndb.Elements().ClearCache(); // we can't rely on the elements in the cache after rollback, just abandon them
    if (!m_initTableHandlers) {
        m_dgndb.Models().ClearCache();
        return;
    }

    // BeAssert(m_action == TxnAction::None); //its merge in case of applychangeset fails.
    AutoRestore<TxnAction> _v(&m_action, TxnAction::Abandon);
    OnChangeSetApplied(changeSet, true);
    NotifyModelChanges();
}

/*---------------------------------------------------------------------------------**//**
* Called from Db::SaveChanges or Db::AbandonChanges when the TxnManager change tracker has changes.
* This method creates a changeset from the change tracker.
* If this is a "cancel", it rolls back the current Txn, and calls _OnAppliedxxx methods on all affected TxnTables.
* If this is a commit, it calls the _OnValidatexxx methods for the TxnTables, and then calls "_PropagateChanges"
* It saves the resultant changeset (the combination of direct changes plus indirect changes) as a Txn in the database.
* The Txn may be undone in this session via the "ReverseTxn" method.
* When this method returns, the entire transaction is committed in BeSQLite. In this manner, it is impossible to make
* changes to the database that aren't also saved in the dgn_Txn table.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::OnCommitStatus TxnManager::_OnCommit(bool isCommit, Utf8CP operation) {
    auto conf = PullMergeConf::Load(m_dgndb);

    // During PullMergeEnd() we donot allow COMMIT but we do let be_Local which is untracked to be saved.
    if (conf.IsRebasingLocalChanges()) {
        if (isCommit) {
            if (HasDataChanges()) {
                LOG.error("Saving changes are not allowed when rebasing local changes");
                return OnCommitStatus::RebaseInProgress;
            } else {
                return OnCommitStatus::NoChanges;
            }
        } else {
            // if (conf.GetInProgressRebaseTxnId().IsValid()) {
            //     LOG.error("Abort changes are not allowed when rebasing local changes");
            //     return OnCommitStatus::RebaseInProgress;                
            // }
        }
    }

    ModelChangesScope v_v_v_(*this);
    DdlChanges ddlChanges = std::move(m_ddlChanges);

    UndoChangeSet currentChanges;
    if (HasDataChanges()) {
        DbResult result = currentChanges.FromChangeTrack(*this);

        Restart();  // Clear the change tracker since we copied any changes to currentChanges

        if (BE_SQLITE_OK != result) {
            LOG.errorv("failed to create a data Changeset: %s", BeSQLiteLib::GetErrorName(result));
            BeAssert(false && "currentChanges.FromChangeTrack failed");
            return OnCommitStatus::Abort;
        }
    }

    if (s_onCommitCallback != nullptr) { // allow a test to inject a failure
        if (CallbackOnCommitStatus::Continue != s_onCommitCallback(*this, isCommit, operation, currentChanges, ddlChanges))
            return OnCommitStatus::Abort;
    }

    if (currentChanges._IsEmpty() && ddlChanges._IsEmpty())
        return OnCommitStatus::NoChanges;

    if (!isCommit) { // this is a call to AbandonChanges, perform the rollback and notify table handlers
        DbResult rc = m_dgndb.ExecuteSql("ROLLBACK");
        if (rc != BE_SQLITE_OK)
            return OnCommitStatus::Abort;

        if (!currentChanges._IsEmpty())
            OnRollback(currentChanges);

        return OnCommitStatus::Completed; // we've already done the rollback, tell BeSQLite not to try to do it
    }

    // NOTE: you can't delete reversed Txns before this line, because on rollback and they come back! That's OK,
    // just leave them reversed and they'll get thrown away on the next commit (or reinstated.)
    DeleteReversedTxns(); // these Txns are no longer reachable.

    // Following is a function to free memory as soon as possible after we separate changes
    auto separateDataAndSchemaChanges = [](
        ChangeSet& in,
        ChangeStream& outData,
        ChangeStream& outSchema) {

        ChangeGroup dataChangeGroup;
        ChangeGroup schemaChangeGroup;
        auto rc = ChangeGroup::FilterIfElse(in,
            [&](Changes::Change const& change) {
            auto const& tbl = change.GetTableName();
            return tbl.StartsWithIAscii("ec_") || tbl.StartsWithIAscii("be_Props");
        }, schemaChangeGroup, dataChangeGroup);

        in.Clear();
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("failed to filter changeset out into schema & data: %s", BeSQLiteLib::GetErrorName(rc));
            return rc;
        }

        rc = outData.FromChangeGroup(dataChangeGroup);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("failed to read changes from data change group: %s", BeSQLiteLib::GetErrorName(rc));
            return rc;
        }

        rc = outSchema.FromChangeGroup(schemaChangeGroup);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("failed to read changes from schema change group: %s", BeSQLiteLib::GetErrorName(rc));
            return rc;
        }
        return rc;
    };

    //! We need to preciously separate out data and schema changeset as it require by rebase.
    //! when we reverse changes schema changes are also reversed.
    ChangeSet dataChanges;
    ChangeSet schemaChanges;
    auto rc = separateDataAndSchemaChanges(currentChanges, dataChanges, schemaChanges);
    if (rc != BE_SQLITE_OK) {
        return OnCommitStatus::Abort;
    }

    const auto propagateChanges = !dataChanges._IsEmpty() && m_initTableHandlers;
    if (propagateChanges) {
        // we cannot propagate changes without table handlers - happens for schema upgrades
        OnBeginValidate();
        OnValidateChanges(dataChanges);

        BentleyStatus status = PropagateChanges();   // Propagate to generate indirect changes
        if (SUCCESS != status) {
            LOG.error("propagate changes failed");
            return OnCommitStatus::Abort;
        }

        // This loop is due to the fact that when we propagate changes, we can dirty models.
        // Then, when we call OnValidateChanges below, it creates another change to the last-mod-time and geometry-GUIDs of the changed models.
        // We need to add that to the changeset too. This loop should never execute more than twice.
        while (HasDataChanges()) {  // do we have any indirect data changes captured in the tracker?
            UndoChangeSet propagatedIndirectChanges;
            DbResult result = propagatedIndirectChanges.FromChangeTrack(*this);
            if (BE_SQLITE_OK != result)
                {
                BeAssert(false && "propagatedIndirectChanges.FromChangeTrack failed");
                LOG.fatalv("failed to create indirect changeset: %s", BeSQLiteLib::GetErrorName(result));
                if (BE_SQLITE_NOMEM == result)
                    throw std::bad_alloc();
                return OnCommitStatus::Abort;
                }
            Restart();
            OnValidateChanges(propagatedIndirectChanges);

            // combine direct and indirect changes into a single dataChangeSet
            result = dataChanges.ConcatenateWith(propagatedIndirectChanges);
            if (BE_SQLITE_OK != result){
                LOG.errorv("failed to concatenate indirect changes: %s", BeSQLiteLib::GetErrorName(result));
                return OnCommitStatus::Abort;
            }
        }
    }

    if (!ddlChanges._IsEmpty()) {
        rc = SaveTxn(ddlChanges, operation, TxnType::Ddl);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("failed to save ddl changes: %s", BeSQLiteLib::GetErrorName(rc));
            return OnCommitStatus::Abort;
        }
    }

    if (!schemaChanges._IsEmpty()) {
        rc = SaveTxn(schemaChanges, operation, TxnType::EcSchema);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("failed to save schema Txn: %s", BeSQLiteLib::GetErrorName(rc));
            return OnCommitStatus::Abort;
        }
    }

    if (!dataChanges._IsEmpty()) {
        rc = SaveTxn(dataChanges, operation, TxnType::Data);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("failed to save data Txn: %s", BeSQLiteLib::GetErrorName(rc));
            return OnCommitStatus::Abort;
        }
    }

    // At this point, all of the changes to all tables have been applied. Tell TxnMonitors
    NotifyOnCommit();

    if (propagateChanges) {
        OnEndValidate();
    }

    return OnCommitStatus::Commit;
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::NotifyOnCommit() {
    // At this point, all of the changes to all tables have been applied. Tell TxnMonitors
    if (!m_inProfileUpgrade) {
        CallJsTxnManager("_onCommit");
        CallMonitors([&](TxnMonitor& monitor) { monitor._OnCommit(*this); });
    }
}
/*---------------------------------------------------------------------------------**//**
* called after the commit or cancel operation is complete
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::_OnCommitted(bool isCommit, Utf8CP) {
    if (isCommit && !m_inProfileUpgrade) { // only notify on commit, and not for profile upgrades
        CallJsTxnManager("_onCommitted");
        CallMonitors([&](TxnMonitor& monitor) { monitor._OnCommitted(*this); });
    }
}

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangesetStatus TxnManager::MergeDdlChanges(ChangesetPropsCR revision, ChangesetFileReader& changeStream)  {
    bool containsSchemaChanges;
    DdlChanges ddlChanges;
    DbResult result = changeStream.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
    if (result != BE_SQLITE_OK)     {
        BeAssert(false);
        return ChangesetStatus::ApplyError;
    }

    if (ddlChanges._IsEmpty())
        return ChangesetStatus::Success;

    result = ApplyDdlChanges(ddlChanges);
    if (BE_SQLITE_OK != result)
        return ChangesetStatus::ApplyError;

    return ChangesetStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus TxnManager::GetPendingTxnsSha256HashString(Utf8StringR hash, bool includeReversedTxns) const {
    if (!HasPendingTxns())
        return BentleyStatus::SUCCESS;

    Statement stmt;
    DbResult rc;
    if (includeReversedTxns)
        rc = stmt.Prepare(m_dgndb, R"sql(SELECT lower(hex(sha3_query('SELECT [Change] FROM [main].[dgn_Txns] ORDER BY [Id]', 256))))sql");
    else
        rc = stmt.Prepare(m_dgndb, R"sql(SELECT lower(hex(sha3_query('SELECT [Change] FROM [main].[dgn_Txns] WHERE [IsDeleted] = 0 ORDER BY [Id]', 256))))sql");
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("failed to prepare statement to compute hash: %s", BeSQLiteLib::GetErrorName(rc));
        return BentleyStatus::ERROR;
    }

    rc = stmt.Step();
    if (rc != BE_SQLITE_ROW) {
        LOG.errorv("failed to compute hash: %s", BeSQLiteLib::GetErrorName(rc));
        return BentleyStatus::ERROR;
    }

    hash = stmt.GetValueText(0);
    return BentleyStatus::SUCCESS;
}

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::Stash(BeFileNameCR stashRootDir, Utf8StringCR description, Utf8StringCR iModelId, bool resetBriefcase, BeJsValue out) {
    if (!m_dgndb.IsDbOpen()){
        m_dgndb.ThrowException("not a open db", BE_SQLITE_ERROR);
    }

    auto conf = PullMergeConf::Load(m_dgndb);
    if (conf.InProgress()) {
        m_dgndb.ThrowException("pull/merge in progress", BE_SQLITE_ERROR);
    }

    if (!m_dgndb.IsBriefcase()) {
        m_dgndb.ThrowException("not a briefcase db", BE_SQLITE_ERROR);
    }

    if (HasChanges()) {
        m_dgndb.ThrowException("there are uncommitted changes", BE_SQLITE_ERROR);
    }

    m_dgndb.SaveChanges("before stashing changes");
    
    auto briefcaseId = m_dgndb.GetBriefcaseId().GetValue();

    Utf8String val;
    BeGuid guid(true);

    auto rc = m_dgndb.QueryBriefcaseLocalValue(val, "bis_elementidsequence");
    if (BE_SQLITE_ROW != rc){
        m_dgndb.ThrowException("failed to query bis_elementidsequence", (int)BE_SQLITE_ERROR);
    }

    const auto elementIdSeq = BeInt64Id::FromString(val.c_str());

    rc = m_dgndb.QueryBriefcaseLocalValue(val, "ec_instanceidsequence");
    if (BE_SQLITE_ROW != rc) {
        m_dgndb.ThrowException("failed to query ec_instanceidsequence", (int)BE_SQLITE_ERROR);
    }

    const auto instanceIdSeq = BeInt64Id::FromString(val.c_str());

    BeJsDocument stashInfo;
    stashInfo.SetEmptyObject();
    stashInfo["id"] = guid.ToString().ToLower();
    stashInfo["iModelId"] = iModelId;
    stashInfo["briefcaseId"] = briefcaseId;    
    stashInfo["timestamp"] = DateTime::GetCurrentTime().ToString();
    stashInfo["description"] = description;
    val.clear();
    if (SUCCESS != GetPendingTxnsSha256HashString(val)) {
        m_dgndb.ThrowException("failed to compute hash via GetPendingTxnsSha256HashString()", (int)BE_SQLITE_ERROR);
    }

    stashInfo["hash"] = val;
    val.clear();
    rc = m_dgndb.QueryBriefcaseLocalValue(val, "parentChangeset");
    if (BE_SQLITE_ROW != rc) {
        rc = m_dgndb.QueryBriefcaseLocalValue(val, "ParentChangeSetId");
        if (BE_SQLITE_ROW != rc) {
            m_dgndb.ThrowException("failed to query parentChangeset or ParentChangeSetId", (int)BE_SQLITE_ERROR);
        }
        val.assign(SqlPrintfString("{\"id\": \"%s\"}", val.c_str()));
    }
    
    if (val.empty()) {
        m_dgndb.ThrowException("failed to query parentChangeset or ParentChangeSetId", (int)BE_SQLITE_ERROR);
    }
    
    stashInfo["parentChangeset"].From(BeJsDocument(val));
    if (instanceIdSeq.IsValid() || elementIdSeq.IsValid()) {
        auto idSeq = stashInfo["idSequences"];
        idSeq.SetEmptyObject();
        if (elementIdSeq.IsValid())
            idSeq["element"] = elementIdSeq.ToHexStr();

        if (instanceIdSeq.IsValid())
            idSeq["instance"] = instanceIdSeq.ToHexStr();
    }

    Db db;
    BeFileName lockFile(Utf8String(m_dgndb.GetTempFileBaseName() + "-locks"));
    BeFileName stashFile = stashRootDir;
    stashFile.AppendSeparator();
    stashFile.AppendUtf8(guid.ToString().ToLower().c_str());
    stashFile.AppendExtension(L"stash");

    rc = db.CreateNewDb(stashFile, Db::CreateParams(), guid);
    if (rc != BE_SQLITE_OK) {
        m_dgndb.ThrowException("fail to create stash file", (int)rc);
    }

    auto throwErrorAndDeleteStashFile = [&](Utf8CP error, int errCode) {
        if (db.IsDbOpen()) {
            db.AbandonChanges();
            db.CloseDb();
        }

        stashInfo.SetNull();
        if (stashFile.DoesPathExist())
            stashFile.BeDeleteFile();

        m_dgndb.ThrowException(error, (int)errCode);
    };

    rc = db.ExecuteDdl(R"sql(
            CREATE TABLE [txns] (
                [Id]	            INTEGER PRIMARY KEY NOT NULL,
                [Deleted]	        BOOLEAN,
                [Grouped]	        BOOLEAN,
                [Operation]	        TEXT,
                [IsSchemaChange]	BOOLEAN,
                [Time]	            TIMESTAMP,
                [Change]	        BLOB
            )
        )sql");
        
    if (rc != BE_SQLITE_OK) {
        throwErrorAndDeleteStashFile("failed to create Txn table", (int)rc);
    }

    rc = db.ExecuteDdl(R"sql(
        CREATE TABLE [locks](
            [id] INTEGER PRIMARY KEY NOT NULL,
            [state] INTEGER NOT NULL,
            [origin] INTEGER
        )
    )sql");

    if (rc != BE_SQLITE_OK) {
        throwErrorAndDeleteStashFile("failed to create locks table", (int)rc);
    }

    if (lockFile.DoesPathExist()) {
        rc = db.AttachDb(lockFile.GetNameUtf8().c_str(), "locks_attach_db");
        if (rc != BE_SQLITE_OK) {
            throwErrorAndDeleteStashFile("failed to attach locks file", (int)rc);
        }
       
        rc = db.ExecuteSql(R"sql(
            INSERT INTO [main].[locks] ([id], [state], [origin]) 
            SELECT [id], [state], [origin] FROM [locks_attach_db].[locks]
        )sql");
        
        if (rc != BE_SQLITE_OK) {
            throwErrorAndDeleteStashFile("failed to insert into locks table", (int)rc);
        }
    }

    Statement lockCountStmt;
    rc = lockCountStmt.Prepare(db, R"sql(SELECT count(*) FROM [main].[locks] WHERE [origin] = 0)sql");
    if (rc != BE_SQLITE_OK) {
        throwErrorAndDeleteStashFile("failed to prepare statement", (int)rc);
    }

    rc = lockCountStmt.Step();
    if (rc != BE_SQLITE_ROW) {
        throwErrorAndDeleteStashFile("failed to get acquired lock count", (int)rc);
    }

    stashInfo["acquiredLocks"] = lockCountStmt.GetValueInt(0);
    lockCountStmt.Finalize();

    rc = db.AttachDb(m_dgndb.GetDbFileName(), "imodel_attach_db");
    if (rc != BE_SQLITE_OK) {
        throwErrorAndDeleteStashFile("failed to attach stash file", (int)rc);
    }

    rc = db.ExecuteSql(R"sql(
        INSERT INTO [main].[txns] ([Id], [Deleted], [Grouped], [Operation], [IsSchemaChange], [Time], [Change]) 
        SELECT [Id], [Deleted], [Grouped], [Operation], [IsSchemaChange], [Time], [Change] FROM [imodel_attach_db].[dgn_Txns]
    )sql");

    if (rc != BE_SQLITE_OK) {
        throwErrorAndDeleteStashFile("failed to insert into stash file", (int)rc);
    }

    auto txnInfos = stashInfo["txns"];
    txnInfos.SetEmptyArray();
    for (TxnId curr = QueryNextTxnId(TxnId(0)); curr.IsValid(); curr = QueryNextTxnId(curr)) {
        GetTxnProps(curr, txnInfos.appendObject());
    }

    db.SaveBriefcaseLocalValue("$stash_info", stashInfo.Stringify());
    rc = db.SaveChanges();
    if (rc != BE_SQLITE_OK) {
        throwErrorAndDeleteStashFile("failed to save changes", (int)rc);
    }

    db.CloseDb();
    // ========
    out.From(stashInfo);

    if (resetBriefcase) {
        TxnId startTxnId = QueryNextTxnId(TxnId(0));
        TxnId endTxnId = GetCurrentTxnId();
        if (startTxnId < endTxnId) {
            OnBeforeUndoRedo(true);
            for (TxnId curr = QueryPreviousTxnId(endTxnId); curr.IsValid() && curr >= startTxnId; curr = QueryPreviousTxnId(curr)) {
                LOG.infov("Reversing TxnId: %s, Descr: %s", BeInt64Id(curr.GetValue()).ToHexStr().c_str(),GetTxnDescription(curr).c_str());
                auto rc = ApplyTxnChanges(curr, TxnAction::Reverse);
                if (BE_SQLITE_OK != rc) {
                    m_dgndb.AbandonChanges();
                    throwErrorAndDeleteStashFile("failed to reset briefcase", (int)rc);
                }
            }
        }
        DeleteAllTxns();
        m_curr = TxnId(SessionId(0), 0);
        m_action = TxnAction::None;
        m_reversedTxn.clear();        
    }
}

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangesetStatus TxnManager::MergeDataChanges(ChangesetPropsCR revision, ChangesetFileReader& changeStream, bool containsSchemaChanges, bool fastForward) {
    if (TrackChangesetHealthStats())
        Profiler::InitScope(*changeStream.GetDb(), "Apply Changeset", revision.GetChangesetId().c_str(), Profiler::Params(false, true));

    DbResult result = ApplyChanges(changeStream, TxnAction::Merge, containsSchemaChanges, false, fastForward);
    if (result != BE_SQLITE_OK) {
        if (changeStream.GetLastErrorMessage().empty())
            m_dgndb.ThrowException("failed to apply changes", result);
        else
            m_dgndb.ThrowException(changeStream.GetLastErrorMessage().c_str(), result);
    }
    changeStream.ClearLastErrorMessage();

    ChangesetStatus status = ChangesetStatus::Success;
    UndoChangeSet indirectChanges;

    if (status == ChangesetStatus::Success) {
        SaveParentChangeset(revision.GetChangesetId(), revision.GetChangesetIndex());

        if (status == ChangesetStatus::Success) {
            if (PullMergeConf::Load(m_dgndb).InProgress()) {
                result = m_dgndb.SaveChanges();
            }
            // Note: All that the above operation does is to COMMIT the current Txn and BEGIN a new one.
            // The user should NOT be able to revert the revision id by a call to AbandonChanges() anymore, since
            // the merged changes are lost after this routine and cannot be used for change propagation anymore.
            if (BE_SQLITE_OK != result) {
                LOG.fatalv("MergeDataChanges failed to save: %s", BeSQLiteLib::GetErrorName(result));
                BeAssert(false);
                status = ChangesetStatus::SQLiteError;
            }
        }
    }
    if (status != ChangesetStatus::Success) {
        // we were unable to merge the changes or save the revisionId, but the revision's changes were successfully applied. Back them out, plus any indirect changes.
        ChangeGroup changeGroup;
        changeStream.AddToChangeGroup(changeGroup);
        if (indirectChanges.IsValid()) {
            result = indirectChanges.AddToChangeGroup(changeGroup);
            BeAssert(result == BE_SQLITE_OK);
        }

        UndoChangeSet allChanges;
        allChanges.FromChangeGroup(changeGroup);
        result = ApplyChanges(allChanges, TxnAction::Reverse, containsSchemaChanges, true);
        BeAssert(result == BE_SQLITE_OK);
    }

    if (TrackChangesetHealthStats())
        SetChangesetHealthStatistics(revision);
    return status;
}

void TxnManager::SetChangesetHealthStatistics(ChangesetPropsCR revision) {
    const auto scope = Profiler::GetScope(m_dgndb);
    if (scope == nullptr) {
        BeAssert(false && "Profiler scope has not been defined");
        return;
    }

    auto stats = scope->GetDetailedSqlStats();
    stats["changeset_id"] = revision.GetChangesetId();
    stats["uncompressed_size_bytes"] = static_cast<int64_t>(revision.GetUncompressedSize());
    stats["sha1_validation_time_ms"] = static_cast<int64_t>(revision.GetSha1ValidationTime());
    m_changesetHealthStatistics[revision.GetChangesetId()] = stats.Stringify();
}

BeJsDocument TxnManager::GetAllChangesetHealthStatistics() const {
    BeJsDocument stats;
    auto changesets = stats["changesets"];
    for (const auto& [changesetId, stat] : m_changesetHealthStatistics)
        changesets.appendObject().From(stat);
    return stats;
}

BeJsDocument TxnManager::GetChangesetHealthStatistics(Utf8StringCR changesetId) const {
    if (const auto it = m_changesetHealthStatistics.find(changesetId); it != m_changesetHealthStatistics.end())
        return BeJsDocument(it->second.Stringify());
    
    LOG.infov("No changeset health statistics found for changeset id: %s", changesetId.c_str());
    return BeJsDocument();
}

/** throw an exception we are currently waiting for a changeset to be uploaded. */
void TxnManager::ThrowIfChangesetInProgress() {
    if (IsChangesetInProgress())
        m_dgndb.ThrowException("changeset creation is in progress", (int) ChangesetStatus::IsCreatingChangeset);
}

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReverseChangeset(ChangesetPropsCR changeset) {
    ThrowIfChangesetInProgress();

    if (m_dgndb.IsReadonly())
        m_dgndb.ThrowException("file is readonly", (int) ChangesetStatus::CannotMergeIntoReadonly);

    if (HasLocalChanges())
        m_dgndb.ThrowException("local changes present", (int) ChangesetStatus::HasLocalChanges);

    if (GetParentChangesetId() != changeset.GetChangesetId())
        m_dgndb.ThrowException("changeset out of order", (int) ChangesetStatus::ParentMismatch);

    if (changeset.ContainsDdlChanges(m_dgndb))
        m_dgndb.ThrowException("Cannot reverse a changeset containing schema changes", (int) ChangesetStatus::ReverseOrReinstateSchemaChanges);

    ChangesetFileReader changeStream(changeset.GetFileName(), &m_dgndb);

    // Skip the entire schema change set when reversing or reinstating - DDL and
    // the meta-data changes. Reversing meta data changes cause conflicts
    DbResult result = ApplyChanges(changeStream, TxnAction::Reverse, false, true);
    if (result != BE_SQLITE_OK)
        m_dgndb.ThrowException("Error applying changeset", (int) ChangesetStatus::ApplyError);

    SaveParentChangeset(changeset.GetParentId(), changeset.GetChangesetIndex() - 1);

    result = m_dgndb.SaveChanges();
    if (BE_SQLITE_OK != result)
        m_dgndb.ThrowException("unable to save changes", (int) ChangesetStatus::SQLiteError);
}

namespace
    {
    bool IsCacheTableNameInChangeset(const Changes& changes)
        {
        for (const auto& change: changes)
            {
            if (change.GetOpcode() == DbOpcode::Insert)  // we only care about update/delete operations
                continue;
            
            if (const auto tableName = change.GetTableName(); !tableName.empty() && 0 == strncmp(tableName.c_str(), "ec_cache_", 9))
                return true;
            }
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::RevertTimelineChanges(std::vector<ChangesetPropsPtr> changesetProps, bool skipSchemaChanges) {
    // If we are readonly, we can't revert changesets
    if (m_dgndb.IsReadonly())
        m_dgndb.ThrowException("file is readonly", (int) ChangesetStatus::CannotMergeIntoReadonly);

    // If we have changes in progress, we can't revert changesets
    if (IsChangesetInProgress())
        m_dgndb.ThrowException("changeset creation is in progress", (int) ChangesetStatus::IsCreatingChangeset);

    if(!skipSchemaChanges){
        // If we have changes in progress, we can't revert changesets
        if (HasPendingTxns())
            m_dgndb.ThrowException("pending transactions present", (int) ChangesetStatus::HasLocalChanges);

        // If we have changes in progress, we can't revert changesets
        if (HasChanges())
            m_dgndb.ThrowException("unsaved changes present", (int) ChangesetStatus::HasUncommittedChanges);
    }

    constexpr auto invert = true;

    // Revert the changesets in reverse order
    m_dgndb.AbandonChanges();
    m_dgndb.Elements().ClearCache();
    m_dgndb.Models().ClearCache();
    m_dgndb.ClearECDbCache();

    Utf8String currentChangesetId = GetParentChangesetId();
    for(auto& changesetProp : changesetProps) {
        changesetProp->ValidateContent(m_dgndb);
        ChangesetFileReader changeStream(changesetProp->GetFileName(), &m_dgndb);
        if (currentChangesetId != changesetProp->GetChangesetId()) {
            m_dgndb.ThrowException("changeset out of order", (int) ChangesetStatus::ParentMismatch);
        }
        currentChangesetId = changesetProp->GetParentId();
        auto dataApplyArgs = ApplyChangesArgs::Default()
            .SetInvert(invert)
            .SetIgnoreNoop(true)
            .SetFkNoAction(true)
            .ApplyOnlyDataChanges();

        auto rc = changeStream.ApplyChanges(m_dgndb, dataApplyArgs);
        if (rc != BE_SQLITE_OK) {
            m_dgndb.AbandonChanges();
            if (changeStream.GetLastErrorMessage().empty())
                m_dgndb.ThrowException("failed to apply changes", rc);
            else
                m_dgndb.ThrowException(changeStream.GetLastErrorMessage().c_str(), rc);
        }

        if (!skipSchemaChanges){
            auto schemaApplyArgs = ApplyChangesArgs::Default()
                .SetInvert(invert)
                .SetIgnoreNoop(true)
                .SetFkNoAction(IsCacheTableNameInChangeset(changeStream.GetChanges()))
                .ApplyOnlySchemaChanges();

            m_dgndb.Schemas().OnBeforeSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
            rc = changeStream.ApplyChanges(m_dgndb, schemaApplyArgs);
            if (rc != BE_SQLITE_OK) {
                m_dgndb.AbandonChanges();
                m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
                if (changeStream.GetLastErrorMessage().empty())
                    m_dgndb.ThrowException("failed to apply changes", rc);
                else
                    m_dgndb.ThrowException(changeStream.GetLastErrorMessage().c_str(), rc);
            }
            m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
        }
    }
}
/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangesetStatus TxnManager::MergeChangeset(ChangesetPropsCR changeset, bool fastForward) {
    ThrowIfChangesetInProgress();

    if (m_dgndb.IsReadonly())
        m_dgndb.ThrowException("file is readonly", (int) ChangesetStatus::CannotMergeIntoReadonly);

    auto conf = PullMergeConf::Load(m_dgndb);
    if (HasChanges() && !(conf.InProgress()))
        m_dgndb.ThrowException("unsaved changes present", (int) ChangesetStatus::HasUncommittedChanges);

    changeset.ValidateContent(m_dgndb);

    if (GetParentChangesetId() != changeset.GetParentId())
        m_dgndb.ThrowException("changeset out of order", (int) ChangesetStatus::ParentMismatch);

    ChangesetFileReader changeStream(changeset.GetFileName(), &m_dgndb);

    const bool containsDDLChanges = changeset.ContainsDdlChanges(m_dgndb);

    ChangesetStatus status;
    if (containsDDLChanges) {
        // Note: Schema changes may not necessary imply ddl changes. They could just be 'minor' ecschema/mapping changes.
        status = MergeDdlChanges(changeset, changeStream);
        if (ChangesetStatus::Success != status)
            return status;
    }

    /**
     * The value of this boolean variable is determined by checking if the changeset
     * contains DDL changes or if the changeset type includes the Schema change type.
     */
    const bool hasEcOrDdlChanges = containsDDLChanges || changeset.ContainsEcChanges();
    return MergeDataChanges(changeset, changeStream, hasEcOrDdlChanges, fastForward);
}

/*---------------------------------------------------------------------------------**/ /**
  * call the javascript `txn.reportError` method
  @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReportError(bool fatal, Utf8CP errorType, Utf8CP msg)  {
    auto jsTxns = m_dgndb.GetJsTxns();
    if (nullptr == jsTxns) {
        m_fatalValidationError |= fatal;
        return;
    }

    // Note: see IModelDb.ts [[ValidationError]]
    auto error = Napi::Object::New(jsTxns.Env());
    error["fatal"] = fatal;
    error["errorType"] = errorType;
    error["message"] = msg;
    DgnDb::CallJsFunction(jsTxns, "reportError", {error});
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::ModelChanges::ModelChanges(TxnManager& mgr) : m_mgr(mgr)
    {
    // We require BisCore 1.0.11 or later for the LastMod and GeometryGuid properties.
    // But we must defer the check, to give the app a chance to upgrade the schema.
    // If the file's opened in read-only mode though, there can be no changes to track.
    if (mgr.GetDgnDb().IsReadonly())
        {
        m_determinedMode = true;
        m_mode = Mode::Readonly;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::ModelChanges::Mode TxnManager::ModelChanges::DetermineMode()
    {
    if (!m_determinedMode)
        {
        m_determinedMode = true;
        if (m_mgr.GetDgnDb().GetGeometricModelUpdateStatement().IsValid())
            {
            m_mode = Mode::Full;
            }
        else
            {
            m_mode = Mode::Legacy;
            ClearAll();
            }
        }

    return m_mode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::ModelChanges::Mode TxnManager::ModelChanges::SetTrackingGeometry(bool track)
    {
    auto mode = DetermineMode();
    if (Mode::Full != mode || track == m_trackGeometry)
        return m_mode;

    m_trackGeometry = track;

    // Establish or reset baselines for loaded models
    m_mgr.GetDgnDb().Models().WithLoadedModels([track](DgnModels::T_DgnModelMap const& models)
        {
        for (auto& kvp : models)
            {
            auto geom = kvp.second->ToGeometricModelP();
            if (nullptr != geom)
                T_HOST.Visualization().SetTrackingGeometry(*geom, track);
            }
        });

    return m_mode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ModelChanges::InsertGeometryChange(DgnModelId modelId, DgnElementId elementId, TxnTable::ChangeType type) {
    BeAssert(IsTrackingGeometry());
    auto iter = m_geometryChanges.find(modelId);
    if (m_geometryChanges.end() == iter)
        iter = m_geometryChanges.Insert(modelId, GeometricElementChanges()).first;

    iter->second.AddElement(elementId, type);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ModelChanges::AddGeometricElementChange(DgnModelId modelId, DgnElementId elementId, TxnTable::ChangeType type, bool fromCommit) {
    m_geometricModels.Insert(modelId, fromCommit);

    if (IsTrackingGeometry()) {
        InsertGeometryChange(modelId, elementId, type);
        return;
    }

    auto model = m_mgr.GetDgnDb().Models().Get<GeometricModel>(modelId);
    if (model.IsNull())
        return;

    if (TxnTable::ChangeType::Delete == type)
        model->RemoveFromRangeIndex(elementId);
    else
        model->UpdateElementRange(elementId, model->GetElementRange(elementId));
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ProcessModelChanges() {
    if (m_initTableHandlers)
        m_modelChanges.Process();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::NotifyModelChanges() {
    if (!m_initTableHandlers)
        return;
    m_modelChanges.Notify();
    Models().NotifyGeometryChanges();
    ClearModelChanges();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ClearModelChanges() {
    if (m_initTableHandlers) {
        m_modelChanges.ClearAll();
        Models().ClearGeometryChanges();
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ModelChanges::Process()
    {
    auto mode = DetermineMode();

    // When we get a Change that deletes a geometric element, we don't have access to its model Id at that time - look it up now.
    for (auto const& deleted : m_deletedGeometricElements)
        {
        auto iter = m_modelsForDeletedElements.find(deleted.first);
        if (m_modelsForDeletedElements.end() != iter)
            AddGeometricElementChange(iter->second, deleted.first, TxnTable::ChangeType::Delete, deleted.second);
        }

    m_deletedGeometricElements.clear();
    m_modelsForDeletedElements.clear();

    if (m_models.empty() && m_geometricModels.empty())
        return;

    if (Mode::Full != mode)
        {
        Clear(true);
        return;
        }

    SetandRestoreIndirectChanges _v(m_mgr);

    // if there were any geometric changes, update the "GeometryGuid" and "LastMod" properties in the Model table.
    if (!m_geometricModels.empty())
        {
        auto stmt = m_mgr.GetDgnDb().GetGeometricModelUpdateStatement();
        BeAssert(stmt.IsValid()); // because DetermineStatus()

        BeGuid guid(true); // create a new GUID to represent this state of the changed geometric models
        for (auto model : m_geometricModels)
            {
            if (!model.second)
                continue; // change wasn't from commit - don't update GeometryGuid or LastMod

            m_models.erase(model.first); // we don't need to update LastMod below - this statement updates it.
            stmt->BindGuid(1, guid);
            stmt->BindId(2, model.first);
            DbResult rc = stmt->Step();
            UNUSED_VARIABLE(rc);
            BeAssert(BE_SQLITE_DONE == rc);
            stmt->Reset();
            }
        }

    if (!m_models.empty())
        {
        auto stmt = m_mgr.GetDgnDb().GetModelLastModUpdateStatement();
        BeAssert(stmt.IsValid()); // because DetermineStatus()

        for (auto model : m_models)
            {
            if (!model.second)
                continue; // change wasn't from commit - don't update LastMod.

            stmt->BindId(1, model.first);
            DbResult rc = stmt->Step();
            UNUSED_VARIABLE(rc);
            BeAssert(BE_SQLITE_DONE == rc);
            stmt->Reset();
            }
        }

    Clear(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ModelChanges::Notify()
    {
    if (!m_geometryChanges.empty())
        m_mgr.OnGeometricModelChanges();
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DisableTracking {
    TxnManager& m_txns;
    bool m_wasTracking;
    DisableTracking(TxnManager& txns) : m_txns(txns) { m_wasTracking = txns.EnableTracking(false); }
    ~DisableTracking() { m_txns.EnableTracking(m_wasTracking); }
};

class ProfilerScope {
    const Profiler::Scope* m_scope = nullptr;
    bool m_isTrackingEnabled;
public:
    ProfilerScope(TxnManagerR txn) {
        m_scope = Profiler::GetScope(txn.GetDgnDb());
        m_isTrackingEnabled = m_scope != nullptr && txn.TrackChangesetHealthStats();
    }

    void Start() const { if (m_isTrackingEnabled) m_scope->Start(); }
    void Resume() const { if (m_isTrackingEnabled && m_scope->IsPaused()) m_scope->Resume(); }
    void Pause() const { if (m_isTrackingEnabled) m_scope->Pause(); }
    void Stop() const { if (m_isTrackingEnabled) m_scope->Stop(); }
};

/*---------------------------------------------------------------------------------**//**
* Apply a changeset to the database. Notify all TxnTables about what was in the Changeset afterwards.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ApplyChanges(ChangeStreamCR changeset, TxnAction action, bool containsSchemaChanges, bool invert, bool fastForward) {
    if (invert && fastForward) {
         LOG.error("ApplyChanges() cannot be called with invert & fastForward flag both been set at same time");
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }

    BeAssert(action != TxnAction::None);
    AutoRestore<TxnAction> saveAction(&m_action, action);
    auto pmConf = PullMergeConf::Load(m_dgndb);
    m_dgndb.Elements().ClearCache(); // we can't rely on the elements in the cache after apply, just clear them all

    // if we're not using table handlers, we won't keep the models up to date, just clear them
    if (!m_initTableHandlers)
        m_dgndb.Models().ClearCache();

    if (!IsInAbandon())
        OnBeginApplyChanges();


    if (containsSchemaChanges)  {
        // notify ECPresentation and ConcurrentQuery to stop/cancel all running task before applying schema changeset.
        m_dgndb.Schemas().OnBeforeSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
    }

    // we want cascade delete to work during rebase.
    bool fkNoAction = !pmConf.IsRebasingLocalChanges();
    const bool ignoreNoop = pmConf.IsRebasingLocalChanges();
    bool fastForwardEncounteredMergeConflict = false;
    auto fastForwardConflictHandler = [&fastForwardEncounteredMergeConflict](ChangeStream::ConflictCause _, Changes::Change change) {
        if(change.IsIndirect())
            return ChangeStream::ConflictResolution::Replace;

        fastForwardEncounteredMergeConflict = true;
        return ChangeStream::ConflictResolution::Abort;
    };

    const auto scope = ProfilerScope(*this);
    scope.Start();
    // apply schema part of changeset before data changes if schema changes are present
    if (containsSchemaChanges)
        {
        // If the SQLITE_CHANGESETAPPLY_FKNOACTION flag is set, we need to check if the cache tables have been tracked in the changeset.
        // If the cache tables are not tracked, we should set the flag to false to allow cascades and avoid any possible FK constraint violations.
        if (fkNoAction)
            fkNoAction = IsCacheTableNameInChangeset(Changes(changeset, false));

        m_dgndb.Schemas().OnBeforeSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
        auto schemaApplyArgs = ApplyChangesArgs::Default()
            .SetInvert(invert)
            .SetIgnoreNoop(true)
            .SetFkNoAction(fkNoAction)
            .ApplyOnlySchemaChanges();

        if (fastForward) {
            schemaApplyArgs.SetConflictHandler(fastForwardConflictHandler);
        }

        if(!m_dgndb.IsReadonly()) {
            const auto result = [&]() {
                auto _v = pmConf.IsRebasingLocalChanges() ? nullptr : std::make_unique<DisableTracking>(*this);
                UNUSED_VARIABLE(_v);
                return changeset.ApplyChanges(m_dgndb, schemaApplyArgs);
            }();
            if (result != BE_SQLITE_OK) {
                if (fastForwardEncounteredMergeConflict)
                    LOG.error("failed to apply changeset in fastforward mode. Atleast one conflict was detected");
                else
                    LOG.errorv("failed to apply changeset: %s", BeSQLiteLib::GetErrorName(result));

                m_dgndb.AbandonChanges();
                m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
                scope.Stop();
                return result;
            }
        }

        scope.Pause();
        m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
            if (action == TxnAction::Merge) {
                const auto result = m_dgndb.AfterSchemaChangeSetApplied();
                if (result != BE_SQLITE_OK) {
                    LOG.errorv("ApplyChanges failed schema changes: %s", BeSQLiteLib::GetErrorName(result));
                    BeAssert(false);
                    m_dgndb.AbandonChanges();
                    scope.Stop();
                    return result;
                }
            }
    }

    auto dataApplyArgs = ApplyChangesArgs::Default()
        .SetInvert(invert)
        .SetIgnoreNoop(ignoreNoop)
        .SetFkNoAction(fkNoAction);

    if (fastForward) {
        dataApplyArgs.SetConflictHandler(fastForwardConflictHandler);
    }

    // If schema changes are present, we need to apply only data changes after schema changes are applied.
    if (containsSchemaChanges){
        dataApplyArgs.ApplyOnlyDataChanges();
    }

    scope.Resume();
    if (!m_dgndb.IsReadonly()) {
        const auto result = [&]() {
            auto _v = pmConf.IsRebasingLocalChanges() ? nullptr : std::make_unique<DisableTracking>(*this);
            UNUSED_VARIABLE(_v);
            return changeset.ApplyChanges(m_dgndb, dataApplyArgs);
        }();

        if (result != BE_SQLITE_OK) {
            if (fastForwardEncounteredMergeConflict)
                LOG.error("failed to apply changeset in fastforward mode. Atleast one conflict was detected");
            else
                LOG.errorv("failed to apply changeset: %s", BeSQLiteLib::GetErrorName(result));

            m_dgndb.AbandonChanges();
            if (containsSchemaChanges) {
                m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
            }
            scope.Stop();
            return result;
        }
    }
    scope.Stop();

    if (action == TxnAction::Merge) {
        auto result = m_dgndb.AfterDataChangeSetApplied(containsSchemaChanges);
        if (result != BE_SQLITE_OK) {
            LOG.errorv("ApplyChanges failed data changes: %s", BeSQLiteLib::GetErrorName(result));
            BeAssert(false);
            m_dgndb.AbandonChanges();
            if (containsSchemaChanges) {
                m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
            }
            return result;
        }
    }

    ModelChangesScope _v_v_(*this);
    OnChangeSetApplied(changeset, invert);

    if (!IsInAbandon() && m_initTableHandlers) {
        CallJsTxnManager("_onChangesApplied");
        CallMonitors([&](TxnMonitor& monitor) { monitor._OnAppliedChanges(*this); });
    }

    if (!IsInAbandon())
        OnEndApplyChanges();

    if (containsSchemaChanges)
        m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);

    return BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**//**
* This removes unchanged indexes from DDL. There is already a fix in to remove it from where
* these unchanged indexes get added on ECDb side. But following fixes issue with existing
* changesets that is already on imodelhub or old bridges that is not consuming the source fix.
* In case of any error this function return original unchanged DDL.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TxnManager::PatchSlowDdlChanges(Utf8StringR patchedDDL, Utf8StringCR compoundSQL)
    {
    /** Performance issue due to recreating indexes that have not changed
     *  DDL in schema changeset are ordered so CREATE TABLE/ ALTER TABLE is followed by DROP INDEX followed by CREATE INDEX
     *  How to skip indexes that have not changed.
     *  1. Find all DROP indexes DDL
     *  2. Find all CREATE indexes DDL
     *  3. See if create index matches sql in sqlite_master.
     *      a. If matches then skip the index and also erase DDL for DROP if any.
     *      b. If does not exist record it and it will be executed after non-index DDLs
     **/
    const auto kStmtDelimiter = ";";
    const auto kIdentDelimiter = " ";
    const auto kQuotedLiteralStart = '[';

    bmap<Utf8String, Utf8String> dropIndexes;
    bmap<Utf8String, Utf8String> createIndexes;
    bmap<Utf8String, Utf8String> currentIndexes;
    // Split DDL on ; this would give use individual SQL. This is safe as we do not string literal ';' for any other use.
    bvector<Utf8String> individualSQL;
    BeStringUtilities::Split(compoundSQL.c_str(), kStmtDelimiter, individualSQL);

    // Read all the index from sqlite_master
    Statement indexStmt;
    if (indexStmt.Prepare(m_dgndb, "SELECT name, sql FROM sqlite_master WHERE type='index' AND sql IS NOT NULL") != BE_SQLITE_OK)
        {
        BeAssert(false && "Should be able to read current indexes");
        return ERROR;
        }

    while (indexStmt.Step() == BE_SQLITE_ROW)
        {
        currentIndexes[indexStmt.GetValueText(0)] = indexStmt.GetValueText(1);
        }
    indexStmt.Finalize(); // This is required as DDL will fail with DB_LOCK

    for (auto && sql : individualSQL)
        {
        if (sql.StartsWith("DROP INDEX IF EXISTS") || sql.StartsWith("CREATE INDEX") || sql.StartsWith("CREATE UNIQUE INDEX"))
            {
            bvector<Utf8String> sqlTokens;
            BeStringUtilities::Split(sql.c_str(), kIdentDelimiter, sqlTokens);
            int index = -1;
            bool drop = false;
            if (sqlTokens[0] == "DROP")
                {
                index = 4;
                drop = true;
                }
            else if (sqlTokens[0] == "CREATE")
                {
                if (sqlTokens[1] == "INDEX")
                    index = 2;
                else
                    index = 3;
                }
            if (index < 0)
                {
                BeAssert(false && "Should find index name");
                return ERROR;
                }
            Utf8String& nameToken = sqlTokens[index];
            Utf8String indexName = nameToken[0] == kQuotedLiteralStart ? nameToken.substr(1, nameToken.size() - 2) : nameToken;
            if (drop)
                {
                // record drop index
                dropIndexes[indexName] = sql;
                }
            else
                {
                // check if we have exact match against sqlite_master index definition for given index name.
                const auto iter = currentIndexes.find(indexName);
                const bool hasExistingIndex = iter != currentIndexes.end();
                const bool indexIsUnchanged = hasExistingIndex &&  (*iter).second == sql;
                if (indexIsUnchanged)
                    {
                    // make sure not to drop the index nor create it.
                    // this safe ton of time for large files.
                    dropIndexes.erase(indexName);
                    }
                else
                    {
                    // record sql for create index which may or may not have a DROP index associated with it.
                    createIndexes[indexName] = sql;
                    if (hasExistingIndex)
                        {
                        // ensure we have drop statement for this index
                        if (dropIndexes.find(indexName) == dropIndexes.end())
                            {
                            dropIndexes[indexName].Sprintf("DROP INDEX IF EXISTS [%s]", indexName.c_str());
                            }
                        }
                    }
                }
            }
        else
            {
            // Append all non-index DDL as is.
            patchedDDL.append(sql).append(kStmtDelimiter);
            }
        }
    // Append all drop index to ddl changes before create index
    for (auto&& kv : dropIndexes)
        patchedDDL.append(kv.second).append(kStmtDelimiter);

    // Append create index to dll after drop index
    for (auto&& kv : createIndexes)
        patchedDDL.append(kv.second).append(kStmtDelimiter);

    return patchedDDL == compoundSQL ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ApplyDdlChanges(DdlChangesCR ddlChanges) {
    BeAssert(!ddlChanges._IsEmpty() && "DbSchemaChangeSet is empty");
    const auto info = GetDgnDb().Schemas().GetSchemaSync().GetInfo();
    const bool wasTracking = EnableTracking(false);
    Utf8String originalDDL = ddlChanges.ToString();

    DbResult result = BE_SQLITE_OK;
    if (!info.IsEmpty()) {
        // In SchemaSync, we still need to apply schema changes to ec_*, dgn_*, and be_* tables.
        // We cannot determine which DDL is for profile tables, so we try applying all of them.
        // If it fails, it's fine because SchemaSync::pull() will patch/update all tables as necessary after applying the changeset.
        // Not applying the DDL can cause the current changeset to fail if a column in the profile table is missing.
        // This issue is detected when applying a changeset that upgrades the ECDb profile from version 4.0.0.1 to a newer version.
        // Version 4.0.0.2 adds new tables and columns to ec_* tables. Since SchemaSync::pull() is called, after pull/merge is complete.
        bvector<Utf8String> individualSQL;
        BeStringUtilities::Split(originalDDL.c_str(), ";", individualSQL);
        for (auto& sql : individualSQL) {
            result = m_dgndb.TryExecuteSql(sql.c_str());
            if (result != BE_SQLITE_OK) {
                LOG.warningv("ApplyDdlChanges() with SchemaSync: Failed to apply DDL changes. Error: %s (%s)", BeSQLiteLib::GetErrorName(result), sql.c_str());
            }
        }
        EnableTracking(wasTracking);
        return BE_SQLITE_OK;
    }

    Utf8String patchedDDL;
    BentleyStatus status = PatchSlowDdlChanges(patchedDDL, originalDDL);
    if (status == SUCCESS) {
        // Info message so we can look out if this issue has gone due to fix in the place which produce these changeset.
        LOG.info("[PATCH] Appling DDL patch for #292801 #281557");
        result = m_dgndb.ExecuteSql(patchedDDL.c_str());
        if (result != BE_SQLITE_OK) {
            LOG.info("[PATCH] Failed to apply patch for #292801 #281557. Fallback to original DDL");
            result = m_dgndb.ExecuteSql(originalDDL.c_str());
        }
    } else {
        result = m_dgndb.ExecuteSql(originalDDL.c_str());
    }

    EnableTracking(wasTracking);
    return result;
}

/*---------------------------------------------------------------------------------**/ /**
* Changesets are stored as compressed blobs in the DGN_TABLE_Txns table. Read one by rowid.
* If the TxnDirection is backwards, invert the changeset.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ReadDataChanges(ChangeSet& dataChangeSet, TxnId rowId, TxnAction action) {
    if (ReadChanges(dataChangeSet, rowId) != ZIP_SUCCESS)
        return BE_SQLITE_ERROR;

    return (action == TxnAction::Reverse) ? dataChangeSet.Invert() : BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**/ /**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors TxnManager::ReadChanges(ChangeSet& changeset, TxnId rowId) {
    ZipErrors status = m_snappyFrom.Init(m_dgndb, DGN_TABLE_Txns, "Change", rowId.GetValue());
    if (ZIP_SUCCESS != status)
        return status;

    ChangesBlobHeader header(m_snappyFrom);
    if ((ChangesBlobHeader::DB_Signature06 != header.m_signature) || 0 == header.m_size)
        return ZIP_ERROR_BAD_DATA;

    return m_snappyFrom.ReadToChunkedArray(changeset.m_data, header.m_size);
}

/*---------------------------------------------------------------------------------**/ /**
* Read a changeset from the dgn_Txn table, potentially inverting it (depending on whether we're performing undo or redo),
* and then apply the changeset to the DgnDb.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ApplyTxnChanges(TxnId rowId, TxnAction action, bool skipSchemaChanges) {
    auto updateTxnDeletedFlag = [&]() -> DbResult {
        CachedStatementPtr stmt = GetTxnStatement("UPDATE " DGN_TABLE_Txns " SET Deleted=? WHERE Id=?");
        stmt->BindInt(1, action == TxnAction::Reverse);
        stmt->BindInt64(2, rowId.GetValue());
        auto rc = stmt->Step();
        return rc != BE_SQLITE_DONE ? rc : BE_SQLITE_OK;
    };

    if (HasDataChanges()) {
        LOG.errorv("ApplyTxnChanges called while there are still unsaved changes present. txnId 0x" PRIx64, rowId.m_id.m_64);
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }
    
    // if (m_dgndb.IsReadonly()) {
    //     LOG.errorv("ApplyTxnChanges called on a read-only database. txnId 0x" PRIx64, rowId.m_id.m_64);
    //     return BE_SQLITE_READONLY;
    // }

    if(TxnAction::Reverse != action && TxnAction::Reinstate != action){
        LOG.errorv("ApplyTxnChanges called with an invalid action. txnId 0x" PRIx64, rowId.m_id.m_64);
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }

    if(skipSchemaChanges) {
        LOG.infov("ApplyTxnChanges called with skipSchemaChanges flag set to true. txnId 0x" PRIx64, rowId.m_id.m_64);
        return updateTxnDeletedFlag();
    }

    const auto type = GetTxnType(rowId);
    if (type == TxnType::Ddl)
        return updateTxnDeletedFlag();

    UndoChangeSet changeset;
    auto rc = ReadDataChanges(changeset, rowId, action);
    if (BE_SQLITE_OK != rc) {
        LOG.errorv("ApplyTxnChanges failed to read changeset for txnId 0x" PRIx64 ": %s", rowId.m_id.m_64, BeSQLiteLib::GetErrorName(rc));
        return rc;
    }

    rc = ApplyChanges(changeset, action, type == TxnType::EcSchema);
    BeAssert(!HasDataChanges());

    if (BE_SQLITE_OK != rc)
        return rc;

    return updateTxnDeletedFlag();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnBeginApplyChanges() {
    if (m_initTableHandlers) {
        for (auto table : m_tables)
            table->_OnApply();
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnEndApplyChanges() {
    if (!m_initTableHandlers)
        return;

    NotifyModelChanges();
    for (auto table : m_tables)
        table->_OnApplied();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReverseTxnRange(TxnRange const& txnRange) {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    if (HasChanges())
        m_dgndb.AbandonChanges();

    for (TxnId curr = QueryPreviousTxnId(txnRange.GetLast()); curr.IsValid() && curr >= txnRange.GetFirst(); curr = QueryPreviousTxnId(curr))
        ApplyTxnChanges(curr, TxnAction::Reverse);

    BeAssert(!HasChanges());
    m_dgndb.SaveChanges(); // make sure we save the updated Txn data to disk.

    m_curr = txnRange.GetFirst(); // we reuse TxnIds

    // save in reversed Txns list
    m_reversedTxn.push_back(txnRange);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseTo(TxnId pos) {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    TxnId lastId = GetCurrentTxnId();
    if (!pos.IsValid() || pos >= lastId)
        return DgnDbStatus::NothingToUndo;

    TxnId firstUndoable = GetSessionStartId();
    if (firstUndoable >= lastId || pos < firstUndoable)
        return DgnDbStatus::CannotUndo;

    OnBeforeUndoRedo(true);
    return ReverseActions(TxnRange(pos, lastId));
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::CancelTo(TxnId pos) {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    DgnDbStatus status = ReverseTo(pos);
    DeleteReversedTxns(); // call this even if we didn't reverse anything - there may have already been reversed changes.
    return status;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseActions(TxnRange const& txnRange) {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    ReverseTxnRange(txnRange); // do the actual undo now.

    while (GetCurrentTxnId() < GetMultiTxnOperationStart())
        EndMultiTxnOperation();

    OnUndoRedo(TxnAction::Reverse);
    return DgnDbStatus::Success;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseTxns(int numActions) {
    TxnId lastId = GetCurrentTxnId();
    TxnId firstUndoableId = GetSessionStartId();

    TxnId firstId = lastId;
    while (numActions > 0 && firstId > firstUndoableId) {
        TxnId prevId = QueryPreviousTxnId(firstId);
        if (!prevId.IsValid())
            break;

        if (!IsMultiTxnMember(prevId))
            --numActions;

        firstId = prevId;
    }

    if (firstId == lastId)
        return DgnDbStatus::NothingToUndo;

    OnBeforeUndoRedo(true);
    return ReverseActions(TxnRange(firstId, lastId));
}

/*---------------------------------------------------------------------------------**//**
* reverse (undo) all previous transactions
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseAll() {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    TxnId lastId = GetCurrentTxnId();
    TxnId startId = GetSessionStartId();

    if (startId >= lastId)
        return DgnDbStatus::NothingToUndo;

    OnBeforeUndoRedo(true);
    return ReverseActions(TxnRange(startId, GetCurrentTxnId()));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReplayExternalTxns(TxnId from) {
    if (!m_initTableHandlers || !m_dgndb.IsReadonly())
        return; // this method can only be called on a readonly connection with the TxnManager active
    
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    TxnId curr = QueryNextTxnId(from);
    bool haveTxns = curr.IsValid();
    if (haveTxns) {
        CallJsTxnManager("_onReplayExternalTxns");
        while (curr.IsValid()) {
            ApplyTxnChanges(curr, TxnAction::Reinstate);
            curr = QueryNextTxnId(curr);
        }
    }

    m_curr = GetLastTxnId(); // this is where the other session ends
    if (m_curr.GetValue() == 0)
        m_curr = TxnId(SessionId(1),0);
    else
        m_curr.Increment();

    if (haveTxns)
        CallJsTxnManager("_onReplayedExternalTxns");
}

/*---------------------------------------------------------------------------------**/ /**
* Reinstate ("redo") a range of transactions.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReinstateTxn(TxnRange const& revTxn) {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    BeAssert(m_curr == revTxn.GetFirst());
    BeAssert(!m_reversedTxn.empty());

    if (HasChanges())
        m_dgndb.AbandonChanges();

    TxnId last = QueryPreviousTxnId(revTxn.GetLast());
    for (TxnId curr = revTxn.GetFirst(); curr.IsValid() && curr <= last; curr = QueryNextTxnId(curr))
        ApplyTxnChanges(curr, TxnAction::Reinstate);

    m_dgndb.SaveChanges(); // make sure we save the updated Txn data to disk.

    m_curr = revTxn.GetLast();
    m_reversedTxn.pop_back();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReinstateActions(TxnRange const& revTxn) {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    ReinstateTxn(revTxn); // do the actual redo now.

    OnUndoRedo(TxnAction::Reinstate);
    return DgnDbStatus::Success;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReinstateTxn() {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    if (!IsRedoPossible())
        return DgnDbStatus::NothingToRedo;

    OnBeforeUndoRedo(false);
    return ReinstateActions(m_reversedTxn.back());
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetUndoString() {
    return IsUndoPossible() ? GetTxnDescription(QueryPreviousTxnId(GetCurrentTxnId())) : "";
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetRedoString() {
    return IsRedoPossible() ? GetTxnDescription(m_reversedTxn.back().GetFirst()) : "";
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DeleteAllTxns() {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    m_dgndb.SaveChanges(); // in case there are outstanding changes that will create a new Txn
    m_dgndb.ExecuteSql("DELETE FROM " DGN_TABLE_Txns);
    m_dgndb.SaveChanges();
    Initialize();
}

/*---------------------------------------------------------------------------------**//**
* Cancel any undone (reversed) transactions.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DeleteReversedTxns() {
    m_reversedTxn.clear(); // do this even if this is already empty - there may be reversed txns from a previous session
    m_dgndb.ExecuteSql("DELETE FROM " DGN_TABLE_Txns " WHERE Deleted=1"); // these transactions are no longer reinstateable. Throw them away.
}

/*---------------------------------------------------------------------------------**//**
* Delete transactions from the start of the table to (but not including) the specified transaction.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DeleteFromStartTo(TxnId lastId) {
    if (PullMergeConf::Load(m_dgndb).InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    Statement stmt(m_dgndb, "DELETE FROM " DGN_TABLE_Txns " WHERE Id < ?");
    stmt.BindInt64(1, lastId.GetValue());

    DbResult result = stmt.Step();
    if (result != BE_SQLITE_DONE)
        m_dgndb.ThrowException("error deleting from Txn table", result);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_Initialize() {
    m_txnMgr.GetDgnDb().CreateTable(TEMP_TABLE(TXN_TABLE_Elements), "ElementId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT,ECClassId INTEGER NOT NULL");
    m_txnMgr.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Elements) "_Midx ON " TXN_TABLE_Elements "(ModelId)");
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::CreateIndexOnECClassId() {
    if (m_haveIndexOnECClassId)
        return;

    m_haveIndexOnECClassId = true;
    m_txnMgr.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Elements) "_Cidx ON " TXN_TABLE_Elements "(ECClassId)");
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnValidate() {
    m_changes = false;
    if (m_stmt.IsPrepared())
        return;

    m_stmt.Prepare(m_txnMgr.GetDgnDb(), "INSERT INTO " TEMP_TABLE(TXN_TABLE_Elements) "(ElementId,ModelId,ChangeType,ECClassId) VALUES(?,?,?,?)");
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnValidated() {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes)
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Elements));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_Initialize()
    {
    auto& db = m_txnMgr.GetDgnDb();
    db.CreateTable(TEMP_TABLE(TXN_TABLE_Models), "ModelId INTEGER NOT NULL PRIMARY KEY,ChangeType INT,ECClassId INTEGER NOT NULL");

    // We will want to identify changes to the GeometryGuid column, if present. (Introduced in BisCore 1.0.11).
    auto ecsql = "SELECT ti.cid SqliteColumnIndex"
        " FROM   ec_PropertyMap pp"
               " JOIN ec_Column c ON c.Id = pp.ColumnId"
               " JOIN ec_Table t ON t.Id = c.TableId"
               " JOIN ec_Class cl ON cl.Id = pp.ClassId"
               " JOIN ec_PropertyPath p ON p.Id = pp.PropertyPathId"
               " JOIN ec_Schema s ON cl.SchemaId = s.Id"
               " JOIN pragma_table_info(t.Name) ti ON ti.name = c.Name"
        " WHERE  s.Name = 'BisCore'"
                 " AND cl.Name = 'GeometricModel'"
                 " AND p.AccessString = 'GeometryGuid'";

    auto stmt = db.GetCachedStatement(ecsql);
    BeAssert(stmt.IsValid());
    if (!stmt.IsValid() || BE_SQLITE_ROW != stmt->Step())
        {
        // No GeometryGuid column.
        return;
        }

    m_geometryGuidColumnIndex = stmt->GetValueInt(0);
    BeAssert(m_geometryGuidColumnIndex > 0);

    // We will want to identify changes to subclasses of GeometricModel.
    auto classId = db.Schemas().GetClassId("BisCore", "GeometricModel");
    BeAssert(classId.IsValid());

    Utf8String sql("SELECT NULL FROM ec_cache_ClassHierarchy WHERE ClassId=? AND BaseClassId=");
    sql.append(classId.ToHexStr());
    m_isGeometricModelStmt.Prepare(db, sql.c_str());
    BeAssert(m_isGeometricModelStmt.IsPrepared());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool dgn_TxnTable::Model::IsGeometryGuidChanged(DgnModelId modelId, BeSQLite::Changes::Change const& change)
    {
    if (!HasGeometryGuid())
        return false;

    if (!change.GetNewValue(m_geometryGuidColumnIndex).IsValid())
        return false;

    // GeometryGuid is in a shared column - confirm this is a GeometricModel.
    auto select = m_txnMgr.GetDgnDb().GetCachedStatement("SELECT ECClassId FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id=?");
    select->BindId(1, modelId);
    if (BE_SQLITE_ROW != select->Step())
        return false;

    auto classId = select->GetValueId<ECClassId>(0);
    auto& stmt = m_isGeometricModelStmt;
    BeAssert(stmt.IsPrepared() && HasGeometryGuid());

    stmt.BindId(1, classId);
    auto isGeometricModel = false;
    if (BE_SQLITE_ROW == stmt.Step())
        {
        isGeometricModel = true;
        // ###TODO fix in progress BeAssert(change.IsIndirect());
        }

    stmt.Reset();
    stmt.ClearBindings();

    return isGeometricModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::NotifyGeometryChanges()
    {
    if (!m_geometryGuidChanges.empty())
        m_txnMgr.OnGeometryGuidChanges(m_geometryGuidChanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnValidate()
    {
    m_changes = false;
    if (m_stmt.IsPrepared())
        return;

    m_stmt.Prepare(m_txnMgr.GetDgnDb(), "INSERT INTO " TEMP_TABLE(TXN_TABLE_Models) "(ModelId,ChangeType,ECClassId) VALUES(?,?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnValidated()
    {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes)
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Models));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void dgn_TxnTable::Model::AddChange(Changes::Change const& change, ChangeType changeType) {
    Changes::Change::Stage stage;
    switch (changeType) {
    case ChangeType::Insert:
        stage = Changes::Change::Stage::New;
        break;

    case ChangeType::Update:
    case ChangeType::Delete:
        stage = Changes::Change::Stage::Old;
        break;

    default:
        BeAssert(false);
        return;
    }

    DgnModelId modelId = DgnModelId(change.GetValue(0, stage).GetValueUInt64());
    BeAssert(modelId.IsValid());

    if (ChangeType::Update == changeType) {
        // For updates, we're only interested in detecting changes to the GeometricModel.GeometryGuid column.
        if (IsGeometryGuidChanged(modelId, change))
            m_geometryGuidChanges.insert(modelId);

        return;
    }

    auto classId = DgnClassId(change.GetValue((int)DgnModel::ColumnNumbers::ECClassId, stage).GetValueUInt64());
    enum Column : int { ModelId = 1,
                        ChangeType = 2,
                        ECClassId = 3 };

    m_changes = true;
    m_stmt.BindId(Column::ModelId, modelId);
    m_stmt.BindInt(Column::ChangeType, (int)changeType);
    m_stmt.BindId(Column::ECClassId, classId);

    auto rc = m_stmt.Step();
    BeAssert(rc == BE_SQLITE_DONE);
    UNUSED_VARIABLE(rc);

    m_stmt.Reset();
    m_stmt.ClearBindings();
}

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnApply() {
    if (!m_txnMgr.IsInAbandon())
        _OnValidate();
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnApplied() {
    if (!m_txnMgr.IsInAbandon())
        _OnValidated();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnAppliedDelete(BeSQLite::Changes::Change const& change) {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Delete);

    DgnModelId modelId = change.GetOldValue(0).GetValueId<DgnModelId>();
    DgnModelPtr model = m_txnMgr.GetDgnDb().Models().FindModel(modelId);
    if (!model.IsValid())
        return;

    m_txnMgr.GetDgnDb().Models().DropLoadedModel(*model);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnAppliedUpdate(BeSQLite::Changes::Change const& change) {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Update);

    DgnModelId modelId = change.GetOldValue(0).GetValueId<DgnModelId>();
    DgnModelPtr model = m_txnMgr.GetDgnDb().Models().FindModel(modelId);
    if (!model.IsValid())
        return;

    model->Read(modelId);
    model->_OnUpdated();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnAppliedAdd(BeSQLite::Changes::Change const& change) {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Insert);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_Initialize()
    {
    m_txnMgr.GetDgnDb().CreateTable(TEMP_TABLE(TXN_TABLE_Depend), "ECInstanceId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT");
    m_txnMgr.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Depend) "_Midx ON " TXN_TABLE_Depend "(ModelId)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_OnValidate()
    {
    m_changes = false;
    if (m_stmt.IsPrepared())
        return;

    m_stmt.Prepare(m_txnMgr.GetDgnDb(), "INSERT INTO " TEMP_TABLE(TXN_TABLE_Depend) " (ECInstanceId,ModelId,ChangeType) VALUES(?,?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_OnValidated()
    {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes && TxnAction::Abandon != m_txnMgr.GetCurrentAction())
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Depend));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::UpdateSummary(Changes::Change change, ChangeType changeType)
    {
    m_changes = true;

    if (ChangeType::Delete == changeType)
        {
        ECInstanceId relid = change.GetOldValue(0).GetValueId<ECInstanceId>();
        ECClassId relclsid = change.GetOldValue(1).GetValueId<ECClassId>();
        int64_t srcelemid = change.GetOldValue(2).GetValueInt64();
        int64_t tgtelemid = change.GetOldValue(3).GetValueInt64();
        BeSQLite::EC::ECInstanceKey relkey(relclsid, relid);
        m_deletedRels.push_back(DepRelData(relkey, DgnElementId((uint64_t)srcelemid), DgnElementId((uint64_t)tgtelemid), true));
        }
    else
        {
        Changes::Change::Stage stage = (ChangeType::Insert == changeType) ? Changes::Change::Stage::New : Changes::Change::Stage::Old;
        ECInstanceId instanceId = change.GetValue(0, stage).GetValueId<ECInstanceId>(); // primary key is column 0
        AddDependency(instanceId, changeType);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::AddElement(DgnElementId elementId, DgnModelId modelId, ChangeType changeType, DgnClassId elementClassId, bool fromCommit)
    {
    enum Column : int {ElementId=1,ModelId=2,ChangeType=3,ECClass=4};

    BeAssert(modelId.IsValid());
    BeAssert(elementId.IsValid());

    m_changes = true;
    m_stmt.BindId(Column::ElementId, elementId);
    m_stmt.BindId(Column::ModelId, modelId);
    m_stmt.BindInt(Column::ChangeType, (int) changeType);
    m_stmt.BindId(Column::ECClass, elementClassId);

    m_stmt.Step(); // This may fail if element was inserted before a call to PropagateChanges and updated after that.

    m_stmt.Reset();
    m_stmt.ClearBindings();
    m_txnMgr.m_modelChanges.AddModel(modelId, fromCommit); // add to set of changed models.
    if (ChangeType::Delete == changeType)
        m_txnMgr.m_modelChanges.AddDeletedElement(elementId, modelId); // Record model Id in case it's a geometric element.
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId TxnTable::GetModelAndClass(ECClassId& classId, DgnElementId elementId) {
    CachedStatementPtr stmt = m_txnMgr.GetDgnDb().Elements().GetStatement("SELECT ModelId,ECClassId FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?");
    stmt->BindId(1, elementId);
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnModelId(); // This happens because we deleted the element locally and rejected an incoming update

    classId = stmt->GetValueId<DgnClassId>(1);
    return stmt->GetValueId<DgnModelId>(0);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void dgn_TxnTable::Element::AddChange(Changes::Change const& change, ChangeType changeType, bool fromCommit) {
    Changes::Change::Stage stage;
    switch (changeType) {
    case ChangeType::Insert:
        stage = Changes::Change::Stage::New;
        break;

    case ChangeType::Update:
    case ChangeType::Delete:
        stage = Changes::Change::Stage::Old;
        break;
    default:
        BeAssert(false);
        return;
    }

    DgnElementId elementId = change.GetValue((int)DgnElement::ColumnNumbers::ElementId, stage).GetValueId<DgnElementId>();
    DgnModelId modelId;
    DgnClassId classId;

    if (ChangeType::Update == changeType) {
        // for updates, the element table must be queried for ModelId since the change set will only contain changed columns
        modelId = GetModelAndClass(classId, elementId);
        if (!modelId.IsValid())
            return; // This happens because we deleted the element locally and rejected an incoming update
    } else {
        modelId = change.GetValue((int)DgnElement::ColumnNumbers::ModelId, stage).GetValueId<DgnModelId>();
        classId = change.GetValue((int)DgnElement::ColumnNumbers::ECClassId, stage).GetValueId<DgnClassId>();
    }

    AddElement(elementId, modelId, changeType, classId, fromCommit);
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool dgn_TxnTable::Geometric::HasChangeInColumns(BeSQLite::Changes::Change const& change) {
    // if any column between first and last has a new value, there were geometric changes.
    for (int i=GetFirstCol(); i<=_GetLastCol(); ++i) {
        if (change.GetNewValue(i).IsValid())
            return true;
    }
    // there were no geometric changes
    return false;
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Geometric::AddChange(BeSQLite::Changes::Change const& change, ChangeType changeType, bool fromCommit) {
    if (ChangeType::Update == changeType && !HasChangeInColumns(change))
        return; // no geometric changes

    auto stage = ChangeType::Insert == changeType ? Changes::Change::Stage::New : Changes::Change::Stage::Old;
    DgnElementId elementId = change.GetValue(0, stage).GetValueId<DgnElementId>();

    if (ChangeType::Delete == changeType) {
        // We don't have access to the model Id here. Rely on the Element txn table to record it for later use.
        m_txnMgr.m_modelChanges.AddDeletedGeometricElement(elementId, fromCommit);
        return;
    }

    DgnClassId classId;
    auto modelId = GetModelAndClass(classId, elementId);
    m_txnMgr.m_modelChanges.AddGeometricElementChange(modelId, elementId, changeType, fromCommit); // mark this model as having geometric changes.
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Element::Iterator::Entry dgn_TxnTable::Element::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        m_db->GetCachedStatement(m_stmt, "SELECT ElementId,ModelId,ChangeType,ECClassId FROM " TEMP_TABLE(TXN_TABLE_Elements));
    else
        m_stmt->Reset();

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnElementId dgn_TxnTable::Element::Iterator::Entry::GetElementId() const { return m_sql->GetValueId<DgnElementId>(0); }
DgnModelId dgn_TxnTable::Element::Iterator::Entry::GetModelId() const { return m_sql->GetValueId<DgnModelId>(1); }
TxnTable::ChangeType dgn_TxnTable::Element::Iterator::Entry::GetChangeType() const { return (TxnTable::ChangeType)m_sql->GetValueInt(2); }
DgnClassId dgn_TxnTable::Element::Iterator::Entry::GetECClassId() const { return m_sql->GetValueId<DgnClassId>(3); }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::AddDependency(EC::ECInstanceId const& relid, ChangeType changeType) {
    CachedECSqlStatementPtr stmt = m_txnMgr.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT element.Model.Id FROM " BIS_SCHEMA(BIS_CLASS_Element) " AS element, " BIS_SCHEMA(BIS_REL_ElementDrivesElement) " AS DEP"
        " WHERE (DEP.ECInstanceId=?) AND (element.ECInstanceId=DEP.SourceECInstanceId)");
    stmt->BindId(1, relid);
    auto stat = stmt->Step();
    UNUSED_VARIABLE(stat);
    BeAssert(stat == BE_SQLITE_ROW);
    DgnModelId mid = stmt->GetValueId<DgnModelId>(0);

    m_stmt.BindId(1, relid);
    m_stmt.BindId(2, mid);
    m_stmt.BindInt(3, (int)changeType);
    auto rc = m_stmt.Step();
    UNUSED_VARIABLE(rc);

    m_stmt.Reset();
    m_stmt.ClearBindings();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnRelationshipLinkTables::TxnRelationshipLinkTables(TxnManagerR t) : m_txnMgr(t)
    {
    if (m_txnMgr.GetDgnDb().TableExists(TEMP_TABLE(TXN_TABLE_RelationshipLinkTables)))
        return;

    Utf8String ddl;
    ddl.append(COLNAME_ECInstanceId).append(" INTEGER NOT NULL PRIMARY KEY,");
    ddl.append(COLNAME_ECClassId).append(" INTEGER NOT NULL,");
    ddl.append(COLNAME_SourceECInstanceId).append(" INTEGER NOT NULL,");
    ddl.append(COLNAME_TargetECInstanceId).append(" INTEGER NOT NULL,");
    ddl.append(COLNAME_ChangeType).append(" INT");
    m_txnMgr.GetDgnDb().CreateTable(TEMP_TABLE(TXN_TABLE_RelationshipLinkTables), ddl.c_str());
    m_txnMgr.GetDgnDb().ExecuteSql(Utf8PrintfString("CREATE INDEX " TEMP_TABLE(TXN_TABLE_RelationshipLinkTables) "_ClassIdx ON " TXN_TABLE_RelationshipLinkTables "(%s)", COLNAME_ECClassId).c_str());
    m_txnMgr.GetDgnDb().ExecuteSql(Utf8PrintfString("CREATE INDEX " TEMP_TABLE(TXN_TABLE_RelationshipLinkTables) "_SourceIdx ON " TXN_TABLE_RelationshipLinkTables "(%s)", COLNAME_SourceECInstanceId).c_str());
    m_txnMgr.GetDgnDb().ExecuteSql(Utf8PrintfString("CREATE INDEX " TEMP_TABLE(TXN_TABLE_RelationshipLinkTables) "_TargetIdx ON " TXN_TABLE_RelationshipLinkTables "(%s)", COLNAME_TargetECInstanceId).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult TxnRelationshipLinkTables::Insert(BeSQLite::EC::ECInstanceId relid, ECN::ECClassId relclsid, DgnElementId srcelemid, DgnElementId tgtelemid, TxnTable::ChangeType changeType)
    {
    if (!m_stmt.IsValid() || !m_stmt->IsPrepared())
        {
        Utf8String sql("INSERT INTO " TEMP_TABLE(TXN_TABLE_RelationshipLinkTables) " (");
        sql.append(COLNAME_ECInstanceId).append(",");          // 1
        sql.append(COLNAME_ECClassId).append(",");             // 2
        sql.append(COLNAME_SourceECInstanceId).append(",");    // 3
        sql.append(COLNAME_TargetECInstanceId).append(",");    // 4
        sql.append(COLNAME_ChangeType);                        // 5
        sql.append(") VALUES(?,?,?,?,?)");
        m_stmt = m_txnMgr.GetTxnStatement(sql.c_str());
        }

    m_stmt->BindId(1, relid);
    m_stmt->BindId(2, relclsid);
    m_stmt->BindId(3, srcelemid);
    m_stmt->BindId(4, tgtelemid);
    m_stmt->BindInt(5, (int)changeType);

    auto rc = m_stmt->Step();
    BeAssert(rc == BE_SQLITE_DONE);

    m_stmt->Reset();
    m_stmt->ClearBindings();

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult dgn_TxnTable::RelationshipLinkTable::QueryTargets(DgnElementId& srcelemid, DgnElementId& tgtelemid, BeSQLite::EC::ECInstanceId relid)
    {
    //  SourceId and TargetId never change.
    auto selectOrig = m_txnMgr.GetDgnDb().GetCachedStatement(Utf8PrintfString("SELECT SourceId,TargetId FROM %s WHERE Id=?", m_tableName.c_str()).c_str());
    selectOrig->BindId(1, relid);
    auto rc = selectOrig->Step();
    if (BE_SQLITE_ROW != rc)
        return rc;

    srcelemid = selectOrig->GetValueId<DgnElementId>(0);
    tgtelemid = selectOrig->GetValueId<DgnElementId>(1);
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::UniqueRelationshipLinkTable::_UpdateSummary(Changes::Change change, ChangeType changeType)
    {
    m_changes = true;

    Changes::Change::Stage stage = (ChangeType::Insert == changeType) ? Changes::Change::Stage::New : Changes::Change::Stage::Old;

    //  Every table-per-class relationship link table is laid out like this:
    //      [0] Relationship instance ID
    //      [1] Relationship class ID
    //      [2] Source instance ID
    //      [3] Target instance ID
    //      [4...] relationship instance properties ...     which we DO NOT TRACK
    int const ECInstanceId_LTColId = 0;
    int const ECClassId_LTColId = 1;
    int const SourceECInstanceId_LTColId = 2;
    int const TargetECInstanceId_LTColId = 3;

    ECInstanceId relid = change.GetValue(ECInstanceId_LTColId, stage).GetValueId<ECInstanceId>();
    BeAssert(relid.IsValid());

    ECClassId relclsid = change.GetValue(ECClassId_LTColId, stage).GetValueId<ECClassId>();
    BeAssert(relclsid.IsValid());

    DgnElementId srcelemid, tgtelemid;
    if (ChangeType::Insert == changeType || ChangeType::Delete == changeType)
        {
        srcelemid = change.GetValue(SourceECInstanceId_LTColId, stage).GetValueId<DgnElementId>();
        tgtelemid = change.GetValue(TargetECInstanceId_LTColId, stage).GetValueId<DgnElementId>();
        }
    else
        {
        QueryTargets(srcelemid, tgtelemid, relid); //  SourceId and TargetId never change.
        }

    m_txnMgr.GetRelationshipLinkTables().Insert(relid, relclsid, srcelemid, tgtelemid, changeType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::RelationshipLinkTable::_OnValidated()
    {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes && TxnAction::Abandon != m_txnMgr.GetCurrentAction())
        {
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_RelationshipLinkTables));
        m_changes = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::AddTxnMonitor(TxnMonitor& monitor) {
    s_monitors.push_back(&monitor);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DropTxnMonitor(TxnMonitor& monitor) {
    auto it = std::find(s_monitors.begin(), s_monitors.end(), &monitor);
    if (it != s_monitors.end())
        *it = nullptr; // removed from list by CallMonitors
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::CallMonitors(std::function<void(TxnMonitor&)> caller) {
    for (auto curr = s_monitors.begin(); curr != s_monitors.end();) {
        if (*curr == nullptr)
            curr = s_monitors.erase(curr);
        else {
            try {
                caller(**curr);
            } catch (...) {
                BeAssert(false && "TxnMonitor threw an exception");
            }

            ++curr;
        }
    }
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnBeforeUndoRedo(bool isUndo) {
    auto jsTxns = m_dgndb.GetJsTxns();
    if (nullptr != jsTxns)
        m_dgndb.CallJsFunction(jsTxns, "_onBeforeUndoRedo", {Napi::Boolean::New(jsTxns.Env(), isUndo)});
}

BE_JSON_NAME(inserted);
BE_JSON_NAME(updated);
BE_JSON_NAME(deleted);
BE_JSON_NAME(range);
BE_JSON_NAME(ranges);
BE_JSON_NAME(id);
BE_JSON_NAME(ids);
BE_JSON_NAME(guid);

/**---------------------------------------------------------------------------------**//**
* A call to this method may happen for one of these circumstances:
*  1. _OnCommit - to save a set of changes as a Txn
*  2. ApplyChanges from undo/redo of a previously saved Txn
*  3. ApplyChanges from merging an externally generated changeset
* Then, it is only called when there are changes to GeometricModels, with `m_initTableHandlers`
* enabled. It uses the `m_modelChanges` member, set up from either the new changeset for
* case 1 above, or the applied changeset for cases 2 and 3.
* It keeps the model-based in-memory rangeIndex and tile trees up to date.
* It also emits the JavaScript `txns._onGeometryChanged` event that supplies:
*  - the list of changed models and their new GeometryGuid
*  - the list of inserted, updated and deleted elements for each model
*  - the new range for each element that was inserted or updated.
* It also calls the _OnGeometricModelChanges event, for tests.
* @note for non-interactive use cases (e.g. applying a changeset for connectors),
* it is assumed that there are no loaded models and therefore no in-memory constructs
* to keep current. Also, no events are generated. It may be that for applying very large
* changesets in interactive sessions, it may be preferable to unload all models and caches,
* apply the changes, and then re-start the session.
* @note for case 1 above, the range index is updated by element handlers in the OnInserted,
* OnUpdated, and OnDeleted methods, to ensure the rangeIndex is correct during the course of the transaction.
* Then, onCommit this method is called and they are re-updated, doing nothing. That's ok.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnGeometricModelChanges()   {
    auto& db = GetDgnDb();
    auto jsTxns = db.GetJsTxns();
    napi_value jsObj = jsTxns ? (napi_value)Napi::Array::New(jsTxns.Env()) : (napi_value)nullptr;
    BeJsDocument rapidDoc; // must be automatic variable. Do not combine with next line.
    BeJsValue json(jsObj ? BeJsValue(Napi::Value(jsTxns.Env(), jsObj)) : rapidDoc);

    auto const& changes = m_modelChanges.GetGeometryChanges();
    for (auto const& change : changes) {
        auto model = db.Models().Get<GeometricModel>(change.first);
        auto const& inserts = change.second.GetElements(TxnTable::ChangeType::Insert);
        auto const& updates = change.second.GetElements(TxnTable::ChangeType::Update);
        auto const& deletes = change.second.GetElements(TxnTable::ChangeType::Delete);

        if (model.IsValid() && (!inserts.empty() || !updates.empty() || !deletes.empty()))
            T_HOST.Visualization().AddChanges(*model, inserts, updates, deletes);

        auto entry = json.appendObject();
        entry[json_id()] = change.first;

        BeJsGeomUtils::DRange3dToJson(entry[json_range()], model.IsValid() ? model->QueryElementsRange() : DRange3d::NullRange());

        auto guid = model.IsValid() ? model->QueryGeometryGuid() : BeGuid(false);
        entry[json_guid()] = guid.ToString();

        if (!model.IsValid())
            continue;

        // both inserts and updates can use the same logic
        auto insertOrUpdate = [&](auto const& changed, auto name) {
            if (changed.empty())
                return;
            auto elems = entry[name];
            elems[json_ids()] = BeIdSet::ToCompactString(changed);
            auto ranges = elems[json_ranges()];
            for (auto const& elemId : changed) {
                auto range =  model->GetElementRange(elemId);
                model->UpdateElementRange(elemId, range); // update entry in range index (removes first, if present). For direct modifications this has already happened, but that's ok.
                BeJsGeomUtils::DRange3dToJson(ranges.appendValue(), range); // and save range for JavaScript event
            }
        };

        insertOrUpdate(inserts, json_inserted());
        insertOrUpdate(updates, json_updated());

        if (!deletes.empty()) {
            entry[json_deleted()] = BeIdSet::ToCompactString(deletes);
            for (auto const& elemId : deletes)
                model->RemoveFromRangeIndex(elemId); // make sure it is not in RangeIndex. For direct modifications this has already happened, but that's ok.
        }
    }
    CallMonitors([&](TxnMonitor& monitor) { monitor._OnGeometricModelChanges(*this, json); }); // this is really only for tests
    m_dgndb.CallJsFunction(jsTxns, "_onGeometryChanged", {jsObj});
}

/*---------------------------------------------------------------------------------**//**
* this method is called after a Txn is created, a Txn is undone/redone, or an incoming changeset is applied,
* and there were changes to the GeometryGuid of one or more models. It calls the JavaScript `txns.__onGeometryGuidsChanged`
* method with an array of [modelId,GeometryGuid] pairs.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnGeometryGuidChanges(bset<DgnModelId> const& modelIds) {
    CallMonitors([&](TxnMonitor& monitor) {
        // This is really only for tests.
        monitor._OnGeometryGuidChanges(*this, modelIds);
    });

    auto& db = GetDgnDb();
    Napi::Object jsTxns = db.GetJsTxns();
    if (jsTxns == nullptr)
        return;

    auto jsObj = Napi::Array::New(jsTxns.Env());
    auto json = BeJsValue(jsObj);
    for (auto modelId : modelIds) {
        auto model = db.Models().Get<GeometricModel>(modelId);
        if (model.IsNull())
            continue;

        auto guid = model->QueryGeometryGuid();
        auto entry = json.appendObject();
        entry[json_id()] = modelId;
        entry[json_guid()] = guid.ToString();
    }

    db.CallJsFunction(jsTxns, "_onGeometryGuidsChanged", {jsObj});
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnUndoRedo(TxnAction action) {
    auto jsTxns = m_dgndb.GetJsTxns();
    auto config = PullMergeConf::Load(m_dgndb);

    if (!config.InProgress()) {
        m_dgndb.SaveChanges();
    }

    if (nullptr != jsTxns) {
        BeAssert(TxnAction::Reverse == action || TxnAction::Reinstate == action);
        bool isUndo = TxnAction::Reverse == action;
        m_dgndb.CallJsFunction(jsTxns, "_onAfterUndoRedo", {Napi::Boolean::New(jsTxns.Env(), isUndo)});
    }

    CallMonitors([&, action](TxnMonitor& monitor) { monitor._OnUndoRedo(*this, action); });
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Model::Iterator::Entry dgn_TxnTable::Model::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        m_db->GetCachedStatement(m_stmt, "SELECT ModelId,ChangeType,ECClassId FROM " TEMP_TABLE(TXN_TABLE_Models));
    else
        m_stmt->Reset();

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId dgn_TxnTable::Model::Iterator::Entry::GetModelId() const {return m_sql->GetValueId<DgnModelId>(0);}
TxnTable::ChangeType dgn_TxnTable::Model::Iterator::Entry::GetChangeType() const {return (TxnTable::ChangeType) m_sql->GetValueInt(1);}
DgnClassId dgn_TxnTable::Model::Iterator::Entry::GetECClassId() const {return m_sql->GetValueId<DgnClassId>(2);}

#ifdef DEBUG_TxnManager_TXNS
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TxnManager::DumpTxns(bool verbose) {
    TxnId endTxnId = GetCurrentTxnId();
    TxnId startTxnId = QueryNextTxnId(TxnId(0));
    if (!startTxnId.IsValid() || startTxnId >= endTxnId)
        return;

    DgnDbR db = (DgnDbR)m_dgndb;
    for (TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = QueryNextTxnId(currTxnId)) {
        Utf8PrintfString title("\n\n---------- Txn#%lld ---------------", currTxnId.GetValue());
        if (IsSchemaChangeTxn(currTxnId)) {
            DbSchemaChangeSet dbSchemaChangeSet;
            ReadDbSchemaChanges(dbSchemaChangeSet, currTxnId);
            dbSchemaChangeSet.Dump(Utf8PrintfString("%s - schemaChanges", title.c_str()).c_str());
        } else {
            ChangeSet sqlChangeSet;
            ReadDataChanges(sqlChangeSet, currTxnId, TxnAction::None);

            if (verbose)
                sqlChangeSet.Dump(title.c_str(), db, false, 2);
            else {
                printf("%s\n\n", title.c_str());
                ChangeSummary changeSummary(db);
                changeSummary.FromChangeSet(sqlChangeSet);
                changeSummary.Dump();
            }
        }
    }
}
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::PullMergeUpdateTxn(ChangeSetCR changeSet, TxnId id) {
    if (0 == changeSet.GetSize()) {
        BeAssert(false);
        LOG.error("called UpdateTxn for empty changeset");
        return BE_SQLITE_ERROR;
    }

    if (changeSet.GetSize() > MAX_REASONABLE_TXN_SIZE) {
        LOG.warningv("changeset size %" PRIu64 " exceeds recommended limit. Please investigate.", changeSet.GetSize());
    }

    if (changeSet.GetSize() > MAX_TXN_SIZE) {
        LOG.fatalv("changeset size %" PRIu64 " exceeds maximum. Panic stop to avoid loss! You must now abandon this briefcase.", changeSet.GetSize());
        return BE_SQLITE_ERROR;
    }

    CachedStatementPtr stmt = GetTxnStatement("UPDATE " DGN_TABLE_Txns " SET [Change]=?, [Deleted]=? WHERE [Id]=?");
    enum Column : int { Id=3, Deleted=2, Change=1 };
    stmt->BindInt64(Column::Id, id.GetValue());
    stmt->BindInt(Column::Deleted, false);

    m_snappyTo.Init();
    uint32_t csetSize = (uint32_t) changeSet.GetSize();
    ChangesBlobHeader header(csetSize);
    m_snappyTo.Write((Byte const*) &header, sizeof(header));
    for (auto const& chunk : changeSet.m_data.m_chunks) {
        m_snappyTo.Write(chunk.data(), (uint32_t) chunk.size());
    }

    DbResult rc = BE_SQLITE_OK;
    const uint32_t zipSize = m_snappyTo.GetCompressedSize();
    if (0 < zipSize ) {
        if (1 == m_snappyTo.GetCurrChunk())
            rc = stmt->BindBlob(Column::Change, m_snappyTo.GetChunkData(0), zipSize, Statement::MakeCopy::No);
        else
            rc = stmt->BindZeroBlob(Column::Change, zipSize); // more than one chunk

        if (BE_SQLITE_OK != rc) {
            BeAssert(false);
            return rc;
        }
    }

    if (changeSet.GetSize() > MAX_REASONABLE_TXN_SIZE/2) {
        LOG.infov("Updating large changeset. Size=%" PRIuPTR ", Compressed size=%" PRIu32, changeSet.GetSize(), m_snappyTo.GetCompressedSize());
    }

    rc = stmt->Step();
    if (BE_SQLITE_DONE != rc) {
        LOG.errorv("SaveChangeSet failed: %s", BeSQLiteLib::GetErrorName(rc));
        BeAssert(false);
        return rc;
    }

    if (1 == m_snappyTo.GetCurrChunk()) {
        return BE_SQLITE_OK;
    }

    rc = m_snappyTo.SaveToRow(m_dgndb, DGN_TABLE_Txns, "Change", id.GetValue());
    if (BE_SQLITE_OK != rc) {
        LOG.errorv("UpdateTxn failed to save to row: %s", BeSQLiteLib::GetErrorName(rc));
        BeAssert(false);
        return rc;
    }

    return rc;
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeResume() {
    auto conf = PullMergeConf::Load(m_dgndb);
    if (!conf.IsRebasingLocalChanges()) {
        return;
    }
    PullMergeEnd();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeBegin() {
  PullMergeReverseLocalChanges();
}

struct MakeQueryOnly {
    DbCR m_db;
    MakeQueryOnly(DbCR db) : m_db(db)   {
        m_db.ExecuteSql("PRAGMA query_only = 1");
    }
    ~MakeQueryOnly() {
        m_db.ExecuteSql("PRAGMA query_only = 0");
    }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<TxnManager::TxnId> TxnManager::PullMergeReverseLocalChanges() {
  auto conf = PullMergeConf::Load(m_dgndb);
    if (conf.InProgress()) {
        m_dgndb.ThrowException("operation failed: pull merge in progress.", BE_SQLITE_ERROR);
    }

    if (HasChanges()) {
        m_dgndb.ThrowException("cannot processed due to unsaved changes.", BE_SQLITE_ERROR);
    }

    if (m_dgndb.IsReadonly() && !HasPendingTxns()) {
        m_dgndb.ThrowException("file is readonly", (int) ChangesetStatus::CannotMergeIntoReadonly);
    }
    
    while(!m_multiTxnOp.empty()) {
        EndMultiTxnOperation();
    }
    
    DeleteReversedTxns();
    TxnId startTxnId = QueryNextTxnId(TxnId(0));
    TxnId endTxnId = GetCurrentTxnId();
    std::vector<TxnId> reversedTxns;
    if (startTxnId < endTxnId) {
        OnBeforeUndoRedo(true);
        for (TxnId curr = QueryPreviousTxnId(endTxnId); curr.IsValid() && curr >= startTxnId; curr = QueryPreviousTxnId(curr)) {
            LOG.infov("Reversing TxnId: %s, Descr: %s", BeInt64Id(curr.GetValue()).ToHexStr().c_str(),GetTxnDescription(curr).c_str());
            auto rc = ApplyTxnChanges(curr, TxnAction::Reverse);
            if (BE_SQLITE_OK != rc) {
                m_dgndb.AbandonChanges();
                Utf8String err = SqlPrintfString("PullMergeBegin(): unable to reverse local changes for txn %s", BeInt64Id(curr.GetValue()).ToHexStr().c_str()).GetUtf8CP();
                m_dgndb.ThrowException(err.c_str(), rc);
            }
            reversedTxns.insert(reversedTxns.begin(), curr);
        }
    }
 

    BeAssert(HasPendingTxns() == false);
    BeAssert(HasDataChanges() == false);

    const auto st = conf.SetMergeStage(PullMergeStage::Merging)
                .SetEndTxnId(endTxnId)
                .SetStartTxnId(startTxnId)
                .Save(m_dgndb);

    if (st != BE_SQLITE_DONE) {
        m_dgndb.ThrowException("PullMergeBegin(): fail to save pull-merge conf ", (int)st);
    }
    CallMonitors([&](TxnMonitor& monitor) { monitor._OnPullMergeBegin(*this); });
    return reversedTxns;
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::PullMergeEraseTxn(TxnId txnId) {    
    LOG.infov("Erasing Txn with id: %" PRIX64, txnId.GetValue());
    if (!txnId.IsValid()) {
        LOG.error("cannot delete a null Txn");
        return false;
    }

    auto stmt = GetTxnStatement("DELETE FROM " DGN_TABLE_Txns " WHERE Id=?");
    if (stmt == nullptr) {
        LOG.error("failed to prepare statement to delete Txn");
        return false;
    }

    stmt->BindUInt64(1, txnId.GetValue());
    auto rc = stmt->Step();
    if (rc != BE_SQLITE_DONE) {
        LOG.errorv("failed to delete Txn: %s", BeSQLiteLib::GetErrorName(rc));
        return false;
    }
    BeAssert(m_dgndb.GetModifiedRowCount() == 1); // we should have deleted exactly one row
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeAbortRebase(TxnId id, Utf8String err, DbResult rc){
    auto conf = PullMergeConf::Load(m_dgndb);
    if (!conf.InProgress()) {
        return;
    }
    LOG.error(err.c_str());
    m_dgndb.ThrowException(err.c_str(), static_cast<int>(rc));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeSetTxnActive(TxnId txnId) {
    const auto idStr = BeInt64Id(txnId.GetValue()).ToHexStr();
    CachedStatementPtr stmt = GetTxnStatement("UPDATE " DGN_TABLE_Txns " SET Deleted=? WHERE Id=?");
    stmt->BindInt(1, false);
    stmt->BindInt64(2, txnId.GetValue());
    auto rc = stmt->Step();
    if (rc != BE_SQLITE_DONE) {
        PullMergeAbortRebase(txnId, SqlPrintfString("unable to save rebased local txn (id: %s)", idStr.c_str()).GetUtf8CP(), rc);
    }
    m_curr = txnId;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeRebaseReinstateTxn() {
    auto conf = PullMergeConf::Load(m_dgndb);
    if (!conf.IsRebasingLocalChanges()){
         m_dgndb.ThrowException("PullMergeRebaseReinstateTxn(): pull merge not in progress.", BE_SQLITE_ERROR);
    }

    auto txnId = conf.GetInProgressRebaseTxnId();
    if (!txnId.IsValid()) {
         m_dgndb.ThrowException("PullMergeRebaseReinstateTxn(): in progress rebase txn id is not set", BE_SQLITE_ERROR);
    }

    const auto type = GetTxnType(txnId);
    const auto desc = GetTxnDescription(txnId);
    const auto currTxnIdStr = BeInt64Id(txnId.GetValue()).ToHexStr();
    if (type == TxnType::Ddl){
        return;
    }

    const auto isSchemaTxn = type == TxnType::EcSchema;
    LocalChangeSet changeset(GetDgnDb(), txnId, type, desc);
    auto rc = ReadDataChanges(changeset, txnId, TxnAction::None);
    if (rc != BE_SQLITE_OK) {
        PullMergeAbortRebase(txnId, "failed to read data changes", rc);
    }

    rc = ApplyChanges(changeset, TxnAction::Merge, isSchemaTxn, false);
    if (rc != BE_SQLITE_OK) {
        if (changeset.GetLastErrorMessage().empty())
            PullMergeAbortRebase(txnId, "failed to apply changes", rc);
        else
            PullMergeAbortRebase(txnId, changeset.GetLastErrorMessage(), rc);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeRebaseUpdateTxn() {
    auto conf = PullMergeConf::Load(m_dgndb);
    if (!conf.IsRebasingLocalChanges()){
         m_dgndb.ThrowException("PullMergeRebaseReinstateTxn(): pull merge not in progress.", BE_SQLITE_ERROR);
    }

    auto txnId = conf.GetInProgressRebaseTxnId();
    if (!txnId.IsValid()) {
         m_dgndb.ThrowException("PullMergeRebaseReinstateTxn(): in progress rebase txn id is not set", BE_SQLITE_ERROR);
    }

    const auto type = GetTxnType(txnId);
    if(type == TxnType::Ddl) {
        PullMergeSetTxnActive(txnId);
        return;
    }

    const auto idStr = BeInt64Id(txnId.GetValue()).ToHexStr();
    ChangeSet rebasedChangeset;
    auto rc = rebasedChangeset.FromChangeTrack(*this);
    if (rc != BE_SQLITE_OK) {
        PullMergeAbortRebase(txnId, SqlPrintfString("failed to create update changeset (id: %s)", idStr.c_str()).GetUtf8CP(), rc);
    }

    Restart();
    const auto mergeNeeded = HasPendingTxns() && m_initTableHandlers;
    ChangeSet indirectChanges;
    if (!mergeNeeded) {
        OnBeginValidate();
        OnValidateChanges(rebasedChangeset);
        if (SUCCESS != PropagateChanges()) {
            PullMergeAbortRebase(txnId, SqlPrintfString("failed to propagate changes (id: %s)", idStr.c_str()).GetUtf8CP(), rc);
        }

        if (HasDataChanges()) {
            rc = indirectChanges.FromChangeTrack(*this);
            if (BE_SQLITE_OK != rc) {
                BeAssert(false);
                LOG.fatalv("EndPullApplyChanges failed at indirectDataChangeSet.FromChangeTrack(): %s", BeSQLiteLib::GetErrorName(rc));
                if (BE_SQLITE_NOMEM == rc)
                    throw std::bad_alloc();

                PullMergeAbortRebase(txnId, SqlPrintfString("failed to propagate changes (id: %s)", idStr.c_str()).GetUtf8CP(), rc);
            }
            Restart();
            rc = rebasedChangeset.ConcatenateWith(indirectChanges);
            if(rc != BE_SQLITE_OK) {
                PullMergeAbortRebase(txnId, SqlPrintfString("failed to combine rebased changeset with propagated changes (id: %s)", idStr.c_str()).GetUtf8CP(), rc);
            }
        }
        OnEndValidate();
    }

    if(rebasedChangeset._IsEmpty()){
        if (!PullMergeEraseTxn(txnId)) {
            PullMergeAbortRebase(txnId, SqlPrintfString("unable to erase empty txn (id: %s)", idStr.c_str()).GetUtf8CP(), BE_SQLITE_ERROR);
        }
    } else {
        rc = PullMergeUpdateTxn(rebasedChangeset, txnId);
        if (rc != BE_SQLITE_OK) {
            PullMergeAbortRebase(txnId, SqlPrintfString("unable to save rebased local txn (id: %s)", idStr.c_str()).GetUtf8CP(), rc);
        }
    }
    Restart();
    rc = m_dgndb.SaveChanges(); // save dgn_txn/be_Local
    if (rc != BE_SQLITE_OK) {
        PullMergeAbortRebase(txnId, SqlPrintfString("unable to save rebased txn (id: %s)", idStr.c_str()).GetUtf8CP(), rc);
    }
    m_curr = txnId;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<TxnManager::TxnId> TxnManager::PullMergeRebaseBegin() {
    auto conf = PullMergeConf::Load(m_dgndb);
    if (!conf.InProgress()) {
        m_dgndb.ThrowException("PullMergeRebaseBegin(): pull merge not in progress.", BE_SQLITE_ERROR);
        return {};
    }

    if (!conf.IsRebasingLocalChanges()) {
        conf.SetMergeStage(PullMergeStage::Rebasing).Save(m_dgndb);
        m_dgndb.SaveChanges();
    }

    TxnId startTxnId = QueryNextTxnId(TxnId(0));
    TxnId endTxnId = conf.GetEndTxnId();

    std::vector<TxnId> txnsToBeRebased;
    for (TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = QueryNextTxnId(currTxnId)) {
        txnsToBeRebased.push_back(currTxnId);
    }
    return txnsToBeRebased;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::PullMergeRebaseNext() {
    auto conf = PullMergeConf::Load(m_dgndb);
    if (!conf.IsRebasingLocalChanges()){
        m_dgndb.ThrowException("PullMergeRebaseEnd(): not rebasing local changes", BE_SQLITE_ERROR);
    }

    if (HasChanges()) {
        m_dgndb.ThrowException("PullMergeRebaseNext(): expect no unsaved changes", BE_SQLITE_ERROR);
    }    
    auto id = conf.GetInProgressRebaseTxnId();
    if (id.IsValid()) {
        id = QueryNextTxnId(id);
    } else {
        id = QueryNextTxnId(TxnId(0));
    }

    conf.SetInProgressRebaseTxnId(id).Save(m_dgndb);
    m_dgndb.SaveChanges();
    return id;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeRebaseAbortTxn() {
    auto conf = PullMergeConf::Load(m_dgndb);
    if (!conf.IsRebasingLocalChanges()){
        m_dgndb.ThrowException("PullMergeRebaseAbortTxn(): not rebasing local changes", BE_SQLITE_ERROR);
    }

    conf.ResetInProgressRebaseTxnId().Save(m_dgndb);
    m_dgndb.AbandonChanges();
    conf.ResetInProgressRebaseTxnId().Save(m_dgndb);
    m_dgndb.SaveChanges();   
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeRebaseEnd() {
    auto conf = PullMergeConf::Load(m_dgndb);
    if (!conf.IsRebasingLocalChanges()){
        m_dgndb.ThrowException("PullMergeRebaseEnd(): not rebasing local changes", BE_SQLITE_ERROR);
    }

    m_curr = conf.GetEndTxnId();
    m_reversedTxn.clear();
    conf.SetMergeStage(PullMergeStage::None)
        .SetEndTxnId(TxnId(0))
        .SetStartTxnId(TxnId(0))
        .Save(m_dgndb);

    const auto rc = m_dgndb.SaveChanges();
    if (rc != BE_SQLITE_OK ){
        m_dgndb.ThrowException("Unable to save merge state", static_cast<int>(rc));
    }
    Restart();
    CallMonitors([&](TxnMonitor& monitor) { monitor._OnPullMergeEnd(*this); });
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeEnd() {
   PullMergeRebaseBegin();
    while(PullMergeRebaseNext().IsValid()){
       PullMergeRebaseReinstateTxn();
       PullMergeRebaseUpdateTxn();
   }
   PullMergeRebaseEnd();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::PullMergeEraseConf() {
    if (!HasPendingTxns()){
        PullMergeConf::Remove(m_dgndb);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::PullMergeStage TxnManager::PullMergeGetStage() const {
    return PullMergeConf::Load(m_dgndb).GetMergeStage();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::PullMergeInProgress() const {
    return PullMergeConf::Load(m_dgndb).InProgress();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangesetStatus TxnManager::PullMergeApply(ChangesetPropsCR revision){
    PullMergeBegin();
    auto rc = MergeChangeset(revision, false);
    PullMergeEnd();
    return rc;
}
