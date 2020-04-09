/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_UNNAMED_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct ChangesBlobHeader
{
    enum {DB_Signature06 = 0x0600};
    uint32_t m_signature;    // write this so we can detect errors on read
    uint32_t m_size;

    ChangesBlobHeader(uint32_t size) {m_signature = DB_Signature06; m_size=size;}
    ChangesBlobHeader(SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::UndoChangeSet::ConflictResolution TxnManager::UndoChangeSet::_OnConflict(ConflictCause cause, BeSQLite::Changes::Change change)
    {
    Utf8CP tableName;
    int nCols,indirect;
    DbOpcode opcode;
    change.GetOperation(&tableName, &nCols, &opcode, &indirect);

    if (cause == ConflictCause::NotFound)
        {
        if (opcode == DbOpcode::Delete)      // a delete that is already gone.
            return ConflictResolution::Skip; // This is caused by propagate delete on a foreign key. It is not a problem.
        if (opcode == DbOpcode::Update)
            return ConflictResolution::Skip; // caused by inserting row and then updating it in the same txn and then undoing the txn. It's not a problem.
        }
    else if (ConflictCause::Data == cause)
        {
        if (DbOpcode::Delete == opcode)
            return ConflictResolution::Skip; // caused by inserting row and then updating it in the same txn and then undoing the txn. It's not a problem.
        }

    BeAssert(false);
    return ConflictResolution::Skip;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::CallJsMonitors(Utf8CP eventName, int* arg)
    {
    Napi::Object jsTxns = m_dgndb.GetJsTxns();
    if (jsTxns == nullptr)
        return;

    std::vector<napi_value> args;
    if (nullptr != arg)
        args.push_back(Napi::Number::New(jsTxns.Env(), *arg));

    m_dgndb.RaiseJsEvent(jsTxns, eventName, args);
    }

/*---------------------------------------------------------------------------------**//**
* We keep a statement cache just for TxnManager statements
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr TxnManager::GetTxnStatement(Utf8CP sql) const
    {
    CachedStatementPtr ptr;
    m_stmts.GetPreparedStatement(ptr, *m_dgndb.GetDbFile(), sql);
    return ptr;
    }

/*---------------------------------------------------------------------------------**//**
* Save an array of bytes representing a change for the current Txn into the DGN_TABLE_Txns table
* in the DgnDb. This also compresses the changes.
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::SaveChanges(IByteArrayCR changeBytes, Utf8CP operation, bool isSchemaChange) {
    if (0 == changeBytes.GetSize()) {
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }

    enum Column : int {Id=1,Deleted=2,Grouped=3,Operation=4,IsSchemaChange=5,Change=6};
    CachedStatementPtr stmt = GetTxnStatement("INSERT INTO " DGN_TABLE_Txns "(Id,Deleted,Grouped,Operation,IsSchemaChange,Change) VALUES(?,?,?,?,?,?)");

    stmt->BindInt64(Column::Id, m_curr.GetValue());
    stmt->BindInt(Column::Deleted,  false);
    if (nullptr != operation)
        stmt->BindText(Column::Operation, operation, Statement::MakeCopy::No);

    stmt->BindBoolean(Column::IsSchemaChange, isSchemaChange);

    // if we're in a multi-txn operation, and if the current TxnId is greater than the first txn, mark it as "grouped"
    stmt->BindInt(Column::Grouped, !m_multiTxnOp.empty() && (m_curr > m_multiTxnOp.back()));

    m_snappyTo.Init();
    ChangesBlobHeader header(changeBytes.GetSize());
    m_snappyTo.Write((Byte const*) &header, sizeof(header));
    m_snappyTo.Write((Byte const*) changeBytes.GetData(), changeBytes.GetSize());

    uint32_t zipSize = m_snappyTo.GetCompressedSize();
    if (0 < zipSize)
        {
        if (1 == m_snappyTo.GetCurrChunk())
            stmt->BindBlob(Column::Change, m_snappyTo.GetChunkData(0), zipSize, Statement::MakeCopy::No);
        else
            stmt->BindZeroBlob(Column::Change, zipSize); // more than one chunk
        }

    auto rc = stmt->Step();
    if (BE_SQLITE_DONE != rc)
        {
        BeAssert(false);
        return rc;
        }

    m_curr.Increment();

    if (1 == m_snappyTo.GetCurrChunk())
        return BE_SQLITE_DONE;

    StatusInt status = m_snappyTo.SaveToRow(m_dgndb, DGN_TABLE_Txns, "Change", m_dgndb.GetLastInsertRowId());
    if (SUCCESS != status)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/18
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
* @bsimethod                                    Sam.Wilson                      03/18
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
    while (BE_SQLITE_ROW == (rc = stmt.Step()))
        rebaser.AddRebase(stmt.GetValueBlob(0), stmt.GetColumnBytes(0));

    return BE_SQLITE_DONE == rc ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DeleteRebases(int64_t id) {
    if (!m_enableRebasers || id==0)
        return;

    Statement stmt(m_dgndb, "DELETE FROM " DGN_TABLE_Rebase " WHERE (Id <= ?)");
    stmt.BindInt64(1, id);
    auto result = stmt.Step();
    BeAssert(BE_SQLITE_DONE == result);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/18
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t TxnManager::QueryLastRebaseId() {
    if (!m_enableRebasers || !m_dgndb.TableExists(DGN_TABLE_Rebase))
        return 0;

    Statement stmt(m_dgndb, "SELECT MAX(Id) FROM " DGN_TABLE_Rebase);
    return BE_SQLITE_ROW == stmt.Step() ? stmt.GetValueInt64(0) : 0;
}

/*---------------------------------------------------------------------------------**//**
* Read the description of a Txn
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetTxnDescription(TxnId rowid) const {
    Statement stmt(m_dgndb, "SELECT Operation FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt.BindInt64(1, rowid.GetValue());
    return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueText(0) : "";
}

/*---------------------------------------------------------------------------------**//**
* Returns true if it's a transaction representing a schema change
* @bsimethod                                                  Ramanujam.Raman   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::IsSchemaChangeTxn(TxnId rowid) const {
    Statement stmt(m_dgndb, "SELECT IsSchemaChange FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt.BindInt64(1, rowid.GetValue());
    return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueBoolean(0) : false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::IsMultiTxnMember(TxnId rowid) const {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Grouped FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt->BindInt64(1, rowid.GetValue());
    return stmt->Step() != BE_SQLITE_ROW ? false : stmt->GetValueBoolean(0);
}

/*---------------------------------------------------------------------------------**//**
 return true if there are any (non-reversed) txns in the db
 @bsimethod                                    Keith.Bentley                    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::HasPendingTxns() const {
    Statement stmt(m_dgndb, "SELECT Id FROM " DGN_TABLE_Txns " WHERE Deleted=0");
    return stmt.Step() == BE_SQLITE_ROW;
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::Initialize() {
    m_action = TxnAction::None;

    Statement stmt(m_dgndb, "SELECT MAX(Id) FROM " DGN_TABLE_Txns " WHERE Deleted=0");
    DbResult result = stmt.Step();
    BeAssert(result == BE_SQLITE_ROW);

    TxnId last = stmt.GetValueInt64(0); // this is where we left off last session
    m_curr = TxnId(SessionId(last.GetSession().GetValue()+1), 0); // increment the session id, reset to index to 0.
    m_reversedTxn.clear();
    m_dynamicTxns.clear();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnManager(DgnDbR dgndb) : m_dgndb(dgndb), m_stmts(20), m_rlt(*this), m_initTableHandlers(false), m_enableNotifyTxnMonitors(true), m_modelChanges(*this)
    {
    m_dgndb.SetChangeTracker(this);
    m_enableRebasers = m_dgndb.TableExists(DGN_TABLE_Rebase);

    Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::InitializeTableHandlers()
    {
    if (!m_isTracking) {
        BeAssert(false && "Tracking must be enabled before initializing table handlers");
        return BE_SQLITE_ERROR;
    }

    if (m_initTableHandlers || m_dgndb.IsReadonly())
        return BE_SQLITE_OK;

    for (auto table : m_tables)
        table->_Initialize();

    DbResult result = m_dgndb.SaveChanges(); // "Commit" the creation of temp tables, so that a subsequent call to AbandonChanges will not un-create them.
    if (result != BE_SQLITE_OK) {
        BeAssert(false);
        return result;
    }

    m_initTableHandlers = true;
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::BeginTrackingRelationship(ECN::ECClassCR relClass)
    {
    if (m_dgndb.IsReadonly())
        return DgnDbStatus::ReadOnly;

    if (!relClass.IsRelationshipClass())
        return DgnDbStatus::BadArg;

    Utf8CP tableName;
    bool isTablePerHierarchy;
    if (SUCCESS != ChangeSummary::GetMappedPrimaryTable(tableName, isTablePerHierarchy, relClass, m_dgndb))
        return DgnDbStatus::BadArg;

    dgn_TxnTable::RelationshipLinkTable* rlt;

    auto handler = FindTxnTable(tableName);
    if (handler != nullptr)
        {
        // Somebody is already tracking this table
        rlt = dynamic_cast<dgn_TxnTable::RelationshipLinkTable*>(handler);
        if (nullptr == rlt)
            {
            //BeAssert(false && "relationship link table appears to be handled already. I can't handle it!");
            return DgnDbStatus::BadArg;
            }

        //  An RLT is tracking this table
        auto unirlt = dynamic_cast<dgn_TxnTable::UniqueRelationshipLinkTable*>(rlt);
        if (nullptr != unirlt)                  // If this table holds only one relationship, that means that
            {
            //BeAssert(unirlt->m_ecclass == &relClass);
            return DgnDbStatus::DuplicateName;  // this RLT must be tracking this relationship class
            }
        else
            {
            auto multi = static_cast<dgn_TxnTable::MultiRelationshipLinkTable*>(rlt);
            if (multi->m_ecclasses.find(&relClass) != multi->m_ecclasses.end())
                return DgnDbStatus::DuplicateName;  // this RLT is already tracking this relationship class
            multi->m_ecclasses.insert(&relClass);
            return DgnDbStatus::Success;            // Start tracking this relationship class
            }
        }

    // Nobody is tracking this table.
    // *** TBD:
    // if (isTablePerHierarchy)
    //  {
    //  auto multi = new dgn_TxnTable::MultiRelationshipLinkTable(*this);
    //  multi->m_ecclasses.insert(&relClass);
    //  handler = multi;
    //  }
    // else
        {
        auto uni = new dgn_TxnTable::UniqueRelationshipLinkTable(*this);
        uni->m_ecclass = &relClass;
        handler = uni;
        }

    m_tables.push_back(handler);
    m_tablesByName.Insert(tableName, handler);           // (takes ownership of handlers by adding a reference to it)

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::EndTrackingRelationship(ECN::ECClassCR relClass)
    {
    /*
    *** WIP_LINKTABLES
    */
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* add a new TxnTable to this TxnManager. TxnTables are responsible for reacting to changes to a given SQLite table, by name.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::AddTxnTable(DgnDomain::TableHandler* tableHandler)
    {
    if (m_dgndb.IsReadonly())
        return;

    // we can get called multiple times with the same tablehandler. Ignore all but the first one.
    TxnTablePtr table= tableHandler->_Create(*this);
    if (m_tablesByName.Insert(table->_GetTableName(), table).second)
        m_tables.push_back(table.get()); // order matters for the calling sequence, so we have to store it both sorted by name and insertion order
    }

/*---------------------------------------------------------------------------------**//**
* Find a TxnTable by name
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnTable* TxnManager::FindTxnTable(Utf8CP tableName) const
    {
    auto it = m_tablesByName.find(tableName);
    return it != m_tablesByName.end() ? it->second.get() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Get the TxnTable that handles changes to the dgn_Element table
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Element& TxnManager::Elements() const
    {
    return *(dgn_TxnTable::Element*) FindTxnTable(dgn_TxnTable::Element::MyTableName());
    }

/*---------------------------------------------------------------------------------**//**
* Get the TxnTable that handles changes to the dgn_ElementDrivesElement table
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::ElementDep& TxnManager::ElementDependencies() const
    {
    return *(dgn_TxnTable::ElementDep*) FindTxnTable(dgn_TxnTable::ElementDep::MyTableName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Model& TxnManager::Models() const
    {
    return *(dgn_TxnTable::Model*) FindTxnTable(dgn_TxnTable::Model::MyTableName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::QueryPreviousTxnId(TxnId curr) const
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Id FROM " DGN_TABLE_Txns " WHERE Id<? ORDER BY Id DESC LIMIT 1");
    stmt->BindInt64(1, curr.GetValue());

    auto rc = stmt->Step();
    return (rc != BE_SQLITE_ROW) ? TxnId() : TxnId(stmt->GetValueInt64(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::QueryNextTxnId(TxnId curr) const
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Id FROM " DGN_TABLE_Txns " WHERE Id>? ORDER BY Id LIMIT 1");
    stmt->BindInt64(1, curr.GetValue());

    auto rc = stmt->Step();
    return (rc != BE_SQLITE_ROW) ? TxnId() : TxnId(stmt->GetValueInt64(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::GetLastUndoableTxnId(AllowCrossSessions allowCrossSessions) const
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Id FROM " DGN_TABLE_Txns " WHERE Id<? AND Id>=? AND IsSchemaChange=1 ORDER BY Id DESC LIMIT 1");
    stmt->BindInt64(1, GetCurrentTxnId().GetValue());
    stmt->BindInt64(2, (allowCrossSessions == AllowCrossSessions::Yes) ? 0 : GetSessionStartId().GetValue());

    DbResult rc = stmt->Step();

    TxnId lastUndoableId;
    if (rc == BE_SQLITE_ROW)
        lastUndoableId = QueryNextTxnId(TxnId(stmt->GetValueInt64(0)));
    else
        {
        if (allowCrossSessions == AllowCrossSessions::No)
            return GetSessionStartId();

        lastUndoableId = QueryNextTxnId(TxnId(0));
        }

    return lastUndoableId.IsValid() ? lastUndoableId : GetCurrentTxnId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
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
* @bsimethod                                                    Keith.Bentley   03/04
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
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::GetMultiTxnOperationStart()
    {
    return m_multiTxnOp.empty() ? GetSessionStartId() : m_multiTxnOp.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TxnManager::DoPropagateChanges(ChangeTracker& tracker)
    {
    tracker.SetMode(Mode::Indirect);
    for (auto table :  m_tables)
        {
        table->_PropagateChanges();
        if (HasFatalError())
            break;
        }
    tracker.SetMode(Mode::Direct);

    return HasFatalError() ? BSIERROR : BSISUCCESS;
    }


#define TABLE_NAME_STARTS_WITH(NAME) (0==strncmp(NAME, tableName, sizeof(NAME)-1))
/*---------------------------------------------------------------------------------**//**
* When journalling changes, SQLite calls this method to determine whether changes to a specific table are eligible or not.
* @note tables with no primary key are skipped automatically.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TrackChangesForTable TxnManager::_FilterTable(Utf8CP tableName)
    {
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
* The supplied changeset represents all of the pending uncommited changes in the current transaction.
* Use the SQLite "ROLLBACK" statement to reverse all of those changes in the database, and then call the OnChangesApplied method
* on the inverted changeset to allow TxnTables to react to the fact that the changes were abandoned.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::OnCommitStatus TxnManager::CancelChanges(BeSQLite::IChangeSet& changeset)
    {
    DbResult rc = GetDgnDb().ExecuteSql("ROLLBACK");
    if (rc != BE_SQLITE_OK)
        return OnCommitStatus::Abort;

    if (!changeset.IsEmpty())
        {
        m_action = TxnAction::Abandon;
        OnChangesApplied(changeset, true);
        m_action = TxnAction::None;
        }

    return OnCommitStatus::Completed;
    }

/*---------------------------------------------------------------------------------**//**
* The supplied changeset was just applied to the database. That means the the database now potentially reflects a different
* state than the in-memory objects for the affected tables. Use the changeset to send _OnAppliedxxx events to the TxnTables for each changed row,
* so they can update in-memory state as necessary.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnChangesApplied(BeSQLite::IChangeSet& changeSet, bool invert)
    {
    Changes const& changes = changeSet.GetChanges(invert);

    Utf8String currTable;
    TxnTable* txnTable = 0;

    // Walk through each changed row in the changeset. They are ordered by table, so we know that all changes to one table will be seen
    // before we see any changes to another table.
    for (auto change : changes)
        {
        Utf8CP tableName;
        int nCols,indirect;
        DbOpcode opcode;

        DbResult rc = change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        BeAssert(rc==BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);

        if (0 != strcmp(currTable.c_str(), tableName)) // changes within a changeset are grouped by table
            {
            currTable = tableName;
            txnTable = FindTxnTable(tableName);
            }

        if (nullptr == txnTable)
            continue; // this table does not have a TxnTable for it, skip it

        switch (opcode)
            {
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
    }

/*---------------------------------------------------------------------------------**//**
* A changeset is about to be committed (or, in the case of "what if" testing, look like it is committed). Let each
* TxnTable prepare its state
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnBeginValidate()
    {
    m_fatalValidationError = false;
    m_txnErrors = 0;
    m_action = TxnAction::Commit;
    for (auto table : m_tables)
        table->_OnValidate();
    CallJsTxnManager("_onBeginValidate");
    }

/*---------------------------------------------------------------------------------**//**
* A changeset was just be committed. Let each TxnTable clean up its state. Also clear the validation errors.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnEndValidate()
    {
    CallJsTxnManager("_onEndValidate");
    for (auto table : m_tables)
        table->_OnValidated();

    m_fatalValidationError = false;
    m_txnErrors = 0;
    m_action = TxnAction::None;
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
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::OnCommitStatus TxnManager::_OnCommit(bool isCommit, Utf8CP operation) {
    BeAssert(!InDynamicTxn() && "How is this being invoked when we have dynamic change trackers on the stack?");
    CancelDynamics();

    DbSchemaChangeSet schemaChanges;
    if (HasDbSchemaChanges())
        schemaChanges = GetDbSchemaChanges();

    // Create a ChangeSet from modified data records. We'll use this to drive indirect changes.
    UndoChangeSet dataChangeSet;
    if (HasDataChanges()) {
        DbResult result = dataChangeSet.FromChangeTrack(*this); // Note: Changes to data in the change set may not make actual records in the Db.
        BeAssert(BE_SQLITE_OK == result);
    }

    if (dataChangeSet.IsEmpty() && schemaChanges.IsEmpty()) {
        // DbFile::StopSavepoint() used to check HasChanges() before invoking us here.
        // It no longer does, because we may have dynamic txns to revert even if TxnManager has no changes of its own
        // That's taken care of in the above call to CancelDynamics(), so we're finished
        if (HasDataChanges())
            Restart();
        if (GetDgnDb().BriefcaseManager().IsBulkOperation() && (RepositoryStatus::Success != GetDgnDb().BriefcaseManager().EndBulkOperation().Result()))
            return OnCommitStatus::Abort;

        return OnCommitStatus::Continue;
    }

    if (isCommit && m_dgndb.Revisions().HasReversedRevisions()) {
        BeAssert(false && "Cannot commit when revisions have been reversed. Abandon changes, reinstate revisions and try again");
        return OnCommitStatus::Abort;
    }

    Restart();  // Clear the change tracker since we have copied any changes to change sets

    if (!isCommit)
        return CancelChanges(dataChangeSet);

    // NOTE: you can't delete reversed Txns for CancelChanges because the database gets rolled back and they come back! That's OK,
    // just leave them reversed and they'll get thrown away on the next commit (or reinstated.)
    DeleteReversedTxns(); // these Txns are no longer reachable.

    if (!dataChangeSet.IsEmpty()) {
        OnBeginValidate();

        Changes changes(dataChangeSet, false);
        AddChanges(changes);

        BentleyStatus status = PropagateChanges();   // Propagate to generate indirect changes

        if (HasDataChanges()) {// did we have any indirect data changes captured in the tracker?
            UndoChangeSet indirectDataChangeSet;
            indirectDataChangeSet.FromChangeTrack(*this);
            Restart();
            Changes indirectChanges(indirectDataChangeSet, false);
            AddChanges(indirectChanges);
            dataChangeSet.ConcatenateWith(indirectDataChangeSet); // combine direct and indirect changes into a single dataChangeSet
        }

        if (GetDgnDb().BriefcaseManager().IsBulkOperation() && (RepositoryStatus::Success != GetDgnDb().BriefcaseManager().EndBulkOperation().Result()))
            status = BentleyStatus::ERROR;

        if (SUCCESS != status) {
            LOG.errorv("Cancelling txn due to fatal validation error.");
            OnEndValidate();
            return CancelChanges(dataChangeSet); // roll back entire txn
        }
    }

    if (!schemaChanges.IsEmpty()) {
        DbResult result = SaveSchemaChanges(schemaChanges, operation);
        if (result != BE_SQLITE_DONE)
            return OnCommitStatus::Abort;
    }

    if (!dataChangeSet.IsEmpty()) {
        DbResult result = SaveDataChanges(dataChangeSet, operation); // save changeSet into DgnDb itself, along with the description of the operation we're performing
        if (result != BE_SQLITE_DONE)
            return OnCommitStatus::Abort;
    }

    // At this point, all of the changes to all tables have been applied. Tell TxnMonitors
    if (m_enableNotifyTxnMonitors)
        T_HOST.GetTxnAdmin()._OnCommit(*this);

    if (!dataChangeSet.IsEmpty())
        OnEndValidate();

    m_dgndb.Revisions().UpdateInitialParentRevisionId(); // All new revisions are now based on the latest parent revision id

    return OnCommitStatus::Continue;
}

/*---------------------------------------------------------------------------------**//**
* called after the commit or cancel operation is complete
* @bsimethod                                    Keith.Bentley                   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::_OnCommitted(bool isCommit, Utf8CP) {
    if (isCommit && m_enableNotifyTxnMonitors) // only notify on commit, not cancel
        T_HOST.GetTxnAdmin()._OnCommitted(*this);
}

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                Ramanujam.Raman                    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionStatus TxnManager::MergeDbSchemaChangesInRevision(DgnRevisionCR revision, RevisionChangesFileReader& changeStream)
    {
    bool containsSchemaChanges;
    DbSchemaChangeSet dbSchemaChanges;
    DbResult result = changeStream.GetSchemaChanges(containsSchemaChanges, dbSchemaChanges);
    if (result != BE_SQLITE_OK)
        {
        BeAssert(false);
        return RevisionStatus::ApplyError;
        }

    if (dbSchemaChanges.IsEmpty())
        return RevisionStatus::Success;

    result = ApplyDbSchemaChangeSet(dbSchemaChanges);
    if (BE_SQLITE_OK != result)
        return RevisionStatus::ApplyError;

    return RevisionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                Ramanujam.Raman                    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionStatus TxnManager::MergeDataChangesInRevision(DgnRevisionCR revision, RevisionChangesFileReader& changeStream, bool containsSchemaChanges)
    {
    Rebase rebase;
    DbResult result = ApplyChanges(changeStream, TxnAction::Merge, containsSchemaChanges, &rebase);
    if (!EXPECTED_CONDITION(result == BE_SQLITE_OK && "Could not apply/merge data changes in revision"))
        {
        BeAssert(false);
        return RevisionStatus::ApplyError;
        }

    RevisionStatus status = RevisionStatus::Success;
    UndoChangeSet indirectChanges;
    if (HasDataChanges() || HasPendingTxns()) // has local data changes
        {
        /*
         * We propagate changes (run dependency rules) ONLY if there are local changes.
         * + The final state of the (incoming) revision is always setup to be exactly right. This
         *   is in turn because we cannot expect dependency handlers to be around on a server (or for
         *   that matter other briefcases), and the revisions offer the only mechanism to get the final
         *   state of the server exactly right. We generalize this behavior to be applicable to the server
         *   and local briefcases - if there are NO local changes, we simply accept the incoming revision's
         *   changes, and don't bother running the dependency handlers (even if the are actually available).
         *
         * Also see comments in RevisionChangesFileReader::ConflictHandler()
         */

        OnBeginValidate();

        Changes changes(changeStream, false);
        AddChanges(changes);

        if (SUCCESS != PropagateChanges())
            status = RevisionStatus::MergePropagationError;

        if (HasDataChanges())
            {
            indirectChanges.FromChangeTrack(*this);
            Restart();

            if (status == RevisionStatus::Success)
                {
                T_HOST.GetTxnAdmin()._OnCommit(*this); // At this point, all of the changes to all tables have been applied. Tell TxnMonitors

                Utf8String mergeComment = DgnCoreL10N::GetString(DgnCoreL10N::REVISION_Merged());
                if (!revision.GetSummary().empty())
                    {
                    mergeComment.append(": ");
                    mergeComment.append(revision.GetSummary());
                    }

                result = SaveDataChanges(indirectChanges, mergeComment.c_str());
                if (BE_SQLITE_DONE != result)
                    {
                    BeAssert(false);
                    status = RevisionStatus::SQLiteError;
                    }
                }
            }

        OnEndValidate();
        }

    if ((RevisionStatus::Success == status) && (0 != rebase.GetSize()))
        {
        int64_t rebaseId;
        result = SaveRebase(rebaseId, rebase);
        if (BE_SQLITE_DONE != result)
            {
            BeAssert(false);
            status = RevisionStatus::SQLiteError;
            }
        }

    if (status == RevisionStatus::Success)
        {
        status = m_dgndb.Revisions().SaveParentRevisionId(revision.GetId());

        if (status == RevisionStatus::Success)
            {
            result = m_dgndb.SaveChanges();
            // Note: All that the above operation does is to COMMIT the current Txn and BEGIN a new one.
            // The user should NOT be able to revert the revision id by a call to AbandonChanges() anymore, since
            // the merged changes are lost after this routine and cannot be used for change propagation anymore.
            if (BE_SQLITE_OK != result)
                {
                LOG.errorv("MergeRevision failed with SQLite error %s", m_dgndb.GetLastError().c_str());
                BeAssert(false);
                status = RevisionStatus::SQLiteError;
                }
            }
        }

    if (status != RevisionStatus::Success)
        {
        // Ensure the entire transaction is rolled back to before the merge, and the txn tables are notified to
        // appropriately revert their in-memory state.
        LOG.errorv("Could not propagate changes after merge due to validation errors.");

        // Note: CancelChanges() requires an iterator over the inverse of the changes notified through AddChanges().
        // The change stream can be inverted only by holding the inverse either in memory, or as a new file on disk.
        // Since this is an unlikely error condition, and it's just a single revision, we just hold the changes in memory
        // and not worry about the memory overhead.
        ChangeGroup undoChangeGroup;
        changeStream.ToChangeGroup(undoChangeGroup);
        if (indirectChanges.IsValid())
            {
            result = undoChangeGroup.AddChanges(indirectChanges.GetSize(), indirectChanges.GetData());
            BeAssert(result == BE_SQLITE_OK);
            }

        UndoChangeSet undoChangeSet;
        undoChangeSet.FromChangeGroup(undoChangeGroup);
        ChangeTracker::OnCommitStatus cancelStatus = CancelChanges(undoChangeSet); // roll back entire txn
        BeAssert(cancelStatus == ChangeTracker::OnCommitStatus::Completed);
        UNUSED_VARIABLE(cancelStatus);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                Ramanujam.Raman                    02/2017
 * @remarks Used only in revering or reinstating revisions.
 * @see TxnManager::MergeRevision
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionStatus TxnManager::ApplyRevision(DgnRevisionCR revision, bool reverse) {
    BeFileNameCR revisionChangesFile = revision.GetRevisionChangesFile();
    RevisionChangesFileReader changeStream(revisionChangesFile, m_dgndb);
    bool containsSchemaChanges = revision.ContainsSchemaChanges(m_dgndb);

    if (!containsSchemaChanges) {
        // Skip the entire schema change set when reversing or reinstating - DDL and the meta-data changes.
        // Reversing meta data changes cause conflicts - see TFS#149046
        DbResult result = ApplyChanges(changeStream, reverse ? TxnAction::Reverse : TxnAction::Reinstate, false, nullptr, reverse);
        if (result != BE_SQLITE_OK)
            return RevisionStatus::ApplyError;
    }

    RevisionStatus status;
    RevisionManagerR revMgr = m_dgndb.Revisions();
    if (reverse)
        status = revMgr.SaveReversedRevisionId(revision.GetParentId());
    else
        status = (revision.GetId() == revMgr.GetParentRevisionId()) ? revMgr.DeleteReversedRevisionId() : revMgr.SaveReversedRevisionId(revision.GetId());

    if (status == RevisionStatus::Success) {
        DbResult result = m_dgndb.SaveChanges();
        if (BE_SQLITE_OK != result) {
            LOG.errorv("Apply failed with SQLite error %s", m_dgndb.GetLastError().c_str());
            BeAssert(false);
            status = RevisionStatus::SQLiteError;
        }
    }

    if (status != RevisionStatus::Success) {
        BeAssert(!containsSchemaChanges && "Never attempt to reverse or reinstate schema changes");
        // Ensure the entire transaction is rolled back to before the merge, and the txn tables are notified to
        // appropriately revert their in-memory state.
        ChangeTracker::OnCommitStatus cancelStatus = CancelChanges(changeStream);
        BeAssert(cancelStatus == ChangeTracker::OnCommitStatus::Completed);
        UNUSED_VARIABLE(cancelStatus);
    }

    return status;
}

/*---------------------------------------------------------------------------------**//**
 * Merge changes from a changeStream that originated in an external repository
 * @bsimethod                                Ramanujam.Raman                    10/2015
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
        // Note: Schema changes may not necessary imply db-schema changes. They could just be 'minor' ecschema/mapping changes.
        status = MergeDbSchemaChangesInRevision(revision, changeStream);
        if (RevisionStatus::Success != status)
            return status;
        }

    return MergeDataChangesInRevision(revision, changeStream, containsSchemaChanges);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReportError(bool fatal, Utf8CP errorType, Utf8CP msg)  {
    auto jsTxns = m_dgndb.GetJsTxns();
    if (nullptr == jsTxns) {
        m_fatalValidationError |= fatal;
        return;
    }

    // Note: see IModelDb.ts [[ValidationError]]
    auto env = jsTxns.Env();
    auto error = Napi::Object::New(env);
    error["fatal"] = Napi::Boolean::New(env, fatal);
    error["errorType"] = Napi::String::New(env, errorType);
    error["message"] = Napi::String::New(env, msg);
    DgnDb::CallJsFunction(jsTxns, "reportError", {error});
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/19
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ModelChanges::Apply() {
    if (m_enabled) {
        m_mgr.SetMode(BeSQLite::ChangeTracker::Mode::Indirect);
        DoApply();
        m_mgr.SetMode(BeSQLite::ChangeTracker::Mode::Direct);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/19
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ModelChanges::DoApply()
    {
    // When we get a Change that deletes a geometric element, we don't have access to its model Id at that time - look it up now.
    for (auto elemId : m_deletedGeometricElements)
        {
        auto iter = m_modelsForDeletedElements.find(elemId);
        if (m_modelsForDeletedElements.end() != iter)
            m_geometricModels.insert(iter->second);
        }

    m_deletedGeometricElements.clear();
    m_modelsForDeletedElements.clear();

    if (m_models.empty() && m_geometricModels.empty())
        return;

    m_mgr.SetMode(Mode::Indirect);
    auto disable = [&]()
        {
        m_enabled = false; // this iModel hasn't been upgraded. Skip this function from now on.
        m_mgr.SetMode(Mode::Direct);
        Clear();
        };

    // if there were any geometric changes, update the "GeometryGuid" and "LastMod" properties in the Model table.
    if (!m_geometricModels.empty())
        {
        auto stmt = m_mgr.GetDgnDb().GetGeometricModelUpdateStatement();
        if (!stmt.IsValid())
            {
            disable();
            return;
            }

        BeGuid guid(true); // create a new GUID to represent this state of the changed geometric models
        for (auto model : m_geometricModels)
            {
            m_models.erase(model); // we don't need to update the LastMod property below, since this statement updates it
            stmt->BindGuid(1, guid);
            stmt->BindId(2, model);
            DbResult rc = stmt->Step();
            UNUSED_VARIABLE(rc);
            BeAssert(BE_SQLITE_DONE == rc);
            stmt->Reset();
            }
        }

    if (!m_models.empty())
        {
        auto stmt = m_mgr.GetDgnDb().GetModelLastModUpdateStatement();
        if (!stmt.IsValid())
            {
            disable();
            return;
            }

        for (auto model : m_models)
            {
            stmt->BindId(1, model);
            DbResult rc = stmt->Step();
            UNUSED_VARIABLE(rc);
            BeAssert(BE_SQLITE_DONE == rc);
            stmt->Reset();
            }
        }

    Clear();
    m_mgr.SetMode(Mode::Direct);
    }

/*---------------------------------------------------------------------------------**//**
* Add all changes to the TxnSummary. TxnTables store information about the changes in their own state
* if they need to hold on to them so they can react after the changeset is applied.
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::AddChanges(Changes const& changes)
    {
    BeAssert(!m_dgndb.IsReadonly());
    TxnTable* txnTable = 0;

    // Walk through each changed row in the changeset. They are ordered by table, so we know that all changes to one table will be seen
    // before we see any changes to another table.
    for (auto change : changes)
        {
        Utf8CP tableName;
        int nCols,indirect;
        DbOpcode opcode;
        Utf8String currTable;

        DbResult rc = change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        if (rc != BE_SQLITE_OK)
            {
            BeAssert(false && "invalid changeset");
            continue;
            }

        if (0 != strcmp(currTable.c_str(), tableName))
            { // changes within a changeset are grouped by table
            currTable = tableName;
            txnTable = FindTxnTable(tableName);
            }

        if (nullptr == txnTable)
            continue; // this table does not have a TxnTable for it, skip it

        switch (opcode)
            {
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

    m_modelChanges.Apply();
    }

/*---------------------------------------------------------------------------------**//**
* Apply a changeset to the database. Notify all TxnTables about what's in the Changeset, both before
* and after it is applied.
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ApplyChanges(IChangeSet& changeset, TxnAction action, bool containsSchemaChanges, Rebase* rebase, bool invert)
    {
    BeAssert(action != TxnAction::None);
    m_action = action;

    // if we're not in interactive mode, we won't keep these caches up to date, just clear them
    if (!m_isInteractive) {
        m_dgndb.Elements().ClearCache();
        m_dgndb.Models().ClearCache();
    }

    if (!IsInAbandon())
        OnBeginApplyChanges();

    bool wasTracking = EnableTracking(false);
    DbResult result = changeset.ApplyChanges(m_dgndb, rebase, invert); // this actually updates the database with the changes
    if (result != BE_SQLITE_OK)
        m_dgndb.AbandonChanges();
    EnableTracking(wasTracking);
    BeAssert(result == BE_SQLITE_OK);

    if (action == TxnAction::Merge && result == BE_SQLITE_OK) {
        if (containsSchemaChanges) {
            /* Note: All caches that hold ec-classes and handler-associations in memory have to be cleared.
            * The call to ClearECDbCache also clears all EC related caches held by DgnDb.
            * Additionally, we force merging of revisions containing schema changes to happen right when the
            * DgnDb is opened, and the Element caches haven't had a chance to get initialized.
            */
            result = m_dgndb.AfterSchemaChangeSetApplied();
            if (result != BE_SQLITE_OK) {
                BeAssert(false);
                m_dgndb.AbandonChanges();
                return result;
            }
        }

        result = m_dgndb.AfterDataChangeSetApplied();
        if (result != BE_SQLITE_OK) {
            BeAssert(false);
            m_dgndb.AbandonChanges();
            return result;
        }
    }

    if (result == BE_SQLITE_OK) {
        if (m_isInteractive)
            OnChangesApplied(changeset, invert);

        if (!IsInAbandon())
            T_HOST.GetTxnAdmin()._OnAppliedChanges(*this);
    }

    if (m_isInteractive && !IsInAbandon())
        OnEndApplyChanges();

    m_action = TxnAction::None;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Affan.Khan        04/20
* This remove unchanged indexes from DDL. There is already a fix in to remove it from where
* these unchanged indexes get added on ECDb side. But following fixes issue with existing
* changesets that is already on imodelhub or old bridges that is not consuming the source fix.
* In case of any error this function return original unchanged DDL.
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TxnManager::PatchSlowSchemaChangeSet(Utf8StringR patchedDDL, Utf8StringCR compoundSQL)
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
* @bsimethod                                                  Ramanujam.Raman   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ApplyDbSchemaChangeSet(DbSchemaChangeSetCR dbSchemaChanges)
    {
    BeAssert(!dbSchemaChanges.IsEmpty() && "DbSchemaChangeSet is empty");
    bool wasTracking = EnableTracking(false);

    Utf8String originalDDL = dbSchemaChanges.ToString();
    Utf8String patchedDDL;
    DbResult result = BE_SQLITE_OK;
    BentleyStatus status = PatchSlowSchemaChangeSet(patchedDDL, originalDDL);
    if (status == SUCCESS)
        {
        // Info message so we can look out if this issue has gone due to fix in the place which produce these changeset.
        LOG.info("[PATCH] Appling DDL patch for #292801 #281557");
        result = m_dgndb.ExecuteSql(patchedDDL.c_str());
        if (result != BE_SQLITE_OK)
            {
            LOG.info("[PATCH] Failed to apply patch for #292801 #281557. Fallback to original DDL");
            result = m_dgndb.ExecuteSql(originalDDL.c_str());
            }
        }
    else
        {
        result = m_dgndb.ExecuteSql(originalDDL.c_str());
        }

    EnableTracking(wasTracking);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReadDbSchemaChanges(BeSQLite::DbSchemaChangeSet& dbSchemaChanges, TxnId rowId)
    {
    uint32_t sizeRead;
    Byte* data = ReadChanges(sizeRead, rowId);
    if (data == nullptr)
        return;

    dbSchemaChanges.AddDDL((Utf8CP) data);
    delete[] data;
    }

/*---------------------------------------------------------------------------------**//**
* Changesets are stored as compressed blobs in the DGN_TABLE_Txns table. Read one by rowid.
* If the TxnDirection is backwards, invert the changeset.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReadDataChanges(ChangeSet& dataChangeSet, TxnId rowId, TxnAction action)
    {
    uint32_t sizeRead;
    Byte* data = ReadChanges(sizeRead, rowId);
    if (data == nullptr)
        return;

    dataChangeSet.FromData(sizeRead, data, action == TxnAction::Reverse);
    delete[] data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Byte* TxnManager::ReadChanges(uint32_t& sizeRead, TxnId rowId)
    {
    if (ZIP_SUCCESS != m_snappyFrom.Init(m_dgndb, DGN_TABLE_Txns, "Change", rowId.GetValue()))
        {
        BeAssert(false);
        return nullptr;
        }

    ChangesBlobHeader header(m_snappyFrom);
    if ((ChangesBlobHeader::DB_Signature06 != header.m_signature) || 0 == header.m_size)
        {
        BeAssert(false);
        return nullptr;
        }

    Byte* data = new Byte[header.m_size];

    m_snappyFrom.ReadAndFinish(data, header.m_size, sizeRead);

    if (sizeRead != header.m_size)
        {
        delete[] data;
        BeAssert(false);
        return nullptr;
        }

    sizeRead = header.m_size;
    return data;
    }

/*---------------------------------------------------------------------------------**//**
* Read a changeset from the dgn_Txn table, potentially inverting it (depending on whether we're performing undo or redo),
* and then apply the changeset to the DgnDb.
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ApplyTxnChanges(TxnId rowId, TxnAction action)
    {
    BeAssert(!HasDataChanges() && !InDynamicTxn());
    BeAssert(TxnAction::Reverse == action || TxnAction::Reinstate == action); // Do not call ApplyChanges() if you don't want undo/redo notifications sent to TxnMonitors...
    BeAssert(!IsSchemaChangeTxn(rowId));

    UndoChangeSet changeset;
    ReadDataChanges(changeset, rowId, action);

    auto rc = ApplyChanges(changeset, action, false);
    BeAssert(!HasDataChanges());

    if (BE_SQLITE_OK != rc)
        return;

    // Mark this row as deleted/undeleted depending on which way we just applied the changes.
    CachedStatementPtr stmt = GetTxnStatement("UPDATE " DGN_TABLE_Txns " SET Deleted=? WHERE Id=?");
    stmt->BindInt(1, action==TxnAction::Reverse);
    stmt->BindInt64(2, rowId.GetValue());
    rc = stmt->Step();
    BeAssert(rc==BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnBeginApplyChanges() {
    for (auto table : m_tables)
        table->_OnApply();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnEndApplyChanges() {
    for (auto table : m_tables)
        table->_OnApplied();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReverseTxnRange(TxnRange const& txnRange)
    {
    if (HasChanges() || InDynamicTxn())
        m_dgndb.AbandonChanges(); // will cancel dynamics if active

    for (TxnId curr=QueryPreviousTxnId(txnRange.GetLast()); curr.IsValid() && curr >= txnRange.GetFirst(); curr=QueryPreviousTxnId(curr))
        ApplyTxnChanges(curr, TxnAction::Reverse);

    BeAssert(!HasChanges());

    m_dgndb.SaveChanges(); // make sure we save the updated Txn data to disk.

    m_curr = txnRange.GetFirst(); // we reuse TxnIds

    // save in reversed Txns list
    m_reversedTxn.push_back(txnRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseTo(TxnId pos, AllowCrossSessions allowCrossSessions)
    {
    if (m_dgndb.Revisions().IsCreatingRevision())
        {
        BeAssert(false && "Cannot reverse transactions after starting a revision. Call FinishCreateRevision() or AbandonCreateRevision() first");
        return DgnDbStatus::IsCreatingRevision;
        }

    TxnId lastId = GetCurrentTxnId();
    if (!pos.IsValid() || pos >= lastId)
        return DgnDbStatus::NothingToUndo;

    TxnId lastUndoableId = GetLastUndoableTxnId(allowCrossSessions);
    if (lastUndoableId >= lastId || pos < lastUndoableId)
        return DgnDbStatus::CannotUndo;

    T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo(*this);
    return ReverseActions(TxnRange(pos, lastId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::CancelTo(TxnId pos, AllowCrossSessions allowCrossSessions)
    {
    DgnDbStatus status = ReverseTo(pos, allowCrossSessions);
    DeleteReversedTxns(); // call this even if we didn't reverse anything - there may have already been reversed changes.
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseActions(TxnRange const& txnRange)
    {
    Utf8String undoStr;
    ReverseTxnRange(txnRange);     // do the actual undo now.

    while (GetCurrentTxnId() < GetMultiTxnOperationStart())
        EndMultiTxnOperation();

    T_HOST.GetTxnAdmin()._OnUndoRedo(*this, TxnAction::Reverse);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseTxns(int numActions, AllowCrossSessions allowCrossSessions)
    {
    if (m_dgndb.Revisions().IsCreatingRevision())
        {
        BeAssert(false);// Cannot reverse transactions after starting a revision. Call FinishCreateRevision() or AbandonCreateRevision() first");
        return DgnDbStatus::IsCreatingRevision;
        }

    TxnId lastId = GetCurrentTxnId();
    TxnId lastUndoableId = GetLastUndoableTxnId(allowCrossSessions);

    TxnId firstId = lastId;
    while (numActions > 0 && firstId > lastUndoableId)
        {
        TxnId prevId = QueryPreviousTxnId(firstId);
        if (!prevId.IsValid())
            break;

        if (!IsMultiTxnMember(prevId))
            --numActions;

        firstId = prevId;
        }

    if (firstId == lastId)
        return DgnDbStatus::NothingToUndo;

    T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo(*this);
    return ReverseActions(TxnRange(firstId, lastId));
    }

/*---------------------------------------------------------------------------------**//**
* reverse (undo) all previous transactions
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseAll()
    {
    if (m_dgndb.Revisions().IsCreatingRevision())
        {
        BeAssert(false);// Cannot reverse transactions after starting a revision. Call FinishCreateRevision() or AbandonCreateRevision() first");
        return DgnDbStatus::IsCreatingRevision;
        }

    TxnId lastId = GetCurrentTxnId();
    TxnId lastUndoableId = GetLastUndoableTxnId(AllowCrossSessions::No);

    if (lastUndoableId >= lastId)
        return DgnDbStatus::NothingToUndo;

    T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo(*this);

    return ReverseActions(TxnRange(lastUndoableId, GetCurrentTxnId()));
    }

/*---------------------------------------------------------------------------------**/ /**
* Reinstate ("redo") a range of transactions.
* @bsimethod                                                    Keith.Bentley   02/04
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
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReinstateActions(TxnRange const& revTxn) {
    ReinstateTxn(revTxn); // do the actual redo now.

    T_HOST.GetTxnAdmin()._OnUndoRedo(*this, TxnAction::Reinstate);
    return DgnDbStatus::Success;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReinstateTxn() {
    if (!IsRedoPossible())
        return DgnDbStatus::NothingToRedo;

    T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo(*this);
    return ReinstateActions(m_reversedTxn.back());
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetUndoString(AllowCrossSessions crossSessions) {
    return IsUndoPossible(crossSessions) ? GetTxnDescription(QueryPreviousTxnId(GetCurrentTxnId())) : "";
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetRedoString() {
    return IsRedoPossible() ? GetTxnDescription(m_reversedTxn.back().GetFirst()) : "";
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DeleteAllTxns() {
    m_dgndb.ExecuteSql("DELETE FROM " DGN_TABLE_Txns);
    if (m_dgndb.TableExists(DGN_TABLE_Rebase))
        m_dgndb.ExecuteSql("DELETE FROM " DGN_TABLE_Rebase);
    Initialize();
}

/*---------------------------------------------------------------------------------**//**
* Cancel any undone (reversed) transactions.
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DeleteReversedTxns() {
    m_reversedTxn.clear(); // do this even if this is already empty - there may be reversed txns from a previous session
    m_dgndb.ExecuteSql("DELETE FROM " DGN_TABLE_Txns " WHERE Deleted=1"); // these transactions are no longer reinstateable. Throw them away.
}

/*---------------------------------------------------------------------------------**//**
* Delete transactions from the start of the table to (but not including) the specified transaction.
* @bsimethod                                                  Ramanujam.Raman   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DeleteFromStartTo(TxnId lastId) {
    Statement stmt(m_dgndb, "DELETE FROM " DGN_TABLE_Txns " WHERE Id < ?");
    stmt.BindInt64(1, lastId.GetValue());

    DbResult result = stmt.Step();
    BeAssert(result == BE_SQLITE_DONE);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_Initialize()
    {
    m_txnMgr.GetDgnDb().CreateTable(TEMP_TABLE(TXN_TABLE_Elements), "ElementId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT,ECClassId INTEGER NOT NULL");
    m_txnMgr.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Elements) "_Midx ON " TXN_TABLE_Elements "(ModelId)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::CreateIndexOnECClassId()
    {
    if (m_haveIndexOnECClassId)
        return;

    m_haveIndexOnECClassId = true;
    m_txnMgr.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Elements) "_Cidx ON " TXN_TABLE_Elements "(ECClassId)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnValidate()
    {
    m_changes = false;
    if (m_stmt.IsPrepared())
        return;

    m_stmt.Prepare(m_txnMgr.GetDgnDb(), "INSERT INTO " TEMP_TABLE(TXN_TABLE_Elements) "(ElementId,ModelId,ChangeType,ECClassId) VALUES(?,?,?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnValidated()
    {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes)
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Elements));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_Initialize()
    {
    m_txnMgr.GetDgnDb().CreateTable(TEMP_TABLE(TXN_TABLE_Models), "ModelId INTEGER NOT NULL PRIMARY KEY,ChangeType INT,ECClassId INTEGER NOT NULL");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnValidate()
    {
    m_changes = false;
    if (m_stmt.IsPrepared())
        return;

    m_stmt.Prepare(m_txnMgr.GetDgnDb(), "INSERT INTO " TEMP_TABLE(TXN_TABLE_Models) "(ModelId,ChangeType,ECClassId) VALUES(?,?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnValidated()
    {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes)
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Models));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2015
//---------------------------------------------------------------------------------------
void dgn_TxnTable::Model::AddChange(Changes::Change const& change, ChangeType changeType)
    {
    Changes::Change::Stage stage;
    switch (changeType)
        {
        case ChangeType::Insert:
            stage = Changes::Change::Stage::New;
            break;

        case ChangeType::Delete:
            stage = Changes::Change::Stage::Old;
            break;

        case ChangeType::Update:
            return; // do nothing for update

        default:
            BeAssert(false);
            return;
        }

    DgnModelId modelId = DgnModelId(change.GetValue(0, stage).GetValueUInt64());
    BeAssert(modelId.IsValid());

    DgnClassId classId;
    if (ChangeType::Update == changeType)
        {
        // for updates, the model table must be queried for ECClassId since the change set will only contain changed columns
        CachedStatementPtr stmt = m_txnMgr.GetDgnDb().Elements().GetStatement("SELECT ECClassId FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id=?");
        stmt->BindId(1, modelId);
        if (BE_SQLITE_ROW != stmt->Step())
            {
            BeAssert(false);
            return;
            }
        classId = stmt->GetValueId<DgnClassId>(0);
        }
    else
        {
        classId = DgnClassId(change.GetValue((int) DgnModel::ColumnNumbers::ECClassId, stage).GetValueUInt64());
        }

    enum Column : int {ModelId=1, ChangeType=2, ECClassId=3};

    m_changes = true;
    m_stmt.BindId(Column::ModelId, modelId);
    m_stmt.BindInt(Column::ChangeType, (int) changeType);
    m_stmt.BindId(Column::ECClassId, classId);

    auto rc = m_stmt.Step();
    BeAssert(rc==BE_SQLITE_DONE);
    UNUSED_VARIABLE(rc);

    m_stmt.Reset();
    m_stmt.ClearBindings();
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                Ramanujam.Raman                    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnApply()
    {
    if (!m_txnMgr.IsInAbandon())
        _OnValidate();
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                Ramanujam.Raman                    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnApplied()
    {
    if (!m_txnMgr.IsInAbandon())
        _OnValidated();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnAppliedDelete(BeSQLite::Changes::Change const& change)
    {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Delete);

    DgnModelId modelId = change.GetOldValue(0).GetValueId<DgnModelId>();
    DgnModelPtr model = m_txnMgr.GetDgnDb().Models().FindModel(modelId);
    if (!model.IsValid())
        return;

    T_HOST.GetTxnAdmin()._OnAppliedModelDelete(*model);

    m_txnMgr.GetDgnDb().Models().DropLoadedModel(*model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnAppliedUpdate(BeSQLite::Changes::Change const& change)
    {
    DgnModelId modelId = change.GetOldValue(0).GetValueId<DgnModelId>();
    DgnModelPtr model = m_txnMgr.GetDgnDb().Models().FindModel(modelId);
    if (!model.IsValid())
        return;

    model->Read(modelId);
    model->_OnUpdated();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnAppliedAdd(BeSQLite::Changes::Change const& change)
    {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Insert);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_Initialize()
    {
    m_txnMgr.GetDgnDb().CreateTable(TEMP_TABLE(TXN_TABLE_Depend), "ECInstanceId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT");
    m_txnMgr.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Depend) "_Midx ON " TXN_TABLE_Depend "(ModelId)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_OnValidate()
    {
    m_changes = false;
    if (m_stmt.IsPrepared())
        return;

    m_stmt.Prepare(m_txnMgr.GetDgnDb(), "INSERT INTO " TEMP_TABLE(TXN_TABLE_Depend) " (ECInstanceId,ModelId,ChangeType) VALUES(?,?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_OnValidated()
    {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes && TxnAction::Abandon != m_txnMgr.GetCurrentAction())
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Depend));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
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
        m_deletedRels.push_back(DepRelData(relkey, DgnElementId((uint64_t)srcelemid), DgnElementId((uint64_t)tgtelemid)));
        }
    else
        {
        Changes::Change::Stage stage = (ChangeType::Insert == changeType) ? Changes::Change::Stage::New : Changes::Change::Stage::Old;
        ECInstanceId instanceId = change.GetValue(0, stage).GetValueId<ECInstanceId>(); // primary key is column 0
        AddDependency(instanceId, changeType);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
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
    if (fromCommit)
        {
        m_txnMgr.m_modelChanges.AddModel(modelId); // add to set of changed models.
        if (ChangeType::Delete == changeType)
            m_txnMgr.m_modelChanges.AddDeletedElement(elementId, modelId); // Record model Id in case it's a geometric element.
        }
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    08/19
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
// @bsimethod                                   Shaun.Sewall                    02/2015
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

    DgnElementId elementId = DgnElementId(change.GetValue(0, stage).GetValueUInt64());
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
 @bsimethod                                    Keith.Bentley                    08/19
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
 @bsimethod                                    Keith.Bentley                    08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Geometric::AddChange(BeSQLite::Changes::Change const& change, ChangeType changeType)
    {
    if (ChangeType::Update == changeType && !HasChangeInColumns(change))
        return; // no geometric changes

    auto stage = ChangeType::Insert == changeType ? Changes::Change::Stage::New : Changes::Change::Stage::Old;
    DgnElementId elementId = change.GetValue(0, stage).GetValueId<DgnElementId>();

    if (ChangeType::Delete == changeType)
        {
        // We don't have access to the model Id here. Rely on the Element txn table to record it for later use.
        m_txnMgr.m_modelChanges.AddDeletedGeometricElement(elementId);
        return;
        }

    DgnClassId classId;
    m_txnMgr.m_modelChanges.AddGeometricModel(GetModelAndClass(classId, elementId)); // mark this model as having geometric changes
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
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
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::AddDependency(EC::ECInstanceId const& relid, ChangeType changeType) {
    CachedECSqlStatementPtr stmt = m_txnMgr.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT element.Model.Id FROM " BIS_SCHEMA(BIS_CLASS_Element) " AS element, " BIS_SCHEMA(BIS_REL_ElementDrivesElement) " AS DEP"
        " WHERE (DEP.ECInstanceId=?) AND (element.ECInstanceId=DEP.SourceECInstanceId)");
    stmt->BindId(1, relid);
    auto stat = stmt->Step();
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
* @bsimethod                                    Sam.Wilson                   04/2016
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
* @bsimethod                                    Sam.Wilson                   04/2016
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
* @bsimethod                                    Sam.Wilson                   04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult dgn_TxnTable::RelationshipLinkTable::QueryTargets(DgnElementId& srcelemid, DgnElementId& tgtelemid, BeSQLite::EC::ECInstanceId relid, ECN::ECClassCR relClass)
    {
    //  SourceECInstanceId and TargetECInstanceId never change.
    auto selectOrig = m_txnMgr.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM %s WHERE ECInstanceId=?", relClass.GetECSqlName().c_str()).c_str());
    selectOrig->BindId(1, relid);
    auto rc = selectOrig->Step();
    if (BE_SQLITE_ROW != rc)
        return rc;

    srcelemid = selectOrig->GetValueId<DgnElementId>(0);
    tgtelemid = selectOrig->GetValueId<DgnElementId>(1);
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                   04/2016
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
        QueryTargets(srcelemid, tgtelemid, relid, *m_ecclass); //  SourceECInstanceId and TargetECInstanceId never change.
        }

    m_txnMgr.GetRelationshipLinkTables().Insert(relid, relclsid, srcelemid, tgtelemid, changeType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                   04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::MultiRelationshipLinkTable::_UpdateSummary(Changes::Change change, ChangeType changeType)
    {
    m_changes = true;

    Changes::Change::Stage stage = (ChangeType::Insert == changeType) ? Changes::Change::Stage::New : Changes::Change::Stage::Old;

    //  Every table-per-hierarchy relationship link table is laid out like this:
    //      [0]ECInstanceId
    //      [1]ECClassId
    //      [2]SourceECInstanceId
    //      [3]TargetECInstanceId
    //      [4...] relationship instance properties ...     which we DO NOT TRACK
    int const ECInstanceId_LTColId = 0;
    int const ECClassId_LTColId = 1;
    int const SourceECInstanceId_LTColId = 2;
    int const TargetECInstanceId_LTColId = 3;

    ECClassId relclsid = change.GetValue(ECClassId_LTColId, stage).GetValueId<ECClassId>();
    ECN::ECClassCP relcls = m_txnMgr.GetDgnDb().Schemas().GetClass(relclsid);

    if (m_ecclasses.find(relcls) == m_ecclasses.end())  // while this class (among others) is mapped into this table,
         return;                                         // I am not actually tracking this class.

    ECInstanceId relid = change.GetValue(ECInstanceId_LTColId, stage).GetValueId<ECInstanceId>();

    DgnElementId srcelemid, tgtelemid;
    if (ChangeType::Insert == changeType || ChangeType::Delete == changeType)
        {
        srcelemid = change.GetValue(SourceECInstanceId_LTColId, stage).GetValueId<DgnElementId>();
        tgtelemid = change.GetValue(TargetECInstanceId_LTColId, stage).GetValueId<DgnElementId>();
        }
    else
        {
        QueryTargets(srcelemid, tgtelemid, relid, *relcls); //  SourceECInstanceId and TargetECInstanceId never change.
        }

    m_txnMgr.GetRelationshipLinkTables().Insert(relid, relclsid, srcelemid, tgtelemid, changeType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                   04/2016
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
* @bsimethod                                    Keith.Bentley                   12/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::AddTxnMonitor(TxnMonitor& monitor)
    {
    m_monitors.push_back(&monitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::DropTxnMonitor(TxnMonitor& monitor)
    {
    auto it = std::find(m_monitors.begin(), m_monitors.end(), &monitor);
    if (it != m_monitors.end())
        *it = nullptr; // removed from list by CallMonitors
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::CallMonitors(std::function<void(TxnMonitor&)> caller)
    {
    for (auto curr = m_monitors.begin(); curr!=m_monitors.end(); )
        {
        if (*curr == nullptr)
            curr = m_monitors.erase(curr);
        else
            {
            try {caller(**curr); ++curr;}
            catch(...) {}
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnCommit(TxnManagerR mgr)
    {
    mgr.CallJsMonitors("onCommit");
    CallMonitors([&mgr](TxnMonitor& monitor){monitor._OnCommit(mgr);});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnCommitted(TxnManagerR mgr)
    {
    mgr.CallJsMonitors("onCommitted");
    CallMonitors([&mgr](TxnMonitor& monitor){monitor._OnCommitted(mgr);});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnAppliedChanges(TxnManagerR mgr)
    {
    mgr.CallJsMonitors("onChangesApplied");
    CallMonitors([&mgr](TxnMonitor& monitor){monitor._OnAppliedChanges(mgr);});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Zukauskas               11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnPrepareForUndoRedo(TxnManagerR mgr)
    {
    mgr.CallJsMonitors("onBeforeUndoRedo");
    CallMonitors([&mgr](TxnMonitor& monitor){monitor._OnPrepareForUndoRedo(mgr);});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnUndoRedo(TxnManager& mgr, TxnAction action)
    {
    int jsAction = (int) action;
    mgr.CallJsMonitors("onAfterUndoRedo", &jsAction);
    CallMonitors([&mgr, action](TxnMonitor& monitor){monitor._OnUndoRedo(mgr, action);});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
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
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DynamicChangeTracker::TrackChangesForTable DynamicChangeTracker::_FilterTable(Utf8CP tableName)
    {
    return m_txnMgr._FilterTable(tableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DynamicChangeTrackerPtr DynamicChangeTracker::Create(TxnManager& mgr)
    {
    return new DynamicChangeTracker(mgr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::BeginDynamicOperation()
    {
    auto tracker = DynamicChangeTracker::Create(*this);
    m_dynamicTxns.push_back(tracker);
    GetDgnDb().SetChangeTracker(tracker.get());
    tracker->EnableTracking(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
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

        Changes changes(changeset, false);
        AddChanges(changes);

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
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::CancelDynamics()
    {
    while (InDynamicTxn())
        EndDynamicOperation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
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
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId dgn_TxnTable::Model::Iterator::Entry::GetModelId() const {return m_sql->GetValueId<DgnModelId>(0);}
TxnTable::ChangeType dgn_TxnTable::Model::Iterator::Entry::GetChangeType() const {return (TxnTable::ChangeType) m_sql->GetValueInt(1);}
DgnClassId dgn_TxnTable::Model::Iterator::Entry::GetECClassId() const {return m_sql->GetValueId<DgnClassId>(2);}

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         08/2018
//---------------------------------------------------------------------------------------
void TxnManager::DumpTxns(bool verbose) const
    {
    TxnManagerR txnMgr = m_dgndb.Txns();

    TxnManager::TxnId endTxnId = txnMgr.GetCurrentTxnId();
    // unused - int64_t lastRebaseId = txnMgr.QueryLastRebaseId();

    TxnManager::TxnId startTxnId = txnMgr.QueryNextTxnId(TxnManager::TxnId(0));
    if (!startTxnId.IsValid() || startTxnId >= endTxnId)
        return;

    DgnDbR db = const_cast<TxnManager*>(this)->GetDgnDb();

    for (TxnManager::TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = txnMgr.QueryNextTxnId(currTxnId))
        {
        Utf8PrintfString title("\n\n---------- Txn#%lld ---------------", currTxnId.GetValue());
        if (txnMgr.IsSchemaChangeTxn(currTxnId))
            {
            DbSchemaChangeSet dbSchemaChangeSet;
            txnMgr.ReadDbSchemaChanges(dbSchemaChangeSet, currTxnId);
            dbSchemaChangeSet.Dump(Utf8PrintfString("%s - schemaChanges", title.c_str()).c_str());
            }
        else
            {
            AbortOnConflictChangeSet sqlChangeSet;
            txnMgr.ReadDataChanges(sqlChangeSet, currTxnId, TxnAction::None);

            if (verbose)
                sqlChangeSet.Dump(title.c_str(), db, false, 2);
            else
                {
                printf("%s\n\n", title.c_str());
                ChangeSummary changeSummary(db);
                changeSummary.FromChangeSet(sqlChangeSet);
                changeSummary.Dump();
                }
            }
        }
    }
