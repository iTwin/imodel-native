/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_UNNAMED_NAMESPACE

typedef bvector<TxnMonitorP> TxnMonitors;
static TxnMonitors s_monitors;
static T_OnCommitCallback s_onCommitCallback = nullptr;

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
    Utf8CP tableName;
    int nCols, indirect;
    DbOpcode opcode;
    change.GetOperation(&tableName, &nCols, &opcode, &indirect);

    if (cause == ConflictCause::NotFound) {
        if (opcode == DbOpcode::Delete)      // a delete that is already gone.
            return ConflictResolution::Skip; // This is caused by propagate delete on a foreign key. It is not a problem.
        if (opcode == DbOpcode::Update)
            return ConflictResolution::Skip; // caused by inserting row and then updating it in the same txn and then undoing the txn. It's not a problem.
    } else if (ConflictCause::Data == cause) {
        if (DbOpcode::Delete == opcode)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::SaveRebase(int64_t& id, Rebase const& rebase)
    {
    if (!m_enableRebasers)
        return BE_SQLITE_DONE;

    BeAssert(0 != rebase.GetSize());

    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE_Rebase "(Rebase) VALUES(?)");
    stmt.BindBlob(1, rebase.GetData(), rebase.GetSize(), Statement::MakeCopy::No);

    auto rc = stmt.Step();
    if (BE_SQLITE_DONE == rc)
        id = m_dgndb.GetLastInsertRowId();
    else
        {
        BeAssert(false);
        }
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::LoadRebases(Rebaser& rebaser, int64_t thruId)
    {
    if (!m_enableRebasers) {
        BeAssert(false && "rebasers are not enabled for the DgnDb");
        return BE_SQLITE_OK;
    }

    Statement stmt(m_dgndb, "SELECT Rebase FROM " DGN_TABLE_Rebase " WHERE (Id <= ?)");
    stmt.BindInt64(1, thruId);

    DbResult rc;
    while (BE_SQLITE_ROW == (rc = stmt.Step())) {
        rc = rebaser.AddRebase(stmt.GetValueBlob(0), stmt.GetColumnBytes(0));
        if (rc != BE_SQLITE_OK)
            return rc;
    }

    return BE_SQLITE_DONE == rc ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DeleteRebases(int64_t id) {
    if (!m_enableRebasers || id==0)
        return;

    Statement stmt(m_dgndb, "DELETE FROM " DGN_TABLE_Rebase " WHERE (Id <= ?)");
    stmt.BindInt64(1, id);
    auto result = stmt.Step();
    UNUSED_VARIABLE(result);
    BeAssert(BE_SQLITE_DONE == result);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t TxnManager::QueryLastRebaseId() {
    if (!m_enableRebasers || !m_dgndb.TableExists(DGN_TABLE_Rebase))
        return 0;

    Statement stmt(m_dgndb, "SELECT MAX(Id) FROM " DGN_TABLE_Rebase);
    return BE_SQLITE_ROW == stmt.Step() ? stmt.GetValueInt64(0) : 0;
}

/*---------------------------------------------------------------------------------**//**
* Read the description of a Txn
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetTxnDescription(TxnId rowid) const {
    Statement stmt(m_dgndb, "SELECT Operation FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt.BindInt64(1, rowid.GetValue());
    return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueText(0) : "";
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

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::Initialize() {
    m_action = TxnAction::None;
    Statement stmt(m_dgndb, "SELECT MAX(Id) FROM " DGN_TABLE_Txns " WHERE Deleted=0");
    DbResult result = stmt.Step();
    UNUSED_VARIABLE(result);
    BeAssert(result == BE_SQLITE_ROW);

    TxnId last = stmt.GetValueInt64(0); // this is where we left off last session
    m_curr = TxnId(SessionId(last.GetSession().GetValue()+1), 0); // increment the session id, reset to index to 0.
    m_reversedTxn.clear();
    m_dynamicTxns.clear();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnManager(DgnDbR dgndb) : m_dgndb(dgndb), m_stmts(20), m_rlt(*this), m_initTableHandlers(false), m_modelChanges(*this) {
    m_dgndb.SetChangeTracker(this);
    m_enableRebasers = m_dgndb.TableExists(DGN_TABLE_Rebase);
    Initialize();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::InitializeTableHandlers() {
    if (!m_isTracking) {
        BeAssert(false && "Tracking must be enabled before initializing table handlers");
        return BE_SQLITE_ERROR;
    }

    if (m_initTableHandlers || m_dgndb.IsReadonly())
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
    if (m_dgndb.IsReadonly())
        return;

    // we can get called multiple times with the same tablehandler. Ignore all but the first one.
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
    if (InDynamicTxn())
        {
        BeAssert(false);
        return DgnDbStatus::InDynamicTransaction;
        }

    m_multiTxnOp.push_back(GetCurrentTxnId());
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::EndMultiTxnOperation()
    {
    if (InDynamicTxn())
        {
        BeAssert(false);
        return DgnDbStatus::InDynamicTransaction;
        }

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
    BeAssert(false == m_indirectChanges); // should never be recursive
    AutoRestore<bool> saveIndirect(&m_indirectChanges, true); // so we can tell whether we're propagating changes from JavaScript
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
* When journalling changes, SQLite calls this method to determine whether changes to a specific table are eligible or not.
* @note tables with no primary key are skipped automatically.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TrackChangesForTable TxnManager::_FilterTable(Utf8CP tableName) {
    // Skip these tables - they hold redundant data that will be automatically updated when the changeset is applied
    return (
               TABLE_NAME_STARTS_WITH(DGN_TABLE_Txns) ||
               TABLE_NAME_STARTS_WITH(DGN_VTABLE_SpatialIndex) ||
               TABLE_NAME_STARTS_WITH(DGN_TABLE_Rebase) ||
               TABLE_NAME_STARTS_WITH("ec_cache_") ||
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
            LOG.error("invalid change in changset");
            BeAssert(false && "invalid change in changeset");
            continue;
        }

        if (0 != strcmp(currTable.c_str(), tableName)) { // changes within a changeset are grouped by table
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

        if (0 != strcmp(currTable.c_str(), tableName)) { // changes within a changeset are grouped by table
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

    BeAssert(m_action == TxnAction::None);
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
    ModelChangesScope v_v_v_(*this);

    bool hasEcSchemaChanges = m_hasEcSchemaChanges;
    m_hasEcSchemaChanges = false;

    BeAssert(!InDynamicTxn() && "dynamic change tracker active");
    CancelDynamics();

    DdlChanges ddlChanges = std::move(m_ddlChanges);

    UndoChangeSet dataChangeSet;
    if (HasDataChanges()) {
        DbResult result = dataChangeSet.FromChangeTrack(*this);

        Restart();  // Clear the change tracker since we copied any changes to dataChangeSet

        if (BE_SQLITE_OK != result) {
            LOG.errorv("failed to create a data Changeset: %s", BeSQLiteLib::GetErrorName(result));
            BeAssert(false && "dataChangeSet.FromChangeTrack failed");
            return OnCommitStatus::Abort;
        }
    }

    if (s_onCommitCallback != nullptr) { // allow a test to inject a failure
        if (CallbackOnCommitStatus::Continue != s_onCommitCallback(*this, isCommit, operation, dataChangeSet, ddlChanges))
            return OnCommitStatus::Abort;
    }

    if (dataChangeSet._IsEmpty() && ddlChanges._IsEmpty())
        return OnCommitStatus::NoChanges;

    if (!isCommit) { // this is a call to AbandonChanges, perform the rollback and notify table handlers
        DbResult rc = m_dgndb.ExecuteSql("ROLLBACK");
        if (rc != BE_SQLITE_OK)
            return OnCommitStatus::Abort;

        if (!dataChangeSet._IsEmpty())
            OnRollback(dataChangeSet);

        return OnCommitStatus::Completed; // we've already done the rollback, tell BeSQLite not to try to do it
    }

    // NOTE: you can't delete reversed Txns before this line, because on rollback and they come back! That's OK,
    // just leave them reversed and they'll get thrown away on the next commit (or reinstated.)
    DeleteReversedTxns(); // these Txns are no longer reachable.

    if (!dataChangeSet._IsEmpty() && m_initTableHandlers) { // we cannot propagate changes without table handlers - happens for schema upgrades
        OnBeginValidate();

        OnValidateChanges(dataChangeSet);

        BentleyStatus status = PropagateChanges();   // Propagate to generate indirect changes
        if (SUCCESS != status) {
            LOG.error("propagate changes failed");
            return OnCommitStatus::Abort;
        }

        // This loop is due to the fact that when we propagate changes, we can dirty models.
        // Then, when we call OnValidateChanges below, it creates another change to the last-mod-time and geometry-GUIDs of the changed models.
        // We need to add that to the changeset too. This loop should never execute more than twice.
        while (HasDataChanges()) {  // do we have any indirect data changes captured in the tracker?
            UndoChangeSet indirectDataChangeSet;
            DbResult result = indirectDataChangeSet.FromChangeTrack(*this);
            if (BE_SQLITE_OK != result)
                {
                BeAssert(false && "indirectDataChangeSet.FromChangeTrack failed");
                LOG.fatalv("failed to create indirect changeset: %s", BeSQLiteLib::GetErrorName(result));
                if (BE_SQLITE_NOMEM == result)
                    throw std::bad_alloc();
                return OnCommitStatus::Abort;
                }
            Restart();
            OnValidateChanges(indirectDataChangeSet);

            // combine direct and indirect changes into a single dataChangeSet
            result = dataChangeSet.ConcatenateWith(indirectDataChangeSet);
            if (BE_SQLITE_OK != result){
                LOG.errorv("failed to concatenate indirect changes: %s", BeSQLiteLib::GetErrorName(result));
                return OnCommitStatus::Abort;
            }
        }
    }

    if (!ddlChanges._IsEmpty()) {
        DbResult result = SaveTxn(ddlChanges, operation, TxnType::Ddl);
        if (result != BE_SQLITE_OK) {
            LOG.errorv("failed to save ddl changes: %s", BeSQLiteLib::GetErrorName(result));
            return OnCommitStatus::Abort;
        }
    }

    if (!dataChangeSet._IsEmpty()) {
        DbResult result = SaveTxn(dataChangeSet, operation, hasEcSchemaChanges ? TxnType::EcSchema : TxnType::Data);
        if (result != BE_SQLITE_OK) {
            LOG.errorv("failed to save Txn: %s", BeSQLiteLib::GetErrorName(result));
            return OnCommitStatus::Abort;
        }
    }

    // At this point, all of the changes to all tables have been applied. Tell TxnMonitors
    NotifyOnCommit();

    if (!dataChangeSet._IsEmpty() && m_initTableHandlers) // we cannot validate without table handlers - happens for schema upgrades
        OnEndValidate();

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
RevisionStatus TxnManager::MakeDdlChangesFromRevision(DgnRevisionCR revision, RevisionChangesFileReader& changeStream)  {
    bool containsSchemaChanges;
    DdlChanges ddlChanges;
    DbResult result = changeStream.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
    if (result != BE_SQLITE_OK)     {
        BeAssert(false);
        return RevisionStatus::ApplyError;
    }

    if (ddlChanges._IsEmpty())
        return RevisionStatus::Success;

    result = ApplyDdlChanges(ddlChanges);
    if (BE_SQLITE_OK != result)
        return RevisionStatus::ApplyError;

    return RevisionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionStatus TxnManager::MergeDataChangesInRevision(DgnRevisionCR revision, RevisionChangesFileReader& changeStream, bool containsSchemaChanges) {
    // if we don't have any pending txns, this is merely an Apply, no merging or propagation needed.
    bool mergeNeeded = HasPendingTxns() && m_initTableHandlers; // if tablehandlers are not present we can't merge - happens for schema upgrade
    Rebase rebase;

    DbResult result = ApplyChanges(changeStream, TxnAction::Merge, containsSchemaChanges, mergeNeeded ? &rebase : nullptr);
    if (result != BE_SQLITE_OK) {
        LOG.fatalv("MergeDataChangesInRevision failed to ApplyChanges: %s", BeSQLiteLib::GetErrorName(result));
        return RevisionStatus::ApplyError;
    }

    RevisionStatus status = RevisionStatus::Success;
    UndoChangeSet indirectChanges;

    // We only need to propagate changes (run dependency rules) if there have been are local changes that need to be validated against the incoming changes.
    if (mergeNeeded) {
        OnBeginValidate();

        OnValidateChanges(changeStream);

        if (SUCCESS != PropagateChanges()) {
            LOG.error("MergeDataChangesInRevision failed to propagate changes");
            status = RevisionStatus::MergePropagationError;
        }

        if (HasDataChanges()) {
            result = indirectChanges.FromChangeTrack(*this);
            if (BeSQLite::BE_SQLITE_OK != result) {
                BeAssert(false);
                LOG.fatalv("MergeDataChangesInRevision failed at indirectDataChangeSet.FromChangeTrack(): %s", BeSQLiteLib::GetErrorName(result));
                if (BE_SQLITE_NOMEM == result)
                    throw std::bad_alloc();
                return RevisionStatus::SQLiteError;
            }
            Restart();

            if (status == RevisionStatus::Success) {
                NotifyOnCommit();

                Utf8String mergeComment = "Merged";
                if (!revision.GetSummary().empty()) {
                    mergeComment.append(": ");
                    mergeComment.append(revision.GetSummary());
                }

                result = SaveTxn(indirectChanges, mergeComment.c_str(), TxnType::Data);
                if (BE_SQLITE_OK != result) {
                    LOG.fatalv("MergeDataChangesInRevision failed saving changes %s", BeSQLiteLib::GetErrorName(result));
                    BeAssert(false);
                    status = RevisionStatus::SQLiteError;
                }
            }
        }
        OnEndValidate();
    }

    if ((RevisionStatus::Success == status) && mergeNeeded && 0 != rebase.GetSize()) {
        int64_t rebaseId;
        result = SaveRebase(rebaseId, rebase);
        if (BE_SQLITE_DONE != result) {
            BeAssert(false);
            status = RevisionStatus::SQLiteError;
        }
    }

    if (status == RevisionStatus::Success) {
        status = m_dgndb.Revisions().SaveParentRevision(revision.GetChangesetId(), revision.GetChangesetIndex());

        if (status == RevisionStatus::Success) {
            result = m_dgndb.SaveChanges();
            // Note: All that the above operation does is to COMMIT the current Txn and BEGIN a new one.
            // The user should NOT be able to revert the revision id by a call to AbandonChanges() anymore, since
            // the merged changes are lost after this routine and cannot be used for change propagation anymore.
            if (BE_SQLITE_OK != result) {
                LOG.fatalv("MergeDataChangesInRevision failed to save: %s", BeSQLiteLib::GetErrorName(result));
                BeAssert(false);
                status = RevisionStatus::SQLiteError;
            }
        }
    }
    if (status != RevisionStatus::Success) {
        // we were unable to merge the changes or save the revisionId, but the revision's changes were successfully applied. Back them out, plus any indirect changes.
        ChangeGroup changeGroup;
        changeStream.AddToChangeGroup(changeGroup);
        if (indirectChanges.IsValid()) {
            result = indirectChanges.AddToChangeGroup(changeGroup);
            BeAssert(result == BE_SQLITE_OK);
        }

        UndoChangeSet allChanges;
        allChanges.FromChangeGroup(changeGroup);
        result = ApplyChanges(allChanges, TxnAction::Reverse, containsSchemaChanges, nullptr, true);
        BeAssert(result == BE_SQLITE_OK);
    }
    return status;
}

/*---------------------------------------------------------------------------------**//**
 * @remarks Used only in revering or reinstating revisions.
 * @see TxnManager::MergeRevision
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionStatus TxnManager::ReverseRevision(DgnRevisionCR revision) {
    BeFileNameCR revisionChangesFile = revision.GetRevisionChangesFile();
    RevisionChangesFileReader changeStream(revisionChangesFile, m_dgndb);
    bool containsSchemaChanges = revision.ContainsSchemaChanges(m_dgndb);

    if (!containsSchemaChanges) {
        // Skip the entire schema change set when reversing or reinstating - DDL and
        // the meta-data changes. Reversing meta data changes cause conflicts - see
        // TFS#149046
        DbResult result = ApplyChanges(changeStream, TxnAction::Reverse, false, nullptr, true);
        if (result != BE_SQLITE_OK)
            return RevisionStatus::ApplyError;
    }

    RevisionStatus status = m_dgndb.Revisions().SaveParentRevision(revision.GetParentId(), -1);

    if (status == RevisionStatus::Success) {
        DbResult result = m_dgndb.SaveChanges();
        if (BE_SQLITE_OK != result) {
            LOG.errorv("ApplyRevision failed at to save: %s",
                       BeSQLiteLib::GetErrorName(result));
            BeAssert(false);
            status = RevisionStatus::SQLiteError;
        }
    }

    if (status != RevisionStatus::Success) {
        BeAssert(!containsSchemaChanges &&
                 "Never attempt to reverse or reinstate schema changes");
        ApplyChanges(changeStream, TxnAction::Reinstate, false, nullptr, false);
    }
    return status;
}

/*---------------------------------------------------------------------------------**/ /**
 * Merge changes from a changeStream that originated in an external repository
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionStatus TxnManager::MergeRevision(DgnRevisionCR revision)
    {
    BeAssert(!InDynamicTxn());

    BeFileNameCR revisionChangesFile = revision.GetRevisionChangesFile();
    RevisionChangesFileReader changeStream(revisionChangesFile, m_dgndb);

    bool containsSchemaChanges = revision.ContainsSchemaChanges(m_dgndb);

    RevisionStatus status;
    if (containsSchemaChanges)
        {
        // Note: Schema changes may not necessary imply ddl changes. They could just be 'minor' ecschema/mapping changes.
        status = MakeDdlChangesFromRevision(revision, changeStream);
        if (RevisionStatus::Success != status)
            return status;
        }

    return MergeDataChangesInRevision(revision, changeStream, containsSchemaChanges);
    }

/*---------------------------------------------------------------------------------**//**
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
        m_determinedStatus = true;
        m_status = Status::Readonly;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::ModelChanges::Status TxnManager::ModelChanges::DetermineStatus()
    {
    if (!m_determinedStatus)
        {
        m_determinedStatus = true;
        if (m_mgr.GetDgnDb().GetGeometricModelUpdateStatement().IsValid())
            m_status = Status::Success;
        else
            Disable();
        }

    return m_status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::ModelChanges::Status TxnManager::ModelChanges::SetTrackingGeometry(bool track)
    {
    DetermineStatus();
    if (IsDisabled() || track == m_trackGeometry)
        return m_status;

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

    return m_status;
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
    if (IsDisabled())
        return;

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
    DetermineStatus();
    if (IsDisabled())
        return;

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

/*---------------------------------------------------------------------------------**//**
* Apply a changeset to the database. Notify all TxnTables about what was in the Changeset afterwards.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ApplyChanges(ChangeStreamCR changeset, TxnAction action, bool containsSchemaChanges, Rebase* rebase, bool invert) {
    BeAssert(action != TxnAction::None);
    AutoRestore<TxnAction> saveAction(&m_action, action);

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


    if (true) {
        DisableTracking _v(*this);
        auto result = changeset.ApplyChanges(m_dgndb, rebase, invert); // this actually updates the database with the changes
        if (result != BE_SQLITE_OK) {
            LOG.errorv("failed to apply changeset: %s", BeSQLiteLib::GetErrorName(result));
            BeAssert(false);
            m_dgndb.AbandonChanges();
            if (containsSchemaChanges)
                m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
            return result;
        }
    }

    if (action == TxnAction::Merge) {
        if (containsSchemaChanges) {
            // Note: All caches that hold ec-classes and handler-associations in memory have to be cleared.
            // The call to ClearECDbCache also clears all EC related caches held by DgnDb.
            // Additionally, we force merging of revisions containing schema changes to happen right when the
            // DgnDb is opened, and the Element caches haven't had a chance to get initialized.
            auto result = m_dgndb.AfterSchemaChangeSetApplied();
            if (result != BE_SQLITE_OK) {
                LOG.errorv("ApplyChanges failed schema changes: %s", BeSQLiteLib::GetErrorName(result));
                BeAssert(false);
                m_dgndb.AbandonChanges();
                m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
                return result;
            }
        }

        auto result = m_dgndb.AfterDataChangeSetApplied();
        if (result != BE_SQLITE_OK) {
            LOG.errorv("ApplyChanges failed data changes: %s", BeSQLiteLib::GetErrorName(result));
            BeAssert(false);
            m_dgndb.AbandonChanges();
            if (containsSchemaChanges)
                m_dgndb.Schemas().OnAfterSchemaChanges().RaiseEvent(m_dgndb, SchemaChangeType::SchemaChangesetApply);
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
    bool wasTracking = EnableTracking(false);

    Utf8String originalDDL = ddlChanges.ToString();
    Utf8String patchedDDL;
    DbResult result = BE_SQLITE_OK;
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
void TxnManager::ApplyTxnChanges(TxnId rowId, TxnAction action) {
    BeAssert(!HasDataChanges() && !InDynamicTxn());
    BeAssert(TxnAction::Reverse == action || TxnAction::Reinstate == action); // Do not call ApplyChanges() if you don't want undo/redo notifications sent to TxnMonitors...
    BeAssert(TxnType::Data == GetTxnType(rowId));

    UndoChangeSet changeset;
    ReadDataChanges(changeset, rowId, action);

    auto rc = ApplyChanges(changeset, action, false);
    BeAssert(!HasDataChanges());

    if (BE_SQLITE_OK != rc)
        return;

    // Mark this row as deleted/undeleted depending on which way we just applied the changes.
    CachedStatementPtr stmt = GetTxnStatement("UPDATE " DGN_TABLE_Txns " SET Deleted=? WHERE Id=?");
    stmt->BindInt(1, action == TxnAction::Reverse);
    stmt->BindInt64(2, rowId.GetValue());
    rc = stmt->Step();
    BeAssert(rc == BE_SQLITE_DONE);
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
    if (HasChanges() || InDynamicTxn())
        m_dgndb.AbandonChanges(); // will cancel dynamics if active

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
    if (m_dgndb.Revisions().IsCreatingRevision()) {
        BeAssert(false && "Cannot reverse transactions after starting a revision");
        return DgnDbStatus::IsCreatingRevision;
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
    DgnDbStatus status = ReverseTo(pos);
    DeleteReversedTxns(); // call this even if we didn't reverse anything - there may have already been reversed changes.
    return status;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseActions(TxnRange const& txnRange) {
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
    if (m_dgndb.Revisions().IsCreatingRevision()) {
        BeAssert(false); // Cannot reverse transactions after starting a revision. Call FinishCreateRevision() or AbandonCreateRevision() first");
        return DgnDbStatus::IsCreatingRevision;
    }

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
    if (m_dgndb.Revisions().IsCreatingRevision()) {
        BeAssert(false); // Cannot reverse transactions after starting a revision. Call FinishCreateRevision() or AbandonCreateRevision() first");
        return DgnDbStatus::IsCreatingRevision;
    }

    TxnId lastId = GetCurrentTxnId();
    TxnId startId = GetSessionStartId();

    if (startId >= lastId)
        return DgnDbStatus::NothingToUndo;

    OnBeforeUndoRedo(true);
    return ReverseActions(TxnRange(startId, GetCurrentTxnId()));
}

/*---------------------------------------------------------------------------------**/ /**
* Reinstate ("redo") a range of transactions.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReinstateTxn(TxnRange const& revTxn) {
    BeAssert(m_curr == revTxn.GetFirst());
    BeAssert(!m_reversedTxn.empty());

    if (HasChanges() || InDynamicTxn())
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
    ReinstateTxn(revTxn); // do the actual redo now.

    OnUndoRedo(TxnAction::Reinstate);
    return DgnDbStatus::Success;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReinstateTxn() {
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
    m_dgndb.SaveChanges(); // in case there are outstanding changes that will create a new Txn
    m_dgndb.ExecuteSql("DELETE FROM " DGN_TABLE_Txns);
    if (m_dgndb.TableExists(DGN_TABLE_Rebase))
        m_dgndb.ExecuteSql("DELETE FROM " DGN_TABLE_Rebase);
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
    Statement stmt(m_dgndb, "DELETE FROM " DGN_TABLE_Txns " WHERE Id < ?");
    stmt.BindInt64(1, lastId.GetValue());

    DbResult result = stmt.Step();
    UNUSED_VARIABLE(result);
    BeAssert(result == BE_SQLITE_DONE);
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
DynamicChangeTracker::OnCommitStatus DynamicChangeTracker::_OnCommit(bool isCommit, Utf8CP operation)
    {
    BeAssert(!isCommit && "You cannot save changes during a dynamic operation");
    if (isCommit)
        return OnCommitStatus::Abort;

    // TxnManager::_OnCommit() is going to pop us from its stack...let's be paranoid that deleting 'this' may have catastrophic effects
    DynamicChangeTrackerPtr justInCaseThisIsReferenced(this);
    m_txnMgr.CancelDynamics();
    return m_txnMgr._OnCommit(false, operation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DynamicChangeTracker::TrackChangesForTable DynamicChangeTracker::_FilterTable(Utf8CP tableName)
    {
    return m_txnMgr._FilterTable(tableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DynamicChangeTrackerPtr DynamicChangeTracker::Create(TxnManager& mgr)
    {
    return new DynamicChangeTracker(mgr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::BeginDynamicOperation()
    {
    auto tracker = DynamicChangeTracker::Create(*this);
    m_dynamicTxns.push_back(tracker);
    GetDgnDb().SetChangeTracker(tracker.get());
    tracker->EnableTracking(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::EndDynamicOperation(DynamicTxnProcessor* processor)
    {
    if (!InDynamicTxn())
        {
        BeAssert(false);
        return;
        }

    auto tracker = m_dynamicTxns.back();
    if (tracker->HasDataChanges())
        {
        UndoChangeSet changeset;
        auto rc = changeset.FromChangeTrack(*tracker);
        BeAssert(BE_SQLITE_OK == rc);
        UNUSED_VARIABLE(rc);

        OnBeginValidate();
        OnValidateChanges(changeset);

        if (nullptr != processor)
            {
            Restart();

            DoPropagateChanges(*tracker);

            if (tracker->HasDataChanges())
                {
                UndoChangeSet indirectChanges;
                indirectChanges.FromChangeTrack(*tracker);
                changeset.ConcatenateWith(indirectChanges);
                }

            processor->_ProcessDynamicChanges();
            }

        OnEndValidate();

        changeset.Invert();
        ApplyChanges(changeset, TxnAction::Abandon, false);
        }

    m_dynamicTxns.pop_back();
    if (InDynamicTxn())
        GetDgnDb().SetChangeTracker(m_dynamicTxns.back().get());
    else
        GetDgnDb().SetChangeTracker(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::CancelDynamics()
    {
    while (InDynamicTxn())
        EndDynamicOperation();
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
