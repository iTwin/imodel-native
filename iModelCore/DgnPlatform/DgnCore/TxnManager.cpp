/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TxnManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//  temp table names used by TxnSummary and ElementGraphTxnMonitor
#define DGN_TABLE_TxnElements       DGN_TABLE_PREFIX "TxnElements"
#define DGN_TABLE_TxnElementDeps    DGN_TABLE_PREFIX "TxnElementDeps"

DPILOG_DEFINE(TxnManager)

extern "C" void DeleteFileW(WCharCP);

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/13
//=======================================================================================
struct UndoTracker : ChangeTracker
{
    bool m_wasTracking;
    static Utf8CP MyName() {return "undo-tracker";}
    static UndoTracker* Find(Db& db) {return (UndoTracker*) db.FindChangeTracker(MyName());}

    UndoTracker() : ChangeTracker (MyName()) {m_wasTracking=false;}

    OnCommitStatus _OnCommit(bool isCommit) override {return OnCommitStatus::Abort;}

    // we don't want any changes made during a "save setttings" to be undoable. Turn off tracker for until we're done.
    void _OnSettingsSave() override {m_wasTracking=m_isTracking; EnableTracking(false);}
    void _OnSettingsSaved() override {if (m_wasTracking) EnableTracking(true);}

    TrackChangesForTable _FilterTable(Utf8CP tableName) override
        {
        // Skip the range tree tables - they hold redundant data that will be automatically updated when the changeset is applied.
        // They all start with the string defined by DGNELEMENT_VTABLE_3dRTree
        if (0 == strncmp (DGN_VTABLE_PrjRTree, tableName, sizeof(DGN_VTABLE_PrjRTree)-1))
            return  TrackChangesForTable::No;

        return TrackChangesForTable::Yes;
        }
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/13
//=======================================================================================
struct UndoChangeSet : ChangeSet
{
    virtual ConflictResolution _OnConflict (ConflictCause cause, Changes::Change iter) override
        {
        BeAssert(false);
        return  ConflictResolution::Skip;
        }
};

#define UNDO_TABLE_Entry "Entry"
#define UNDO_TABLE_Data  "Data"
#define UNDO_TABLE_Mark  "Mark"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult UndoDb::Open()
    {
#ifdef DEBUG_MAKE_UNDODB_PERSISTENT
    Utf8CP tmp = "c:\\temp\\undo.db"; // for debugging undo logic
    ::DeleteFileW (tmp);
#else
    Utf8CP tmp = "";
#endif

    DbResult rc = CreateNewDb (tmp);
    if (rc != BE_SQLITE_OK)
        return  rc;

    CreateTable(UNDO_TABLE_Entry, "TxnId INTEGER PRIMARY KEY,Source INTEGER,Descr CHAR");
    CreateTable(UNDO_TABLE_Data,  "TxnId INTEGER,Changes BLOB");
    CreateTable(UNDO_TABLE_Mark,  "TxnId INTEGER PRIMARY KEY REFERENCES " UNDO_TABLE_Entry " ON DELETE CASCADE, Descr CHAR");
    ExecuteSql("CREATE TRIGGER delete_data AFTER DELETE ON " UNDO_TABLE_Entry " BEGIN DELETE FROM " UNDO_TABLE_Data " WHERE TxnId=OLD.TxnId; END");
    SaveChanges();

    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void UndoDb::Empty()
    {
    ExecuteSql("DELETE FROM " UNDO_TABLE_Mark);
    ExecuteSql("DELETE FROM " UNDO_TABLE_Data);
    ExecuteSql("DELETE FROM " UNDO_TABLE_Entry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void UndoDb::TruncateChanges(TxnId id)
    {
    if (0==id.GetValue())
        {
        Empty();
        return;
        }
    CachedStatementPtr stmt;
    GetCachedStatement(stmt, "DELETE FROM " UNDO_TABLE_Entry " WHERE TxnId>=?"); // deletes data too
    stmt->BindInt (1, id.GetValue());
    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult UndoDb::SaveEntry (TxnId id, uint64_t source, Utf8StringCR descr)
    {
    CachedStatementPtr insertEntry;
    DbResult rc = GetCachedStatement(insertEntry, "INSERT INTO " UNDO_TABLE_Entry " (TxnId,Source,Descr) VALUES(?,?,?)");
    if (BE_SQLITE_OK != rc)
        return  rc;

    insertEntry->BindInt(1, id.GetValue());
    insertEntry->BindInt64(2, source);
    insertEntry->BindText(3, descr, Statement::MakeCopy::No);
    rc = insertEntry->Step();
    BeAssert (rc == BE_SQLITE_DONE);
    return  rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult UndoDb::SaveChange (TxnId id, ChangeSet& changeset)
    {
    CachedStatementPtr insertData;
    DbResult rc = GetCachedStatement(insertData, "INSERT INTO " UNDO_TABLE_Data " (TxnId,Changes) VALUES(?,?)");
    if (BE_SQLITE_OK != rc)
        return  rc;

    insertData->BindInt   (1, id.GetValue());
    insertData->BindBlob  (2, changeset.GetData(), changeset.GetSize(), Statement::MakeCopy::No);
    rc = insertData->Step();
    BeAssert (rc == BE_SQLITE_DONE);
    return  rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult UndoDb::SaveMark (TxnId id, Utf8CP name)
    {
    Statement stmt;
    stmt.Prepare (*this, "INSERT OR REPLACE INTO " UNDO_TABLE_Mark " (TxnId,Descr) VALUES(?,?)");

    stmt.BindInt (1, id.GetValue());
    stmt.BindText (2, name, Statement::MakeCopy::No);
    return  stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
TxnId UndoDb::FindMark (Utf8StringR name, TxnId before)
    {
    name.clear();

    Statement stmt;
    stmt.Prepare (*this, "SELECT TxnId,Descr FROM " UNDO_TABLE_Mark " WHERE TxnId<? ORDER BY TxnId DESC");
    stmt.BindInt (1, before);

    DbResult rc = stmt.Step();
    if (rc != BE_SQLITE_ROW)
        return  TxnId(0);           // no marks saved, that means "undo all"

    name.assign(stmt.GetValueText(1));
    return TxnId(stmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult UndoDb::ReadEntry (TxnId id, uint64_t& source, Utf8StringR cmdName)
    {
    CachedStatementPtr selectEntry;
    DbResult rc = GetCachedStatement(selectEntry, "SELECT Source,Descr FROM " UNDO_TABLE_Entry " WHERE TxnId=?");
    if (BE_SQLITE_OK != rc)
        return  rc;

    selectEntry->BindInt(1, id.GetValue());
    rc = selectEntry->Step();
    if (rc != BE_SQLITE_ROW)
        return  rc;

    source = selectEntry->GetValueInt64(0);
    cmdName.assign(selectEntry->GetValueText(1));
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
UndoDb::ChangedFiles::Entry UndoDb::ChangedFiles::begin () const
    {
    BeAssert (!m_sql.IsPrepared());

    DbResult status = m_sql.Prepare (m_db, "SELECT Changes FROM " UNDO_TABLE_Data " WHERE TxnId=?");
    if (status != BE_SQLITE_OK)
        { BeAssert(false); }

    m_sql.BindInt (1, m_id.GetValue());

    return Entry (&m_sql, BE_SQLITE_ROW == m_sql.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void UndoDb::ChangedFiles::Entry::GetChangeSet(ChangeSet& changeSet, TxnDirection direction) const
    {
    changeSet.FromData (m_sql->GetColumnBytes(0), m_sql->GetValueBlob(0), direction==TxnDirection::Undo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2007
+---------------+---------------+---------------+---------------+---------------+------*/
ITxnManager::ITxnManager(DgnDbR project) : m_dgndb(project)
    {
    m_isActive          = false;
    m_boundaryMarked    = false;
    m_firstTxn.m_value  = 0;
    m_undoInProgress    = false;
    m_callRestartFunc   = false;
    m_inDynamics        = false;
    m_doChangePropagation = true;
    m_txnSource         = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::Deactivate()
    {
    if (!IsActive())
        return;

    m_db.CloseDb();
    SetActive (false);

    m_dgndb.DropChangeTracker(UndoTracker::MyName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::Activate()
    {
    if (IsActive())
        return;

    m_txnGroup.clear();
    m_boundaryMarked = false;
    m_currentTxnID.Init();
    m_reversedTxn.clear();
    m_firstTxn.m_value = 0;
    m_callRestartFunc = true;
    m_db.Open();

    SetActive (true);

    UndoTracker* tracker = new UndoTracker();
    DbResult stat = m_dgndb.AddChangeTracker(*tracker);
    if (stat != BE_SQLITE_OK)
        { BeAssert(false); }

    tracker->EnableTracking(true);
    }

bool ITxnManager::IsUndoInProgress() {return m_undoInProgress;}
bool ITxnManager::IsActive()         {return m_isActive;}
bool ITxnManager::HasEntries()       {return GetFirstTxnId() < GetCurrTxnId();}
bool ITxnManager::RedoIsPossible()   {return IsActive() && !m_reversedTxn.empty();}
TxnId ITxnManager::GetCurrTxnId()    {return m_currentTxnID;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::StartTxnGroup (bool startNewTxn)
    {
    if (startNewTxn)
        CheckTxnBoundary();

    m_txnGroup.push_back (GetCurrTxnId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::EndTxnGroup()
    {
    if (m_txnGroup.empty())
        {
        BeAssert (0);
        return;
        }

    m_txnGroup.pop_back();
    CheckTxnBoundary();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/08
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ITxnManager::GetTxnGroupCount()
    {
    return m_txnGroup.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
TxnId ITxnManager::GetCurrGroupStart()
    {
    return  m_txnGroup.empty() ? TxnId(0) : m_txnGroup.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::CancelChanges (BeSQLite::ChangeSet& direct)
    {
    UndoChangeSet inverted;     // invert it so we can reverse these changes
    inverted.FromData (direct.GetSize(), direct.GetData(), true);

    ApplyChangeSetInternal (inverted, m_currentTxnID, m_txnDescr, m_txnSource, TxnDirection::Undo, HowToCleanUpElements::CallCancelled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::SetUndoInProgress(bool yesNo)
    {
    BeAssert (m_undoInProgress != yesNo);
    BeAssert (IsActive());

    UndoTracker* undoTracker = UndoTracker::Find(m_dgndb);
    if (NULL == undoTracker)
        {
        BeAssert (false);
        return;
        }

    // we're about to reverse or reinstate a committed transaction. Before that can happen, we need to cancel
    // any pending uncommitted changes.
    if (yesNo && undoTracker->HasChanges())
        {
        UndoChangeSet direct;        // get the current changeset holding the uncommitted changes
        direct.FromChangeTrack (*undoTracker);

        CancelChanges (direct);

        // clear the undo tracker since these changes are gone
        undoTracker->EndTracking();
        }

    // While we're processing undo, we don't want to be tracking changes.
    undoTracker->EnableTracking(!yesNo);
    m_undoInProgress = yesNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ITxnManager::ComputeIndirectChanges(BeSQLite::ChangeSet& changeset)
    {
    TxnSummary summary(m_dgndb, m_currentTxnID, m_txnDescr, m_txnSource, changeset);

    if (summary.HasModelDependencyChanges())
        {
        //  If there were changes to model dependencies, then we have to recompute model dependency order, updating the "DependencyIndex" column of the Model table.
        //  This must be done before invoking the ElementDependencyChangeMonitor below. This must be part of *indirect* changes, so that they will be re-played during change-merging.
        UpdateModelDependencyIndex();

        if (HasAnyFatalValidationErrors())
            return BSIERROR;
        }

    if (summary.GetDgnDb().GetTxnManager().GetDoChangePropagation())
        {
        DgnElementDependencyGraph graph (m_dgndb, &summary);
        graph.InvokeAffectedDependencyHandlers (summary);
        }

    T_HOST.GetTxnAdmin()._OnTxnBoundary(summary);

    return HasAnyFatalValidationErrors() ? BSIERROR : BSISUCCESS;
    }

//=======================================================================================
// @bsiclass                                    Sam.Wilson                      02/2015
//=======================================================================================
struct ClearOnReturn
    {
    bool& m_var;
    ClearOnReturn (bool& v) : m_var(v) {;}
    ~ClearOnReturn() {m_var = false;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::CheckTxnBoundary()
    {
    m_validationErrors.clear();

    if (!m_boundaryMarked || !m_txnGroup.empty())     // only if there's no group active
        return;

    ClearOnReturn clearBoundaryMarkedOnReturn(m_boundaryMarked);

    UndoTracker* undoTracker = UndoTracker::Find(m_dgndb);
    if (undoTracker==NULL || !undoTracker->HasChanges())
        return;

    TxnId startPos = GetCurrTxnId(); // in case we have to roll back

    // Grab and hold in memory all of the direct changes. We'll use this changeset to drive indirect changes, and we'll also save this changeset toward the end of the function.
    UndoChangeSet changeset;
    auto dbresult = changeset.FromChangeTrack(*undoTracker);
    if (BE_SQLITE_OK != dbresult)
        {
        CancelToPos (startPos);
        LOG.errorv("ITxnManager::CheckTxnBoundary failed to get the changeset. Error=%x.", dbresult);
        return;
        }

    if (0 == changeset.GetSize())
        {
        BeAssert(false && "We should have already checked for no changes");
        return;
        }

    //  Compute indirect changes
    undoTracker->Restart();
    undoTracker->SetIndirectChanges(true);
    BentleyStatus status = ComputeIndirectChanges(changeset);
    undoTracker->SetIndirectChanges(false);

    // *** DO NOT RETURN WITHOUT CALLING RESTART ***

    if (BSISUCCESS != status)
        {
        // NB: Can't call CancelToPos here, because we have already extracted the changeset from undoTracker.
        undoTracker->EnableTracking(false); 
        UndoChangeSet indirectChanges; 
        indirectChanges.FromChangeTrack(*undoTracker);
        CancelChanges (indirectChanges);    // undo in reverse order: indirect first    *** WIP If I didn't have to undo in inverse order, I could cancel direct changes
        CancelChanges (changeset);          //      "       "         directly after    ***     first, then free it, and then get and cancel indirect changes.
        LOG.errorv("Cancelling txn due to fatal validation error.");
        undoTracker->EnableTracking(true);  // resume undoable changes
        }
    else
        {
        m_db.SaveChange(m_currentTxnID, changeset);
        changeset.Free(); // done with changeset -- free memory now, before allocating memory to hold indirect changes.
        
        UndoChangeSet indirectChanges; 
        indirectChanges.FromChangeTrack(*undoTracker);
        m_db.SaveChange(m_currentTxnID, indirectChanges);
        
        m_db.SaveEntry(m_currentTxnID, m_txnSource, m_txnDescr);
        m_db.SaveChanges();
        m_currentTxnID.Next();
        }

    undoTracker->Restart();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ITxnManager::IValidationErrorPtr> ITxnManager::GetValidationErrors() const
    {
    return m_validationErrors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ITxnManager::GetValidationErrorCount() const
    {
    return m_validationErrors.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ReportValidationError (IValidationError& e)
    {
    m_validationErrors.push_back (&e);

    auto sev = (e.GetSeverity() == ValidationErrorSeverity::Fatal)? "Fatal": "Warning";
    LOG.errorv("Validation error. Severity:%s Class:[%s] Description:[%s]", sev, typeid(e).name(), e.GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ITxnManager::HasAnyFatalValidationErrors() const
    {
    for (auto const& e : m_validationErrors)
        {
        if (e->GetSeverity() == ValidationErrorSeverity::Fatal)
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnSummary::GetChangedElementsTableName() const {return m_dgndb.GetTxnManager().GetChangedElementsTableName();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ITxnManager::GetChangedElementsTableName() const {return "temp." DGN_TABLE_TxnElements;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnSummary::GetChangedElementDrivesElementRelationshipsTableName() const {return m_dgndb.GetTxnManager().GetChangedElementDrivesElementRelationshipsTableName();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ITxnManager::GetChangedElementDrivesElementRelationshipsTableName() const {return "temp." DGN_TABLE_TxnElementDeps;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::InitTempTables()
    {
    //  DGN_TABLE_TxnElementDeps
        { 
        auto ttable = GetChangedElementsTableName();
        if (m_dgndb.TableExists (ttable.c_str()))
            return;

        //  Create DGN_TABLE_TxnElements
        auto stat = m_dgndb.CreateTable (ttable.c_str(), "ElementId INTEGER NOT NULL PRIMARY KEY, ModelId INTEGER NOT NULL, Op CHAR");
        BeAssert (stat == BE_SQLITE_OK);
        BeAssert (ttable.substr (0, 5) == "temp.");
        auto basetable = ttable.substr(5); // drop the "temp." prefix 
        stat = m_dgndb.ExecuteSql (Utf8PrintfString("CREATE INDEX %sModelIdx ON %s (ModelId)", ttable.c_str(), basetable.c_str()));
        BeAssert (stat == BE_SQLITE_OK);
        stat = m_dgndb.ExecuteSql (Utf8PrintfString("CREATE INDEX %sElementIdx ON %s (ElementId)", ttable.c_str(), basetable.c_str()));
        BeAssert (stat == BE_SQLITE_OK);
        }

    //  DGN_TABLE_TxnElements
        {
        auto ttable = GetChangedElementDrivesElementRelationshipsTableName();
        auto stat = m_dgndb.CreateTable (ttable.c_str(), "ECInstanceId INTEGER NOT NULL PRIMARY KEY, ModelId INTEGER NOT NULL, Op CHAR");
        auto basetable = ttable.substr(5); // drop the "temp." prefix 
        stat = m_dgndb.ExecuteSql (Utf8PrintfString("CREATE INDEX %sModelIdx ON %s (ModelId)", ttable.c_str(), basetable.c_str()));
        stat = m_dgndb.ExecuteSql (Utf8PrintfString("CREATE INDEX %sECInstanceIdIdx ON %s (ECInstanceId)", ttable.c_str(), basetable.c_str()));
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnSummary::AddAffectedElement(DgnElementId const& eid, DgnModelId mid, ChangeType changeType)
    {
    if (!mid.IsValid())
        mid = m_dgndb.Elements().QueryModelId(eid);

    if (!m_addElementStatement.IsValid())
        m_dgndb.GetCachedStatement (m_addElementStatement, Utf8PrintfString("INSERT INTO %s (ElementId,ModelId,Op) VALUES(?,?,?)", GetChangedElementsTableName().c_str()));

    m_addElementStatement->Reset();
    m_addElementStatement->ClearBindings();
    m_addElementStatement->BindId(1, eid);
    m_addElementStatement->BindId(2, mid);
    m_addElementStatement->BindText(3, (changeType==ChangeType::Add)? "+": (changeType==ChangeType::Delete)? "-": "*", BeSQLite::Statement::MakeCopy::No);
    m_addElementStatement->Step();

    m_affectedModels.insert (mid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnSummary::AddAffectedDependency (BeSQLite::EC::ECInstanceId const& relid, ChangeType ct)
    {
    CachedECSqlStatementPtr selectSourceElementModelId = GetDgnDb().GetPreparedECSqlStatement (
"SELECT element.ModelId FROM " DGN_SCHEMA(DGN_CLASSNAME_Element) " AS element, " DGN_SCHEMA(DGN_RELNAME_ElementDrivesElement)  " AS DEP"
" WHERE (DEP.ECInstanceId=?) AND (element.ECInstanceId=DEP.SourceECInstanceId)");
    selectSourceElementModelId->BindId(1, relid);
    auto stat = selectSourceElementModelId->Step();
    BeAssert(stat == ECSqlStepStatus::HasRow);
    DgnModelId mid = selectSourceElementModelId->GetValueId<DgnModelId>(0);

    if (!m_addElementDepStatement.IsValid())
        m_dgndb.GetCachedStatement (m_addElementDepStatement, Utf8PrintfString("INSERT INTO %s (ECInstanceId,ModelId,Op) VALUES(?,?,?)", GetChangedElementDrivesElementRelationshipsTableName().c_str()));

    m_addElementDepStatement->Reset();
    m_addElementDepStatement->ClearBindings();
    m_addElementDepStatement->BindId(1, relid);
    m_addElementDepStatement->BindId(2, mid);
    m_addElementDepStatement->BindText(3, (ct==ChangeType::Add)? "+": (ct==ChangeType::Delete)? "-": "*", BeSQLite::Statement::MakeCopy::No);
    m_addElementDepStatement->Step();

    m_affectedModels.insert (mid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
TxnSummary::TxnSummary(DgnDbR project, TxnId txnId, Utf8String txnDescr, uint64_t txnSource, ChangeSet& changeset) :
                       m_dgndb(project), m_txnId(txnId), m_txnDescr(txnDescr), m_txnSource(txnSource), m_modelDepsChanged(false), m_elementDepsChanged(false)
    {
    m_dgndb.GetTxnManager().InitTempTables();

    Utf8String currTable;
    DgnDomain::TableHandler* tblHandler = 0;

    Changes changes(changeset);
    for (auto change : changes)
        {
        Utf8CP tableName;
        int nCols,indirect;
        DbOpcode opcode;
        DbResult rc = change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        BeAssert (rc==BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);

        if (0 != strcmp (currTable.c_str(), tableName)) // changes within a changeset are grouped by table
            {
            currTable = tableName;
            tblHandler = DgnDomains::FindTableHandler (tableName);
            }

        if (NULL == tblHandler)
            continue;

        switch (opcode)
            {
            case DbOpcode::Delete:
                tblHandler->_OnDelete(*this, change);
                break;
            case DbOpcode::Insert:
                tblHandler->_OnAdd(*this, change);
                break;
            case DbOpcode::Update:
                tblHandler->_OnUpdate(*this, change);
                break;
            default:
                BeAssert(false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
TxnSummary::TxnSummary (DgnElementDependencyGraph const& graph) : m_dgndb(graph.GetDgnDb()), m_txnId(TxnId(0)), m_txnSource(0), m_modelDepsChanged(false), m_elementDepsChanged(false)
    {
    m_dgndb.GetTxnManager().InitTempTables();
    // DgnElementDependencyGraph will call AddAffectedElement, AddAffectedDependency directly
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnSummary::~TxnSummary ()
    {
    m_dgndb.ExecuteSql (Utf8PrintfString("DELETE FROM %s", GetChangedElementsTableName().c_str()));
    m_dgndb.ExecuteSql (Utf8PrintfString("DELETE FROM %s", GetChangedElementDrivesElementRelationshipsTableName().c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ITxnManager::ApplyChangeSetInternal(BeSQLite::ChangeSet& changeset, TxnId txnId, 
        Utf8StringCR txnDescr, uint64_t txnSource, TxnDirection isUndo, HowToCleanUpElements cleanup)
    {
    TxnSummary summary (m_dgndb, txnId, txnDescr, txnSource, changeset);

    // notify monitors that changeset is about to be applied
    T_HOST.GetTxnAdmin()._OnTxnReverse(summary, isUndo);

    DbResult rc = changeset.ApplyChanges(m_dgndb);
    if (rc != BE_SQLITE_OK)
        {
        LOG.errorv("ApplyChangeSet failed with %x", rc);
        BeAssert(false);
        }

    if (true)
        {
        if (cleanup == HowToCleanUpElements::CallApplied)
            m_dgndb.Elements().OnChangesetApplied(summary);

        // At this point, all of the changes to all tables have been applied.
        T_HOST.GetTxnAdmin()._OnTxnReversed(summary, isUndo);
        }

    if (cleanup == HowToCleanUpElements::CallCancelled)
        m_dgndb.Elements().OnChangesetCanceled(summary);

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ITxnManager::ApplyChangeSet(BeSQLite::ChangeSet& changeset, TxnId txnId, Utf8StringCR txnDescr, uint64_t txnSource, TxnDirection isUndo)
    {
    return ApplyChangeSetInternal(changeset, txnId, txnDescr, txnSource, isUndo, HowToCleanUpElements::CallApplied);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ApplyChanges(TxnId txnId, TxnDirection direction)
    {
    Utf8String txnDescr; uint64_t txnSource;
    m_db.ReadEntry (txnId, txnSource, txnDescr);

    UndoDb::ChangedFiles files(m_db, txnId);
    for (auto& entry : files)
        {
        UndoChangeSet changeset;
        entry.GetChangeSet(changeset, direction);

        ApplyChangeSet (changeset, txnId, txnDescr, txnSource, direction);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::CancelChanges (TxnId txnId)
    {
    Utf8String txnDescr; uint64_t txnSource;
    m_db.ReadEntry (txnId, txnSource, txnDescr);

    UndoDb::ChangedFiles files(m_db, txnId);
    for (auto entry : files)
        {
        UndoChangeSet changeset;
        entry.GetChangeSet(changeset, TxnDirection::Undo);

        TxnSummary summary (m_dgndb, txnId, txnDescr, txnSource, changeset);
        m_dgndb.Elements().OnChangesetCanceled(summary);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool ITxnManager::HasAnyChanges()
    {
    UndoTracker* undoTracker = UndoTracker::Find(m_dgndb);
    return undoTracker && undoTracker->HasChanges();
    }

/*---------------------------------------------------------------------------------**//**
* Called to delineate a "transaction".
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::CloseCurrentTxn ()
    {
    if (m_undoInProgress)
        {
        BeAssert(false);
        return;           // we're somehow back here from within an undo/redo callback. Ignore this. TR#253986
        }

    m_boundaryMarked = true;
    CheckTxnBoundary();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ReverseTxnRange (TxnRange& txnRange, Utf8StringP undoStr, bool multiStep)
    {
    SetUndoInProgress(true);

    for (TxnId curr (txnRange.GetLast()-1); curr.IsValid() && (curr >= txnRange.GetFirst()); curr.Prev())
        ApplyChanges(curr, TxnDirection::Undo);

    SetUndoInProgress(false);

    if (undoStr)
        {
        uint64_t source;
        Utf8String cmdName;
        Utf8String fmtString = DgnCoreL10N::GetString(DgnCoreL10N::UNDOMSG_FMT_UNDONE);
        m_db.ReadEntry (txnRange.GetFirst(), source, cmdName);
        undoStr->assign (cmdName + fmtString);
        }

    m_currentTxnID = txnRange.GetFirst();

    // save in undone txn log
    RevTxn revTxn(txnRange, multiStep);
    m_reversedTxn.push_back(revTxn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxnManager::ReverseToPos (TxnId pos)
    {
    if (!PrepareForUndo())
        return ERROR;

    TxnId last = GetCurrTxnId();
    if (pos > last)
        return ERROR;

    TxnRange range (pos, last);
    return ReverseActions (range, true, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxnManager::CancelToPos (TxnId pos, bool callRestartFunc)
    {
    if (!PrepareForUndo())
        return ERROR;

    AutoRestore<bool> saveCallRestartFunc (&m_callRestartFunc, callRestartFunc);

    StatusInt status = ReverseToPos (pos);
    if (SUCCESS == status)
        ClearReversedTxns();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxnManager::ReverseActions (TxnRange& txnRange, bool multiStep, bool showMsg)
    {
    Utf8String undoStr;
    ReverseTxnRange (txnRange, &undoStr, multiStep);     // do the actual undo now.

    while (GetCurrTxnId() < GetCurrGroupStart())
        EndTxnGroup();

    T_HOST.GetTxnAdmin()._OnUndoRedoFinished (m_dgndb, TxnDirection::Undo);

    if (showMsg)
        NotificationManager::OutputMessage (NotifyMessageDetails (OutputMessagePriority::Info, undoStr.c_str()));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool ITxnManager::PrepareForUndo()
    {
    if (IsActive() && HasEntries())
        {
        T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo();
        return  true;
        }

    T_HOST.GetTxnAdmin()._OnNothingToUndo();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ReverseTxns (int numActions)
    {
    if (!PrepareForUndo())
        return;

    TxnId last = GetCurrTxnId();
    TxnId first(last - numActions);

    TxnRange range(first, last);
    ReverseActions(range, numActions>1, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/08
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ReverseSingleTxn (bool callRestartFunc)
    {
    AutoRestore <bool> saveCallRestartFunc (&m_callRestartFunc, callRestartFunc);
    ReverseTxns(1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ReverseToMark(Utf8StringR name)
    {
    if (!PrepareForUndo())
        return;

    TxnId markId (m_db.FindMark(name, GetCurrTxnId()));

    TxnRange range(markId, GetCurrTxnId());
    ReverseActions(range, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* reverse (undo) all previous transactions
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ReverseAll (bool prompt)
    {
    if (!PrepareForUndo())
        return;

    if (prompt && !T_HOST.GetTxnAdmin()._OnPromptReverseAll())
        {
        T_HOST.GetTxnAdmin()._RestartTool();
        return;
        }

    TxnRange range(GetFirstTxnId(), GetCurrTxnId());
    ReverseActions(range, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* Reinstate ("redo") a range of transactions. Also returns the string that identifies what was reinstated.
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ReinstateTxn (TxnRange& revTxn, Utf8StringP redoStr)
    {
    BeAssert (m_currentTxnID == revTxn.GetFirst());

    SetUndoInProgress(true);

    for (TxnId curr (revTxn.GetFirst()); curr.IsValid() && curr < revTxn.GetLast(); curr.Next())
        ApplyChanges(curr, TxnDirection::Redo);

    SetUndoInProgress(false);

    if (redoStr)
        {
        uint64_t source;
        Utf8String cmdName;
        Utf8String fmtString = DgnCoreL10N::GetString(DgnCoreL10N::UNDOMSG_FMT_REDONE);
        m_db.ReadEntry (revTxn.GetFirst(), source, cmdName);
        redoStr->assign (cmdName + fmtString);
        }

    m_currentTxnID = revTxn.GetLast();
    m_reversedTxn.pop_back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxnManager::ReinstateActions (RevTxn& revTxn)
    {
    Utf8String redoStr;
    ReinstateTxn (revTxn.m_range, &redoStr);     // do the actual redo now.

    T_HOST.GetTxnAdmin()._OnUndoRedoFinished (m_dgndb, TxnDirection::Redo);

    NotificationManager::OutputMessage (NotifyMessageDetails (OutputMessagePriority::Info, redoStr.c_str()));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxnManager::ReinstateTxn()
    {
    if (!IsActive() || !RedoIsPossible())
        {
        T_HOST.GetTxnAdmin()._OnNothingToRedo();
        return ERROR;
        }

    T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo();

    RevTxn*  revTxn = &m_reversedTxn.back();
    return  ReinstateActions(*revTxn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::GetUndoString (Utf8StringR string)
    {
    string.clear();
    if (!HasEntries())
        return;

    uint64_t source;
    m_db.ReadEntry(TxnId(GetCurrTxnId()-1), source, string);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::GetRedoString (Utf8StringR string)
    {
    string.clear();
    if (!RedoIsPossible())
        return;

    RevTxn*  revTxn = &m_reversedTxn.back();
    if (revTxn->m_multiStep)
        return;

    uint64_t source;
    m_db.ReadEntry (revTxn->m_range.GetFirst(), source, string);
    }

/*---------------------------------------------------------------------------------**//**
* Cancel any undone (rolled-back) transactions. Table Handlers use this to free any memory associated with
* the transactions, since they will no longer be reachable.
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ClearReversedTxns()
    {
    if (m_reversedTxn.empty())
        return;                     // nothing currently undone, nothing to do

    RevTxn firstTxn = m_reversedTxn.back(); // oldest, most recently undone
    RevTxn lastTxn  = m_reversedTxn.front();
    for (TxnId curr (lastTxn.m_range.GetLast()-1); curr.IsValid() && (curr >= firstTxn.m_range.GetFirst()); curr.Prev())
        CancelChanges(curr);

    // these transactions are no longer reinstateable. Throw them away.
    m_reversedTxn.clear();
    m_db.TruncateChanges(firstTxn.m_range.GetFirst());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ITxnManager::SaveUndoMark(Utf8CP name)
    {
    if (!IsActive())
        return ERROR;

    ClearReversedTxns();

    m_db.SaveMark (TxnId(m_currentTxnID-1), name);
    return  SUCCESS;
    }

void ITxnManager::SetTxnSource(uint64_t source) {m_txnSource = source;}
void ITxnManager::SetTxnDescription(Utf8CP descr) {m_txnDescr.assign(descr);}

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
void DgnPlatformLib::Host::TxnAdmin::DropTxnMonitor (TxnMonitor& monitor)
    {
    auto it = std::find(m_monitors.begin(), m_monitors.end(), &monitor);
    if (it != m_monitors.end())
        *it = NULL; // removed from list by CallMonitors
    }

template <typename CALLER> void DgnPlatformLib::Host::TxnAdmin::CallMonitors(CALLER const& caller)
    {
    for (auto curr = m_monitors.begin(); curr!=m_monitors.end(); )
        {
        if (*curr == NULL)
            curr = m_monitors.erase(curr);
        else
            {
            try {caller(**curr); ++curr;}
            catch (...) {}
            }
        }
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnReverseCaller
{
    TxnSummaryCR m_summary;
    TxnDirection m_isUndo;
    TxnReverseCaller(TxnSummaryCR summary, TxnDirection isUndo) : m_summary(summary), m_isUndo(isUndo) {}
    void operator() (TxnMonitorR monitor) const {monitor._OnTxnReverse(m_summary, m_isUndo);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnReversedCaller
{
    TxnSummaryCR m_summary;
    TxnDirection m_isUndo;
    TxnReversedCaller(TxnSummaryCR summary, TxnDirection isUndo) : m_summary(summary), m_isUndo(isUndo) {}
    void operator() (TxnMonitorR monitor) const {monitor._OnTxnReversed(m_summary, m_isUndo);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnBoundaryCaller
{
    TxnSummaryCR m_summary;
    TxnBoundaryCaller(TxnSummaryCR summary) : m_summary(summary) {}
    void operator() (TxnMonitorR monitor) const {monitor._OnTxnBoundary(m_summary);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct UndoRedoFinishedCaller
    {
    DgnDbR   m_dgndb;
    TxnDirection m_isUndo;
    UndoRedoFinishedCaller (DgnDbR project, TxnDirection isUndo) : m_dgndb(project), m_isUndo(isUndo) {}
    void operator() (TxnMonitorR handler) const {handler._OnUndoRedoFinished(m_dgndb, m_isUndo);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnTxnBoundary (TxnSummaryCR summary)
    {
    CallMonitors (TxnBoundaryCaller(summary));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnTxnReverse (TxnSummaryCR summary, TxnDirection isUndo)
    {
    CallMonitors (TxnReverseCaller(summary, isUndo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnTxnReversed (TxnSummaryCR summary, TxnDirection isUndo)
    {
    CallMonitors (TxnReversedCaller(summary, isUndo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnUndoRedoFinished (DgnDbR project, TxnDirection isUndo)
    {
    CallMonitors (UndoRedoFinishedCaller (project, isUndo));
    }

