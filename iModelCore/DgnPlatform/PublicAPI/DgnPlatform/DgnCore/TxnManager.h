/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/TxnManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include    "DgnDb.h"

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

DGNPLATFORM_TYPEDEFS(TxnMonitor)

//  temp table names used by TxnSummary and ElementGraphTxnMonitor
#define TXN_TABLE_PREFIX   "txn_"

#define TEMP_TABLE(name)    "temp." name
#define TXN_TABLE(name)      TXN_TABLE_PREFIX name

#define TXN_TABLE_Elements   TXN_TABLE("Elements")
#define TXN_TABLE_Depend     TXN_TABLE("Depend")

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnElementDependencyGraph;

/*=================================================================================**//**
 @addtogroup TxnMgr
 <h1>Transaction Manager</h1>
 The TxnManager API manages Txns. A Txn is a named, committed, undoable unit of work, stored in the DgnDb as a changeset.

 @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class TxnDirection {Backwards=0, Forward=1};

//=======================================================================================
//! A 32 bit value to identify the group of entries that form a single transaction.
// @bsiclass                                                      Keith.Bentley   02/04
//=======================================================================================
struct TxnId
{
    int32_t m_value;

public:
    TxnId()  {m_value = -1;}
    explicit TxnId(int32_t val) {m_value = val;}
    void Init() {m_value = 0;}
    void Next() {++m_value;}
    void Prev() {--m_value;}
    bool IsValid() const {return -1 != m_value;}
    int32_t GetValue() const {return m_value;}
    operator int32_t() const {return m_value;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct TxnRowId
{
    int64_t m_value;
    
public:
    TxnRowId() {m_value = -1;}
    explicit TxnRowId(int64_t val) {m_value = val;}
    void Next() {++m_value;}
    void Prev() {--m_value;}
    bool IsValid() const {return -1 != m_value;}
    int64_t GetValue() const {return m_value;}
    operator int64_t() const {return m_value;}
};

//=======================================================================================
//! A summary of all of the Element-based changes that occurred during a Txn. TxnMonitors are supplied with a
//! TxnSummary so they can determine what Elements were affected by a Txn.
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnSummary
{
    //! Indicates an error when propagating changes for a Txn.
    struct ValidationError
    {
        enum class Severity
        {
            Fatal,   //!< Validation could not be completed, and the transaction should be rolled back.
            Warning, //!< Validation was completed. Consistency checks may have failed. The results should be reviewed.
        };

        Utf8String  m_description;
        Severity    m_severity;
        ValidationError(Severity sev, Utf8CP desc) : m_severity(sev), m_description(desc) {}
        Severity GetSeverity() const {return m_severity;} //!< Return the severity of the error
        Utf8CP GetDescription() const {return m_description.c_str();}//!< A human-readable, localized description of the error
    };

private:
    DgnDbR m_dgndb;
    BeSQLite::CachedStatementPtr m_elementStmt;
    BeSQLite::CachedStatementPtr m_dependencyStmt;
    TxnDirection m_direction;
    bool m_modelDepsChanged;
    bool m_elementDepsChanged;
    bvector<ValidationError> m_validationErrors; //!< Validation errors detected on the last boundary check
    DgnModelIdSet m_modelsInTxn;
    DgnElementIdSet m_failedTargets;

public:
    enum class ChangeType : int {Insert, Update, Delete};

    struct ElementIterator : BeSQLite::DbTableIterator
    {
    public:
        ElementIterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR)db) {}
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct ElementIterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}

        public:
            DGNPLATFORM_EXPORT DgnModelId GetModelId() const;
            DGNPLATFORM_EXPORT DgnElementId GetElementId() const;
            DGNPLATFORM_EXPORT ChangeType GetChangeType() const;
            DGNPLATFORM_EXPORT double GetLastMod() const;
            Entry const& operator*() const {return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry(nullptr, false);}
    };

    DGNPLATFORM_EXPORT explicit TxnSummary(DgnDbR db, TxnDirection);
    DGNPLATFORM_EXPORT ~TxnSummary();
    void AddChangeSet(BeSQLite::ChangeSet&);

    //! Table handler should call this function to record the fact that the specified Element was changed in the current txn.
    DGNPLATFORM_EXPORT void AddAffectedElement(DgnElementId const&, DgnModelId, double lastMod, ChangeType);

    //! Table handler should call this function to record the fact that the specified relationship with a dependency was changed in the current txn.
    DGNPLATFORM_EXPORT void AddAffectedDependency(BeSQLite::EC::ECInstanceId const&, ChangeType);

    void AddFailedTarget(DgnElementId eid) {m_failedTargets.insert(eid);}
    void SetModelDependencyChanges() {m_modelDepsChanged=true;}
    void SetElementDependencyChanges() {m_elementDepsChanged=true;}
    void UpdateModelDependencyIndex();
    DgnModelIdSet const& GetModelSet() const {return m_modelsInTxn;}
    DgnElementIdSet const& GetFailedTargets() const {return m_failedTargets;}

    //! Get the DgnDb for this TxnSummary
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Query if there are any changes to ModelDrivesModel ECRelationships
    bool HasModelDependencyChanges() const {return m_modelDepsChanged;}

    //! Query if there are any changes to ElementDrivesElement ECRelationships
    bool HasElementDependencyChanges() const {return m_elementDepsChanged;}

    //! TxnMonitors may call this to report a validation error. If the severity of the validation error is set to ValidationErrorSeverity::Fatal, 
    //! then the transaction will be cancelled.
    DGNPLATFORM_EXPORT void ReportError(ValidationError&);

    //! Query the validation errors that were reported during the last boundary check.
    bvector<ValidationError> const& GetErrors() const {return m_validationErrors;}

    //! Query if any Fatal validation errors were reported during the last boundary check.
    DGNPLATFORM_EXPORT bool HasFatalErrors() const;

    ElementIterator MakeElementIterator() const {return ElementIterator(m_dgndb);}
};

//=======================================================================================
//! Interface to be implemented to monitor changes to elements.
//! @ingroup TxnMgr
// @bsiclass                                                      Keith.Bentley   10/07
//=======================================================================================
struct TxnMonitor
{
    virtual void _OnTxnCommit(TxnSummaryCR) = 0;
    virtual void _OnTxnReverse(TxnSummaryCR) = 0;
    virtual void _OnTxnReversed(TxnSummaryCR) = 0;
    virtual void _OnUndoRedoFinished(DgnDbR, TxnDirection isUndo) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct ITxnOptions
    {
    bool m_clearReversedTxns;
    bool m_writesIllegal;

    ITxnOptions()
        {
        m_clearReversedTxns = true;
        m_writesIllegal = false;
        }
    };

//=======================================================================================
//! The first and last entry number that forms a single transaction.
//! @private
// @bsiclass                                                      Keith.Bentley   02/04
//=======================================================================================
class TxnRange
{
    TxnId m_first;
    TxnId m_last;

public:
    TxnRange(TxnId first, TxnId last) : m_first(first), m_last(last) {}

    TxnId GetFirst() {return m_first;}
    TxnId GetLast() {return m_last;}
};

//=======================================================================================
//! To reinstate a reversed transaction, we need to know the first and last entry number.
//! @private
// @bsiclass                                                      Keith.Bentley   02/04
//=======================================================================================
struct RevTxn
{
    TxnRange    m_range;
    bool        m_multiStep;
    RevTxn(TxnRange& range, bool multiStep) : m_range(range) {m_multiStep = multiStep;}
};

//=======================================================================================
//! This class implements the DgnDb::Txns()
//!    - Reversing (undo) and Reinstating (redo) Txns
//!    - change propagation.
//!    - combining multi-step Txns into a single reversible "operation"
// @bsiclass
//=======================================================================================
struct TxnManager : BeSQLite::ChangeTracker
{
    struct ChangeEntry
    {
        BeGuid          m_sessionId;
        TxnId           m_txnId;
        bool            m_settingsChange;
        Utf8String      m_description;
        Utf8String      m_mark;
    };

    struct UndoChangeSet : BeSQLite::ChangeSet
    {
        virtual ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override
            {
            BeAssert(false);
            return ConflictResolution::Skip;
            }
    };

protected:
    DgnDbR          m_dgndb;
    ChangeEntry     m_curr;
    bvector<TxnId>  m_multiTxnOp;
    bvector<RevTxn> m_reversedTxn;
    bool            m_undoInProgress;
    bool            m_inDynamics;
    bool            m_propagateChanges;
    BeSQLite::StatementCache    m_stmts;
    BeSQLite::SnappyFromBlob    m_snappyFrom;
    BeSQLite::SnappyToBlob      m_snappyTo;

    BeSQLite::CachedStatementPtr GetTxnStatement(Utf8CP sql) const;
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override;
    void _OnSettingsSave() override {m_curr.m_settingsChange=true;}
    void _OnSettingsSaved() override {m_curr.m_settingsChange=false;}
    TrackChangesForTable _FilterTable(Utf8CP tableName) override;

    BeSQLite::DbResult SaveCurrentChange(BeSQLite::ChangeSet& changeset, Utf8CP operation);
    BeSQLite::DbResult ReadEntry(TxnRowId rowid, ChangeEntry& entry);
    void ReadChangeSet(UndoChangeSet&, TxnRowId rowid, TxnDirection direction);
    TxnRowId FirstRow(TxnId id);
    TxnRowId LastRow(TxnId id);

    void TruncateChanges(TxnId id);
    void SetUndoInProgress(bool);
    void ReverseTxnRange(TxnRange& txnRange, Utf8StringP, bool);
    void ReinstateTxn(TxnRange&, Utf8StringP redoStr);
    void ApplyChanges(TxnRowId, TxnDirection);
    void CancelChanges(BeSQLite::ChangeSet&);
    void CancelChanges(TxnRowId );
    StatusInt ReinstateActions(RevTxn& revTxn);
    bool PrepareForUndo();
    StatusInt ReverseActions(TxnRange& txnRange, bool multiStep, bool showMsg);
    BentleyStatus PropagateChanges(TxnSummaryR summary);
    void DeleteReversedTxns();

public:

    void SetPropagateChanges(bool val) {m_propagateChanges = val;}
    DGNPLATFORM_EXPORT BentleyStatus SaveUndoMark(Utf8CP name);
    DGNPLATFORM_EXPORT Utf8String GetUndoString();
    DGNPLATFORM_EXPORT Utf8String GetRedoString();

    DGNPLATFORM_EXPORT TxnManager(DgnDbR);

    //! Apply a changeset and then clean up the screen, reload elements, refresh other cached data, and notify txn listeners.
    //! @param changeset the changeset to apply
    //! @param isUndo    the undo/redo flag to pass to monitors
    //! @return the result of calling changeset.ApplyChanges
    DGNPLATFORM_EXPORT BeSQLite::DbResult ApplyChangeSet(BeSQLite::ChangeSet& changeset, TxnDirection isUndo);

    //! @name Multi-transaction Operations
    //@{
    //! Begin a new multi-transaction operation. This can be used to cause a series of transactions, that would normally
    //! be considered separate actions for undo, to be "grouped" into a single undoable operation. This means that when the user issues the "undo"
    //! command, the entire group of changes is undone as a single action. Multi Txn operations can be nested, and until the outermost operation is closed,
    //! all changes constitute a single transaction.
    //! @remarks This method should \e always be paired with a call to EndMultiTxnOperation.
    DGNPLATFORM_EXPORT void BeginMultiTxnOperation();

    //! End a multi-transaction operation
    DGNPLATFORM_EXPORT void EndMultiTxnOperation();

    //! Return the depth of the mulit-transaction stack
    size_t GetMultiTxnOperationDepth() {return m_multiTxnOp.size();}

    //! @return The TxnId of the the innermost multi-Transaction operation. If no multi-Transaction operation is active, the TxnId will be zero.
    DGNPLATFORM_EXPORT TxnId GetMultiTxnOperationStart();
    //@}

    //! @name Reversing and Reinstating Transactions
    //@{
    //! Query if there are currently any reversible (undoable) changes in the Transaction Manager
    //! @return true if there are currently any reversible (undoable) changes in the Transaction Manager.
    bool IsUndoPossible() {return 0 < GetCurrTxnId();}

    //! Query if there are currently any reinstateable (redoable) changes in the Transaction Manager
    //! @return True if there are currently any reinstateable (redoable) changes in the Transaction Manager.
    bool IsRedoPossible() {return !m_reversedTxn.empty();}

    //! Reverse (undo) the most recent transaction(s).
    //! @param[in] numActions the number of transactions to reverse. If numActions is greater than 1, the entire set of transactions will
    //!       be reinstated together when/if ReinstateTxn is called (e.g., the user issues the "REDO" command.)
    //! @remarks  Reversed Transactions can be reinstated by calling ReinstateTxn. To completely remove all vestiges of (including the memory
    //!           used by) a transaction, call ClearReversedTxns.
    //! @see ReinstateTxn ClearReversedTxns
    DGNPLATFORM_EXPORT StatusInt ReverseTxns(int numActions);

    //! Reverse (undo) the most recent transaction.
    StatusInt ReverseSingleTxn() {return ReverseTxns(1);}

    //! Reverse all element changes back to the most recent Mark. Marks are created by calling SaveUndoMark.
    //! @param[out] name of mark undone.
    DGNPLATFORM_EXPORT void ReverseToMark(Utf8StringR name);

    //! Reverse all element changes back to the beginning of the session.
    //! @param[in] prompt display a dialog warning the user of the severity of this action and giving an opportunity to cancel.
    DGNPLATFORM_EXPORT void ReverseAll(bool prompt);

    //! Reverse all element changes back to a previously saved TxnPos.
    //! @param[in] pos a TxnPos obtained from a previous call to GetCurrTxnPos.
    //! @return SUCCESS if the transactions were reversed, ERROR if TxnPos is invalid.
    //! @see  GetCurrTxnPos CancelToPos
    DGNPLATFORM_EXPORT StatusInt ReverseToPos(TxnId pos);

    //! Get the Id of the most recently commited transaction.
    //! @return the current TxnPos. This value can be saved and later used to reverse changes that happen after this time.
    //! @see   ReverseToPos CancelToPos
    TxnId GetCurrTxnId() {return m_curr.m_txnId;}

    //! Reverse and then cancel (make non-reinstatable) all element changes back to a previous TxnPos. 
    //! @param[in] pos a TxnPos obtained from a previous call to GetCurrTxnPos.
    //! @return SUCCESS if the transactions were reversed and cleared, ERROR if TxnPos is invalid.
    DGNPLATFORM_EXPORT StatusInt CancelToPos(TxnId pos);

    //! Reinstate the most recently reversed transaction. Since at any time multiple transactions can be reversed, it
    //! may take multiple calls to this method to reinstate all reversed operations.
    //! @return SUCCESS if a reversed transaction was reinstated, ERROR if no Txns were reversed.
    DGNPLATFORM_EXPORT StatusInt ReinstateTxn();

    //! Query if undo/redo is in progress
    bool IsUndoInProgress() {return m_undoInProgress;}
    //@}

    //! Get the DgnDb of this Txns
    DgnDbR GetDgnDb() {return m_dgndb;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
