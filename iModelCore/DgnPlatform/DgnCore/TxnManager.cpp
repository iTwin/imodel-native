/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TxnManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

extern "C" void DeleteFileW(WCharCP);

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct CompareTableNames
{
    bool operator() (Utf8CP a, Utf8CP b) const {return strcmp(a, b) < 0;}
};

typedef bmap<Utf8CP,TxnTableHandler*,CompareTableNames>   T_HandlerMap;
static T_HandlerMap  s_tableHandlers;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/13
//=======================================================================================
struct UndoTracker : ChangeTracker
{
    bool m_wasTracking;
    static Utf8CP MyName() {return "undo-tracker";}
    static UndoTracker* Find(Db& db) {return (UndoTracker*) db.FindChangeTracker(MyName());}

    UndoTracker() : ChangeTracker (MyName()) {m_wasTracking=false;}

    // we don't want any changes made during a "save setttings" to be undoable. Turn off tracker for until we're done.
    virtual void _OnSettingsSave() override {m_wasTracking=m_isTracking; EnableTracking(false);}
    virtual void _OnSettingsSaved() override {if (m_wasTracking) EnableTracking(true);}

    virtual TrackChangesForTable _FilterTable(Utf8CP tableName) override
        {
        // Skip the range tree tables - they hold redundant data that will be automatically updated when the changeset is applied.
        // They all start with the string defined by DGNELEMENT_VTABLE_3dRTree
        if (0 == strncmp (DGN_VTABLE_PrjRTree, tableName, sizeof(DGN_VTABLE_PrjRTree)-1))
            return  TRACK_TABLE_No;

        return TRACK_TABLE_Yes;
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
        return  CONFLICT_RESOLUTION_Skip;
        }
};

//=======================================================================================
//! for debugging, BeAssert if anyone tries to write a transactionable change while this object is on the stack.
//! @bsiclass                                                     Keith.Bentley   03/07
//=======================================================================================
struct  IllegalTxnMark : IllegalTxn
{
private:
    ITxnManager&    m_mgr;
    ITxn*           m_oldTxn;

public:
    IllegalTxnMark (ITxnManager& mgr) : m_mgr(mgr) {m_oldTxn = &mgr.SetCurrentTxn (*this); mgr.GetDgnProject().ExecuteSql("pragma query_only=TRUE");}
    ~IllegalTxnMark () {m_mgr.GetDgnProject().ExecuteSql("pragma query_only=FALSE"); m_mgr.SetCurrentTxn (*m_oldTxn);}
};

static UndoableTxn  s_undoableTxn;

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

    CreateTable (UNDO_TABLE_Entry, "TxnId INTEGER PRIMARY KEY,Source INTEGER,Descr CHAR");
    CreateTable (UNDO_TABLE_Data,  "TxnId INTEGER,Changes BLOB");
    CreateTable (UNDO_TABLE_Mark,  "TxnId INTEGER PRIMARY KEY REFERENCES " UNDO_TABLE_Entry " ON DELETE CASCADE, Descr CHAR");
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
DbResult UndoDb::SaveEntry (TxnId id, UInt64 source, Utf8StringCR descr)
    {
    CachedStatementPtr insertEntry;
    DbResult rc = GetCachedStatement(insertEntry, "INSERT INTO " UNDO_TABLE_Entry " (TxnId,Source,Descr) VALUES(?,?,?)");
    if (BE_SQLITE_OK != rc)
        return  rc;

    insertEntry->BindInt(1, id.GetValue());
    insertEntry->BindInt64(2, source);
    insertEntry->BindText(3, descr, Statement::MAKE_COPY_No);
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
    insertData->BindBlob  (2, changeset.GetData(), changeset.GetSize(), Statement::MAKE_COPY_No);
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
    stmt.BindText (2, name, Statement::MAKE_COPY_No);
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
DbResult UndoDb::ReadEntry (TxnId id, UInt64& source, Utf8StringR cmdName)
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
void UndoDb::ChangedFiles::Entry::GetChangeSet(ChangeSet& changeSet, bool invert) const
    {
    changeSet.FromData (m_sql->GetColumnBytes(0), m_sql->GetValueBlob(0), invert);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2007
+---------------+---------------+---------------+---------------+---------------+------*/
ITxnManager::ITxnManager(DgnProjectR project) : m_project(project)
    {
    m_isActive          = false;
    m_boundaryMarked    = false;
    m_firstTxn.m_value  = 0;
    m_undoInProgress    = false;
    m_callRestartFunc   = false;
    m_inDynamics        = false;
    m_txnSource         = 0;
    m_currTxn           = NULL;

    SetCurrentTxn (s_undoableTxn);
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
    SetCurrentTxn (s_undoableTxn);

    m_project.DropChangeTracker(UndoTracker::MyName());
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

    SetCurrentTxn (s_undoableTxn);
    SetActive (true);

    UndoTracker* tracker = new UndoTracker();
    DbResult stat = m_project.AddChangeTracker(*tracker);
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
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::SetUndoInProgress(bool yesNo)
    {
    BeAssert (m_undoInProgress != yesNo);
    BeAssert (IsActive());

    UndoTracker* undoTracker = UndoTracker::Find(m_project);
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

        UndoChangeSet inverted;     // invert it so we can reverse these changes
        inverted.FromData (direct.GetSize(), direct.GetData(), true);

        TxnSummary summary (m_project, m_currentTxnID, m_txnDescr, m_txnSource, inverted);

        // notify monitors
        if (true)
            {
            IllegalTxnMark _v_v_v(*this);      // don't allow database changes
            T_HOST.GetTxnAdmin()._OnTxnReverse(summary, true);
            }

        // reverse the changes.
        DbResult rc = inverted.ApplyChanges(m_project);
        if (rc != BE_SQLITE_OK)
            { BeAssert(false); }

        if (true)
            {
            IllegalTxnMark _v_v_v(*this);      // don't allow database changes
            T_HOST.GetTxnAdmin()._OnTxnReversed(summary, true);
            }

        m_project.Models().ElementPool().OnChangesetCanceled(summary);

        // clear the undo tracker since these changes are gone
        undoTracker->EndTracking();
        }

    // While we're processing undo, we don't want to be tracking changes.
    undoTracker->EnableTracking(!yesNo);
    m_undoInProgress = yesNo;
    }


#define MAX_PROPAGATION_WAVES 5 // only allow 5 waves of change propagation.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::CheckTxnBoundary()
    {
    if (!m_boundaryMarked || !m_txnGroup.empty())     // only if there's no group active
        return;

    UndoTracker* undoTracker = UndoTracker::Find(m_project);
    if (undoTracker==NULL || !undoTracker->HasChanges())
        return;

    for (int wave=0; wave<MAX_PROPAGATION_WAVES; ++wave)
        {
        UndoChangeSet changeset;
        changeset.FromChangeTrack (*undoTracker);

        undoTracker->Restart();
        undoTracker->SetIndirectChanges(true);

        if (0 == changeset.GetSize())
            break;

        TxnSummary summary (m_project, m_currentTxnID, m_txnDescr, m_txnSource, changeset);
        T_HOST.GetTxnAdmin()._OnTxnBoundary (summary);

        m_db.SaveChange(m_currentTxnID, changeset);
        }

    undoTracker->SetIndirectChanges(false);

    m_db.SaveEntry(m_currentTxnID, m_txnSource, m_txnDescr);
    m_db.SaveChanges();

    m_boundaryMarked = false;
    m_currentTxnID.Next();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
TxnSummary::TxnSummary (DgnProjectR project, TxnId txnId, Utf8String txnDescr, UInt64 txnSource, ChangeSet& changeset) :
        m_project(project), m_txnId(txnId), m_txnDescr(txnDescr), m_txnSource(txnSource)
    {
    m_physicalRange.Init();
    m_drawingRange.Init();

    Utf8String currTable;
    TxnTableHandler* tblHandler = 0;

    Changes changes(changeset);
    for (auto change : changes)
        {
        Utf8CP tableName;
        int nCols,indirect;
        DbOpcode opcode;
        DbResult rc = change.GetOperation (&tableName, &nCols, &opcode, &indirect);
        if (rc!=BE_SQLITE_OK)
            { BeAssert(false); }

        if (0 != strcmp (currTable.c_str(), tableName)) // changes within a changeset are grouped by table
            {
            currTable = tableName;
            tblHandler = ITxnManager::FindTableHandler (tableName);
            }

        if (NULL == tblHandler)
            continue;

        switch (opcode)
            {
            case BE_SQLITEOP_DELETE:
                tblHandler->_OnDelete (*this, change);
                break;
            case BE_SQLITEOP_INSERT:
                tblHandler->_OnAdd(*this, change);
                break;
            case BE_SQLITEOP_UPDATE:
                tblHandler->_OnUpdate (*this, change);
                break;
            default:
                BeAssert(false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ITxnManager::ApplyChangeSet (BeSQLite::ChangeSet& changeset, TxnId txnId, Utf8StringCR txnDescr, UInt64 txnSource, bool isUndo)
    {
    TxnSummary summary (m_project, txnId, txnDescr, txnSource, changeset);

    // notify monitors that changeset is about to be applied
    if (true)
        {
        IllegalTxnMark _v_v_v(*this);      // don't allow any database changes
        T_HOST.GetTxnAdmin()._OnTxnReverse(summary, isUndo);
        }

    DbResult rc = changeset.ApplyChanges(m_project);
    if (rc != BE_SQLITE_OK)
        {
        BeAssert(false);
        }

    if (true)
        {
        IllegalTxnMark _v_v_v(*this);      // don't allow any database changes
        m_project.Models().ElementPool().OnChangesetApplied(summary);

        // At this point, all of the changes to all tables have been applied.
        T_HOST.GetTxnAdmin()._OnTxnReversed(summary, isUndo);
        }

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ApplyChanges (TxnId txnId, TxnApplyDirection direction)
    {
    bool isUndo = direction==APPLY_FOR_Undo;

    Utf8String txnDescr; UInt64 txnSource;
    m_db.ReadEntry (txnId, txnSource, txnDescr);

    UndoDb::ChangedFiles files(m_db, txnId);
    for (auto entry : files)
        {
        UndoChangeSet changeset;
        entry.GetChangeSet(changeset, isUndo);

        ApplyChangeSet (changeset, txnId, txnDescr, txnSource, isUndo);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::CancelChanges (TxnId txnId)
    {
    Utf8String txnDescr; UInt64 txnSource;
    m_db.ReadEntry (txnId, txnSource, txnDescr);

    UndoDb::ChangedFiles files(m_db, txnId);
    for (auto entry : files)
        {
        UndoChangeSet changeset;
        entry.GetChangeSet(changeset, true);

        TxnSummary summary (m_project, txnId, txnDescr, txnSource, changeset);
        m_project.Models().ElementPool().OnChangesetCanceled(summary);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool ITxnManager::HasAnyChanges()
    {
    UndoTracker* undoTracker = UndoTracker::Find(m_project);
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
        ApplyChanges(curr, APPLY_FOR_Undo);

    SetUndoInProgress(false);

    if (undoStr)
        {
        UInt64 source;
        Utf8String cmdName;
        Utf8String fmtString = DgnCoreL10N::GetString(DgnCoreL10N::UNDOMSG_FMT_UNDONE);
        m_db.ReadEntry (txnRange.GetFirst(), source, cmdName);
        undoStr->assign (cmdName + fmtString);
        }

    m_currentTxnID = txnRange.GetFirst();

    // save in undone txn log
    RevTxn revTxn (txnRange, multiStep);
    m_reversedTxn.push_back (revTxn);
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

    T_HOST.GetTxnAdmin()._OnUndoRedoFinished (m_project, true);

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

    TxnRange range (first, last);
    ReverseActions (range, numActions>1, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/08
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ReverseSingleTxn (bool callRestartFunc)
    {
    AutoRestore <bool> saveCallRestartFunc (&m_callRestartFunc, callRestartFunc);
    ReverseTxns (1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::ReverseToMark(Utf8StringR name)
    {
    if (!PrepareForUndo())
        return;

    TxnId markId (m_db.FindMark(name, GetCurrTxnId()));

    TxnRange range (markId, GetCurrTxnId());
    ReverseActions (range, true, true);
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

    TxnRange range (GetFirstTxnId(), GetCurrTxnId());
    ReverseActions (range, true, true);
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
        ApplyChanges(curr, APPLY_FOR_Redo);

    SetUndoInProgress(false);

    if (redoStr)
        {
        UInt64 source;
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
    IllegalTxnMark _v_v_v(*this);      // don't allow any recursion!

    Utf8String redoStr;
    ReinstateTxn (revTxn.m_range, &redoStr);     // do the actual redo now.

    T_HOST.GetTxnAdmin()._OnUndoRedoFinished (m_project, false);

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
    return  ReinstateActions (*revTxn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::GetUndoString (Utf8StringR string)
    {
    string.clear();
    if (!HasEntries())
        return;

    UInt64 source;
    m_db.ReadEntry (TxnId(GetCurrTxnId()-1), source, string);
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

    UInt64 source;
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
    RevTxn lastTxn = m_reversedTxn.front();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt UndoableTxn::_CheckDgnModelForWrite (DgnModelP dgnModel)
    {
    if (NULL == dgnModel)
        return DGNHANDLERS_STATUS_NoModel;

    // make sure they're not trying to write to a query model.
    if (!dgnModel->GetModelId().IsValid())
        return  DGNMODEL_STATUS_InvalidModel;

    return dgnModel->IsReadOnly() ? DGNHANDLERS_STATUS_FileReadonly : DGNHANDLERS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxn::ClearReversedTxns(DgnProjectR project)
    {
    if (m_opts.m_clearReversedTxns)
        project.GetTxnManager().ClearReversedTxns();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/08
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnAdmin::AddTxnMonitor (TxnMonitor& monitor)
    {
    BeAssert (!DgnPlatformLib::InStaticInitialization());// && "TxnMonitors are per thread, not per process");
    m_monitors.push_back (&monitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/08
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnAdmin::DropTxnMonitor (TxnMonitor& monitor)
    {
    auto it = std::find (m_monitors.begin(), m_monitors.end(), &monitor);
    if (it != m_monitors.end())
        *it = NULL; // removed from list by CallMonitors
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::DumpTxnHistory (int maxChanges)
    {
    toolSubsystem_printf ("\nTxn depth=%d, curr=%d", m_txnGroup.size(), m_currentTxnID.GetValue());
    toolSubsystem_printf ("\nTxn History:");

    Statement stmt;
    stmt.Prepare (m_db, "SELECT TxnId,Source,Descr FROM " UNDO_TABLE_Entry " ORDER BY TxnId DESC");
    while (BE_SQLITE_ROW == stmt.Step() && 0<maxChanges--)
        {
        toolSubsystem_printf ("\n{%4x, %lx} %hs", stmt.GetValueInt(0), stmt.GetValueInt(1), stmt.GetValueText(2));
        }

    toolSubsystem_printf ("\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
TxnTableHandler* ITxnManager::FindTableHandler(Utf8CP tableName)
    {
    auto it=s_tableHandlers.find(tableName);
    return it != s_tableHandlers.end() ? it->second : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ITxnManager::AddTableHandler (TxnTableHandler& tblHandler) {s_tableHandlers.Insert(tblHandler._GetTableName(), &tblHandler);}
void ITxnManager::SetTxnSource (UInt64 source) {m_txnSource = source;}
void ITxnManager::SetTxnDescription (Utf8CP descr) {m_txnDescr.assign (descr);}

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnReverseCaller
{
    TxnSummaryCR m_summary;
    bool m_isUndo;
    TxnReverseCaller(TxnSummaryCR summary, bool isUndo) : m_summary(summary), m_isUndo(isUndo) {}
    void operator() (TxnMonitorR monitor) const {monitor._OnTxnReverse(m_summary, m_isUndo);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnReversedCaller
{
    TxnSummaryCR m_summary;
    bool m_isUndo;
    TxnReversedCaller(TxnSummaryCR summary, bool isUndo) : m_summary(summary), m_isUndo(isUndo) {}
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
    DgnProjectR   m_project;
    bool m_isUndo;
    UndoRedoFinishedCaller (DgnProjectR project, bool isUndo) : m_project(project), m_isUndo(isUndo) {}
    void operator() (TxnMonitorR handler) const {handler._OnUndoRedoFinished (m_project, m_isUndo);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnAdmin::_OnTxnBoundary (TxnSummaryCR summary)
    {
    summary.CallHandlers_Boundary();
    CallMonitors (TxnBoundaryCaller(summary));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnAdmin::_OnTxnReverse (TxnSummaryCR summary, bool isUndo)
    {
    summary.CallHandlers_Reverse(isUndo);
    CallMonitors (TxnReverseCaller(summary, isUndo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnAdmin::_OnTxnReversed (TxnSummaryCR summary, bool isUndo)
    {
    summary.CallHandlers_Reversed(isUndo);
    CallMonitors (TxnReversedCaller(summary, isUndo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnAdmin::_OnUndoRedoFinished (DgnProjectR project, bool isUndo)
    {
    CallMonitors (UndoRedoFinishedCaller (project, isUndo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnSummary::CallHandlers_Boundary() const
    {
    DgnProjectR project =GetDgnProject();
    DgnDomains& domains = project.Domains();

    for (auto& add : m_added)
        {
        auto el = add.GetElement(project);
        if (el.IsValid())
            el->GetHandler()->_OnTxnBoundary_Add(*el);
        }
    for (auto& el : m_deleted)
        {
        HandlerP txnHandler = domains.FindElementHandler(el.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnBoundary_Delete(project, el.GetElementId());
        }

    for (auto el : m_modified)
        el->GetHandler()->_OnTxnBoundary_Modify(*el);

    for (auto& xatt : m_xattAdded)
        {
        XAttributeHandlerP txnHandler = domains.FindXAttributeHandler(xatt.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnBoundary_Add(*xatt.GetElement(project), xatt.GetXAttrId());
        }
    for (auto& xatt : m_xattDeleted)
        {
        XAttributeHandlerP txnHandler = domains.FindXAttributeHandler(xatt.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnBoundary_Delete(project, xatt.GetElementId(), xatt.GetXAttrId());
        }
    for (auto& xatt : m_xattModified)
        {
        XAttributeHandlerP txnHandler = domains.FindXAttributeHandler(xatt.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnBoundary_Modify(*xatt.GetElement(project), xatt.GetXAttrId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnSummary::CallHandlers_Reverse (bool isUndo) const
    {
    DgnProjectR project =GetDgnProject();
    DgnDomains& domains = project.Domains();

    for (auto& el : m_added)
        {
        HandlerP txnHandler = domains.FindElementHandler(el.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnReverse_Delete(project, el.GetElementId(), isUndo);
        }
    for (auto& it : m_deleted)
        {
        auto el = it.GetElement(project);
        if (el.IsValid())
            el->GetHandler()->_OnTxnReverse_Add(*el, isUndo);
        }

    for (auto& el : m_modified)
        {
        el->GetHandler()->_OnTxnReverse_Modify(*el, isUndo);
        }

    for (auto& xatt : m_xattAdded)
        {
        XAttributeHandlerP txnHandler = domains.FindXAttributeHandler(xatt.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnReverse_Add(*xatt.GetElement(project), xatt.GetXAttrId(), isUndo);
        }
    for (auto& xatt : m_xattDeleted)
        {
        XAttributeHandlerP txnHandler = domains.FindXAttributeHandler(xatt.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnReverse_Delete(project, xatt.GetElementId(), xatt.GetXAttrId(), isUndo);
        }
    for (auto& xatt : m_xattModified)
        {
        XAttributeHandlerP txnHandler = domains.FindXAttributeHandler(xatt.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnReverse_Modify(*xatt.GetElement(project), xatt.GetXAttrId(), isUndo);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnSummary::CallHandlers_Reversed (bool isUndo) const
    {
    DgnProjectR project =GetDgnProject();
    DgnDomains& domains = project.Domains();

    for (auto& add : m_added)
        {
        auto el = add.GetElement(project);
        if (el.IsValid())
            el->GetHandler()->_OnTxnReversed_Delete(*el, isUndo);
        }
    for (auto& el : m_deleted)
        {
        HandlerP txnHandler = domains.FindElementHandler(el.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnReversed_Add(project, el.GetElementId(), isUndo);
        }

    for (auto& el : m_modified)
        el->GetHandler()->_OnTxnReversed_Modify(*el, isUndo);

    for (auto& xatt : m_xattAdded)
        {
        XAttributeHandlerP txnHandler = domains.FindXAttributeHandler(xatt.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnReversed_Add(project, xatt.GetElementId(), xatt.GetXAttrId(), isUndo);
        }
    for (auto& xatt : m_xattDeleted)
        {
        XAttributeHandlerP txnHandler = domains.FindXAttributeHandler(xatt.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnReversed_Delete(*xatt.GetElement(project), xatt.GetXAttrId(), isUndo);
        }
    for (auto& xatt : m_xattModified)
        {
        XAttributeHandlerP txnHandler = domains.FindXAttributeHandler(xatt.GetHandlerId());
        if (NULL != txnHandler)
            txnHandler->_OnTxnReversed_Modify(*xatt.GetElement(project), xatt.GetXAttrId(), isUndo);
        }
    }

TxnSummary::TxnLifecycleSet const& TxnSummary::GetElementAdds() const {return m_added;}
TxnSummary::TxnLifecycleSet const& TxnSummary::GetElementDeletes() const {return m_deleted;}
TxnSummary::TxnModifySet const& TxnSummary::GetElementModifies() const {return m_modified;}
TxnSummary::TxnXAttrSet const& TxnSummary::GetXAttributeAdds() const {return m_xattAdded;}
TxnSummary::TxnXAttrSet const& TxnSummary::GetXAttributeDeletes() const {return m_xattDeleted;}
TxnSummary::TxnXAttrSet const& TxnSummary::GetXAttributeModifies() const {return m_xattModified;}
DRange3dCR TxnSummary::GetPhysicalRange() const {return m_physicalRange;}
DRange2dCR TxnSummary::GetDrawingRange() const {return m_drawingRange;}
TxnId TxnSummary::GetTxnId() const {return m_txnId;}
DgnProjectR TxnSummary::GetDgnProject() const {return m_project;}
Utf8StringCR TxnSummary::GetTxnDescription() const {return m_txnDescr;}
UInt64 TxnSummary::GetTxnSource() const {return m_txnSource;}
