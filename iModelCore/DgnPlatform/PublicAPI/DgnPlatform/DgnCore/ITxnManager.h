/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ITxnManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include    "DgnDb.h"

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

DGNPLATFORM_TYPEDEFS (TxnMonitor)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnElementDependencyGraph;

/*=================================================================================**//**
 @addtogroup TxnMgr
 <h1>Transaction Manager</h1>
 The Transaction Manager enables you to save changes, journal transactionable changes during a session, and to organize
 changes into units called transactions.

 <h2>Making and Saving Changes</h2>
 ITxn is the API for making and saving changes. See ITxnManager::GetCurrentTxn for how to get the current transaction.

 Use the Transaction Manager in order to add, save changes to, or to delete DgnModels and DgnElement instances and their contents.

 <h2>Transactions</h2>

 ITxnManager is the API for managing transactions.

 A transaction is defined as a set of changes that are treated as a unit. All changes and deletions are made as part of a transaction.
 Most transactions are undoable and can be reversed or reinstated as a unit. Undoable transactions support \ref TxnMonitor listeners.

 The Transaction Manager implements the ITxnManager interface. This interface has methods to query the current transaction, and to reverse, cancel, or reinstate transactions.

 All changes are made in the context of the current transaction. There is always a current transaction running. You don't have to start
 or create one. ITxnManager::GetCurrentTxn returns the current transaction.

 ITxnManager::CloseCurrentTxn defines a \em boundary that ends the current transaction and starts a new one.
 Setting a transaction boundary validates the preceding changes. Validating an undoable transaction also notifies \ref TxnMonitor listeners.
 For an undoable transaction, setting a transaction boundary defines an undo point.

 The current transaction implements the ITxn interface. The ITxn interface provides methods to write changes to elements, XAttributes, and models.
 The EditElementHandle methods to add, replace, and delete elements turn around and call the appropriate methods on the current transaction.

 @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class TxnDirection {Undo=0, Redo=1};

//=======================================================================================
//! A 32 bit value to identify the group of entries that form a single transaction.
// @bsiclass                                                      Keith.Bentley   02/04
//=======================================================================================
struct TxnId
{
    int32_t m_value;

public:
    TxnId ()  {m_value = -1;}
    explicit TxnId (int32_t val) {m_value = val;}
//__PUBLISH_SECTION_END__
    void Init() {m_value = 0;}
    void Next() {++m_value;}
    void Prev() {--m_value;}
//__PUBLISH_SECTION_START__
    bool IsValid() const {return -1 != m_value;}
    int32_t GetValue() const {return m_value;}
    operator int32_t() const {return m_value;}
};

//=======================================================================================
//! A summary of all of the Element-based changes that occurred during a Txn. TxnMonitors are supplied with a
//! TxnSummary so they can determine what Elements were affected by a Txn.
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnSummary
{
private:
    DgnDbR          m_dgndb;
    TxnId           m_txnId;
    Utf8String      m_txnDescr;
    uint64_t        m_txnSource;
    BeSQLite::CachedStatementPtr m_addElementStatement;
    BeSQLite::CachedStatementPtr m_addElementDepStatement;
    bool            m_modelDepsChanged;
    bool            m_elementDepsChanged;

public:
    bset<DgnElementId> m_deletedElements;
    bset<DgnElementId> m_addedElements;
    bset<DgnElementP>  m_updatedElements;
    bset<DgnModelId>   m_affectedModels;
    bset<DgnElementId> m_failedDependencyTargets;

    enum class ChangeType {Add, Mod, Delete};
    explicit TxnSummary(DgnElementDependencyGraph const&);
    TxnSummary(DgnDbR, TxnId, Utf8String, uint64_t, BeSQLite::ChangeSet&);
    ~TxnSummary();

    //! Table handler should call this function to record the fact that the specified Element was changed in the current txn.
    DGNPLATFORM_EXPORT void AddAffectedElement (DgnElementId const&, DgnModelId, ChangeType);

    //! Table handler should call this function to record the fact that the specified relationship with a dependency was changed in the current txn.
    DGNPLATFORM_EXPORT void AddAffectedDependency (BeSQLite::EC::ECInstanceId const&, ChangeType);

    void AddFailedDependencyTarget(DgnElementId eid) {m_failedDependencyTargets.insert(eid);}
    void SetModelDependencyChanges() {m_modelDepsChanged=true;}
    void SetElementDependencyChanges() {m_elementDepsChanged=true;}

    //! Get the name of the temp table that records all Elements that were modified, added, or deleted during this transaction.
    //! The temp table is laid out as follows: 
    //! * ElementId INTEGER NOT NULL PRIMARY KEY, 
    //! * ModelId INTEGER NOT NULL, 
    //! * Op CHAR
    //! Op is text that can have the value "+", "-", or "*", indicating add, delete, or replacement, respectively.
    DGNPLATFORM_EXPORT Utf8String GetChangedElementsTableName() const;

    //! Get the name of the temp table that records all ElementDrivesElements relationship instances that were modified, added, or deleted during this transaction.
    //! The temp table is laid out as follows: 
    //! * ECInstanceId INTEGER NOT NULL PRIMARY KEY, 
    //! * ModelId INTEGER NOT NULL, 
    //! * Op CHAR
    //! Op is text that can have the value "+", "-", or "*", indicating add, delete, or replacement, respectively.
    DGNPLATFORM_EXPORT Utf8String GetChangedElementDrivesElementRelationshipsTableName() const;

    //! Get the DgnDb for this TxnSummary
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Get the TxnId of this TxnSummary
    TxnId GetTxnId() const {return m_txnId;}

    //! Get the description of this TxnSummary
    //! @see ITxnManager::SetTxnDescription
    Utf8StringCR GetTxnDescription() const {return m_txnDescr;}

    //! Get the source for this TxnSummary
    //! @see ITxnManager::SetTxnSource
    uint64_t GetTxnSource() const {return m_txnSource;}

    //! Query if there are any changes to ModelDrivesModel ECRelationships
    bool HasModelDependencyChanges() const {return m_modelDepsChanged;}

    //! Query if there are any changes to ElementDrivesElement ECRelationships
    bool HasElementDependencyChanges() const {return m_elementDepsChanged;}
};

//=======================================================================================
//! Interface to be implemented to monitor changes to elements.
//! @ingroup TxnMgr
// @bsiclass                                                      Keith.Bentley   10/07
//=======================================================================================
struct TxnMonitor
{
    virtual void _OnTxnBoundary (TxnSummaryCR) = 0;
    virtual void _OnTxnReverse (TxnSummaryCR, TxnDirection isUndo) = 0;
    virtual void _OnTxnReversed (TxnSummaryCR, TxnDirection isUndo) = 0;
    virtual void _OnUndoRedoFinished (DgnDbR, TxnDirection isUndo) {}
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

#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
//=======================================================================================
// @bsiclass
//=======================================================================================
struct ITxn : NonCopyableClass
{
private:
    StatusInt CheckElementForWrite (DgnElementP);

protected:
    ITxnOptions m_opts;
    ITxn(ITxnOptions opts = ITxnOptions()) : m_opts (opts) {}
    virtual ~ITxn() {}

    void ClearReversedTxns (DgnDbR);
    virtual StatusInt _CheckDgnModelForWrite (DgnModelP) {return SUCCESS;}

public:
    /// @name Element I/O
    //@{

    //! Delete an element from a model.
    //! @param[in]          elem            The DgnElementP of the element to be deleted. Must be a valid existing element.
    //! @return SUCCESS if the element was deleted.
    DGNPLATFORM_EXPORT StatusInt DeleteElement (DgnElementP elem);

#if defined (NOT_NOW_WIP_REMOVE_ELEMENTHANDLE)
    //! Replace an existing element in a model with a different one.
    //! @param[in,out] el The element to be replaced.
    //! @return SUCCESS if the element was replaced and out is non-NULL.
    DGNPLATFORM_EXPORT StatusInt ReplaceElement (EditElementHandleR el);
#endif
    //@}
};
#endif

//__PUBLISH_SECTION_END__

#if !defined (DOCUMENTATION_GENERATOR)
//=======================================================================================
//! The first and last entry number that forms a single transaction.
// @bsiclass                                                      Keith.Bentley   02/04
//=======================================================================================
class TxnRange
{
    TxnId     m_first;
    TxnId     m_last;

public:
    TxnRange (TxnId first, TxnId last) : m_first(first), m_last(last) {}

    TxnId GetFirst () {return m_first;}
    TxnId GetLast () {return m_last;}
};

//=======================================================================================
//! To reinstate a reversed transaction, we need to know the first and last entry number.
// @bsiclass                                                      Keith.Bentley   02/04
//=======================================================================================
struct RevTxn
{
    TxnRange    m_range;
    bool        m_multiStep;
    RevTxn (TxnRange& range, bool multiStep) : m_range (range) {m_multiStep = multiStep;}
};
#endif

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct UndoDb : BeSQLite::Db
{
    BeSQLite::DbResult Open();
    void Empty();
    void TruncateChanges (TxnId id);
    BeSQLite::DbResult SaveEntry(TxnId id, uint64_t source, Utf8StringCR descr);
    BeSQLite::DbResult ReadEntry(TxnId id, uint64_t& source, Utf8StringR cmdName);
    BeSQLite::DbResult SaveChange(TxnId id, BeSQLite::ChangeSet& changeset);
    BeSQLite::DbResult SaveMark(TxnId id, Utf8CP);
    TxnId FindMark(Utf8StringR, TxnId before);

    struct ChangedFiles
        {
        UndoDb& m_db;
        TxnId   m_id;
        mutable BeSQLite::Statement m_sql;
        ChangedFiles (UndoDb& db, TxnId id) : m_db(db), m_id(id){}

        struct Entry : std::iterator<std::input_iterator_tag, Entry const>
            {
            friend struct  ChangedFiles;
        private:
            bool  m_isValid;
            BeSQLiteStatementP m_sql;
            Entry (BeSQLiteStatementP sql, bool isValid) {m_sql = sql; m_isValid = isValid;}

        public:
            bool IsValid() const {return m_isValid;}
            Entry& operator++() {m_isValid = (BeSQLite::BE_SQLITE_ROW == m_sql->Step()); return *this;}
            Entry const& operator* () const {return *this;}
            bool operator!=(Entry const& rhs) const {return (m_isValid != rhs.m_isValid);}
            bool operator==(Entry const& rhs) const {return (m_isValid == rhs.m_isValid);}

            void GetChangeSet(BeSQLite::ChangeSet& changeSet, TxnDirection) const;
            };

        typedef Entry const_iterator;
        const_iterator begin() const;
        const_iterator end() const {return Entry (&m_sql, false);}
        };
};

//__PUBLISH_SECTION_START__

//=======================================================================================
//! This class provides a transaction mechanism for handling changes to Elements in DgnDbs.
//! 
//! <h2>API Summary</h2>
//!  * #Activate @copydoc Activate
//!  * #CloseCurrentTxn @copydoc CloseCurrentTxn
//!  * #ReverseSingleTxn @copydoc ReverseSingleTxn
//!  * #CancelToPos @copydoc CancelToPos
//! 
//! @ingroup TxnMgr
// @bsiclass
//=======================================================================================
struct ITxnManager
{
    enum class ValidationErrorSeverity
        {
        Fatal,      //!< Validation could not be completed, and the transaction should be rolled back.
        Warning,    //!< Validation was completed. Consistency checks may have failed. The results should be reviewed.
        };

    //! Base class for validation errors. 
    struct IValidationError : IRefCounted
    {
      protected:
        virtual Utf8String _GetDescription() const = 0;
        virtual ValidationErrorSeverity _GetSeverity() const = 0;

      public:
        //! Return the severity of the error
        DGNPLATFORM_EXPORT ValidationErrorSeverity GetSeverity() const;

        //! Return a human-readable, localized description of the error
        DGNPLATFORM_EXPORT Utf8String GetDescription() const;
    };

    typedef RefCountedPtr<IValidationError> IValidationErrorPtr;

    //! A general purpose validation error
    struct ValidationError : RefCounted<IValidationError>
    {
        DEFINE_BENTLEY_NEW_DELETE_OPERATORS

        Utf8String m_description;
        ValidationErrorSeverity m_severity;

        virtual Utf8String _GetDescription() const {return m_description;}
        virtual ValidationErrorSeverity _GetSeverity() const {return m_severity;}
        
        ValidationError (ValidationErrorSeverity sev, Utf8StringCR desc) : m_severity(sev), m_description(desc) {;}
    };

//__PUBLISH_SECTION_END__

    friend struct TxnIter;
    friend struct ITxn;

protected:
    DgnDbR          m_dgndb;
    UndoDb          m_db;
    bvector<TxnId>  m_txnGroup;
    bvector<RevTxn> m_reversedTxn;
    Utf8String      m_txnDescr;
    uint64_t        m_txnSource;
    TxnId           m_firstTxn;
    TxnId           m_currentTxnID;
    bool            m_isActive;
    bool            m_boundaryMarked;
    bool            m_undoInProgress;
    bool            m_callRestartFunc;
    bool            m_inDynamics;
    bool            m_doChangePropagation;
    bvector<IValidationErrorPtr> m_validationErrors; //!< Validation errors detected on the last boundary check

    void SetActive (bool newValue) {m_isActive = newValue;}
    void SetUndoInProgress(bool);
    TxnId GetFirstTxnId () {return m_firstTxn;}
    void ReverseTxnRange (TxnRange& txnRange, Utf8StringP, bool);
    void CheckTxnBoundary ();
    void ReinstateTxn (TxnRange&, Utf8StringP redoStr);
    bool HasAnyChanges();
    void ApplyChanges (TxnId, TxnDirection);
    void CancelChanges (BeSQLite::ChangeSet&);
    void CancelChanges (TxnId txnId);
    StatusInt ReinstateActions (RevTxn& revTxn);
    bool PrepareForUndo();
    StatusInt ReverseActions (TxnRange& txnRange, bool multiStep, bool showMsg);
    enum class HowToCleanUpElements {CallApplied, CallCancelled};
    BeSQLite::DbResult ApplyChangeSetInternal (BeSQLite::ChangeSet& changeset, TxnId txnId, Utf8StringCR txnDescr, uint64_t txnSource, TxnDirection isUndo, HowToCleanUpElements);

    BentleyStatus ComputeIndirectChanges(BeSQLite::ChangeSet&);

public:
    void InitTempTables();

    void UpdateModelDependencyIndex();

    Utf8String GetChangedElementsTableName() const;

    Utf8String GetChangedElementDrivesElementRelationshipsTableName() const;

    DGNPLATFORM_EXPORT bool GetDoChangePropagation() const {return m_doChangePropagation;}
    DGNPLATFORM_EXPORT void SetDoChangePropagation (bool b) {m_doChangePropagation=b;}

    DGNPLATFORM_EXPORT BentleyStatus SaveUndoMark(Utf8CP name);
    DGNPLATFORM_EXPORT void GetUndoString (Utf8StringR);
    DGNPLATFORM_EXPORT void GetRedoString (Utf8StringR);

    DGNPLATFORM_EXPORT ITxnManager (DgnDbR);

    //! Apply a changeset and then clean up the screen, reload elements, refresh other cached data, and notify txn listeners.
    //! @param changeset the changeset to apply
    //! @param txnId     the transaction id to pass to monitors
    //! @param txnDescr  the transaction description to pass to monitors
    //! @param txnSource the transaction source to pass to monitors
    //! @param isUndo    the undo/redo flag to pass to monitors
    //! @return the result of calling changeset.ApplyChanges
    DGNPLATFORM_EXPORT BeSQLite::DbResult ApplyChangeSet(BeSQLite::ChangeSet& changeset, TxnId txnId, Utf8StringCR txnDescr, uint64_t txnSource, TxnDirection isUndo);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! @name The Current Transaction
    //@{

    //! End the current transaction, either committing or cancelling the changes made.
    //! If changes are to be discarded, then this function will call CancelToPos.
    //! If changes are to be retained, then this function will invoke TxnMonitor callbacks and then set a closing mark.
    //! The closing mark designates that all of the preceding changes are to be undone together as a single operation, and that future
    //! changes are to be undone separately.
    //! @remarks If there is a Transaction Group active, the effect of this method is only to validate the transaction, if requested.
    //! @remarks If this is an undoable transaction, then TxnMonitors are invoked before changes are committed.
    DGNPLATFORM_EXPORT void CloseCurrentTxn ();

    //! TxnMonitors may call this to report a validation error. If the severity of the validation error is set to ValidationErrorSeverity::Fatal, then the transaction will be cancelled.
    DGNPLATFORM_EXPORT void ReportValidationError (IValidationError&);

    //! Query the number of validation errors that were reported during the last boundary check.
    DGNPLATFORM_EXPORT size_t GetValidationErrorCount() const;

    //! Query the validation errors that were reported during the last boundary check.
    DGNPLATFORM_EXPORT bvector<IValidationErrorPtr> GetValidationErrors() const;

    //! Query if any Fatal validation errors were reported during the last boundary check.
    DGNPLATFORM_EXPORT bool HasAnyFatalValidationErrors() const;

    //@}

    //! @name Start and Stop Journaling Changes for Undo
    //@{
    //! Query if the Transaction Manager is currently active
    //! @return True if the Transaction Manager is currently active.
    //! @see Activate
    DGNPLATFORM_EXPORT bool IsActive();

    //! Turn on the Transaction Manager. After this, all changes to elements in transactable files will be journaled.
    //! @See DgnFile::SetTransactable
    //! @see Deactivate
    DGNPLATFORM_EXPORT void Activate();

    //! Turn off the Transaction Manager. After this, changes to the database will not be journaled for undo.
    //! Turning off the Transaction Manager also permanently clears any journaled changes. Therefore, turning the Transaction Manager
    //! off and then back on has the effect of clearing it.
    //! @see Activate
    DGNPLATFORM_EXPORT void Deactivate();
    //@}

    //! @name Transaction Groups
    //@{

    //! Start a new Transaction Group. Transaction Groups are used to cause changes that would normally
    //! be considered separate actions to be "grouped" into a single action. This means that when the user issues the "undo"
    //! command, the entire group of changes is undone as a single action. Groups can be nested, and until the outermost group is closed,
    //! all element changes constitute a single transaction.
    //! @param[in] startNewTxn if true, and if there is no current Transaction Group, and if there are pending changes, then start a new group. Otherwise
    //!                       changes from this group are included with the previous changes.
    //! @remarks This method should \e always be paired with a call to EndTxnGroup.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void StartTxnGroup (bool startNewTxn);

    //! Close the current Transaction Group.
    DGNPLATFORM_EXPORT void EndTxnGroup();

    //! Return the number of entries in the current Transaction Group.
    DGNPLATFORM_EXPORT size_t GetTxnGroupCount ();

    //! @return The TxnId of the beginning of the innermost Transaction Group. If no Transaction Group is active, the TxnId will be zero.
    DGNPLATFORM_EXPORT TxnId GetCurrGroupStart();
    //@}

    //! @name Reversing and Reinstating Transactions
    //@{

    //! Query if there are currently any reversible (undoable) changes in the Transaction Manager
    //! @return true if there are currently any reversible (undoable) changes in the Transaction Manager.
    DGNPLATFORM_EXPORT bool HasEntries ();

    //! Query if there are currently any reinstateable (redoable) changes in the Transaction Manager
    //! @return True if there are currently any reinstateable (redoable) changes in the Transaction Manager.
    DGNPLATFORM_EXPORT bool RedoIsPossible ();

    //! Reverse (undo) the most recent transaction(s).
    //! @param[in] numActions the number of transactions to reverse. If numActions is greater than 1, the entire set of transactions will
    //!       be reinstated together when/if ReinstateTxn is called (e.g., the user issues the "REDO" command.)
    //! @remarks  Reversed Transactions can be reinstated by calling ReinstateTxn. To completely remove all vestiges of (including the memory
    //!           used by) a transaction, call ClearReversedTxns.
    //! @see ReinstateTxn ClearReversedTxns
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void ReverseTxns (int numActions);

    //! Reverse (undo) the most recent transaction.
    //! @param[in] callRestartFunc whether to restart the current tool afterwards, only the current tool should pass false.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void ReverseSingleTxn (bool callRestartFunc=true);

    //! Reverse all element changes back to the most recent Mark. Marks are created by calling SaveUndoMark.
    //! @param[out] name of mark undone.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void ReverseToMark (Utf8StringR name);

    //! Reverse all element changes back to the beginning of the session.
    //! @param[in] prompt display a dialog warning the user of the severity of this action and giving an opportunity to cancel.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void ReverseAll (bool prompt);

    //! Reverse all element changes back to a previously saved TxnPos.
    //! @param[in] pos a TxnPos obtained from a previous call to GetCurrTxnPos.
    //! @return SUCCESS if the transactions were reversed, ERROR if TxnPos is invalid.
    //! @remarks This method cancels all pending nested transactions.
    //! @see  GetCurrTxnPos CancelToPos
    DGNPLATFORM_EXPORT StatusInt ReverseToPos (TxnId pos);

    //! Get the Id of the most recently commited transaction.
    //! @return the current TxnPos. This value can be saved and later used to reverse changes that happen after this time.
    //! @see   ReverseToPos CancelToPos
    DGNPLATFORM_EXPORT TxnId GetCurrTxnId();

    //! Reverse and then cancel all element changes back to a previously saved TxnPos. This method is
    //! the same as calling ReverseToPos followed (on successful return) by ClearReversedTxns, and is provided as a convenience.
    //! @param[in] pos a TxnPos obtained from a previous call to GetCurrTxnPos.
    //! @param[in] callRestartFunc whether to restart the current tool afterwards, only the current tool should pass false.
    //! @return SUCCESS if the transactions were reversed and cleared, ERROR if TxnPos is invalid.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT StatusInt CancelToPos (TxnId pos, bool callRestartFunc=true);

    //! Clear vestiges of any reversed transactions from memory. Reversed transactions can be reinstated. Therefore, they still
    //! occupy memory in the cache and in the transaction buffer. This method clears them and makes them non-reinstateable.
    //! @remarks If any transactions are reversed, the Transaction Manager will automatically call this method before any new element
    //!          changes can be journaled. That is, after a reverse, ClearReversedTxns is implied if anything other than Reinstate happens.
    DGNPLATFORM_EXPORT void ClearReversedTxns ();

    //! Reinstate the most recent previously applied and then reversed transaction. Since at any time multiple transactions can be reversed, it
    //! may take multiple calls to this method to reinstate all reversed operations.
    //! @return SUCCESS if a reversed transaction was reinstated, ERROR if no transactions were reversed.
    DGNPLATFORM_EXPORT StatusInt ReinstateTxn ();

    //! Query if undo/redo is in progress
    DGNPLATFORM_EXPORT bool IsUndoInProgress ();

    //! Set value for the "TxnSource" to be saved in undo
    DGNPLATFORM_EXPORT void SetTxnSource (uint64_t souce);

    //! Set description of current txn. Used to show what will be undone/redone.
    DGNPLATFORM_EXPORT void SetTxnDescription (Utf8CP descr);

    //! Get the DgnDb of this ITxnManager
    DGNPLATFORM_EXPORT DgnDbR GetDgnDb();
    //@}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnAdmin : DgnHost::IHostObject
    {
//__PUBLISH_SECTION_END__
    typedef bvector<TxnMonitorP> TxnMonitors;
protected:
    TxnMonitors m_monitors;

    template <typename CALLER> void CallMonitors (CALLER const& caller)
        {
        try {
            for (auto curr = m_monitors.begin(); curr!=m_monitors.end(); )
                {
                if (*curr == NULL)
                    curr = m_monitors.erase(curr);
                else
                    {caller (**curr); ++curr;}
                }
            }
        catch (...) {}
        }

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS
    
    virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}
    virtual bool _OnPromptReverseAll() {return true;}
    virtual void _RestartTool()  {}
    virtual void _OnNothingToUndo() {}
    virtual void _OnPrepareForUndoRedo(){}
    virtual void _OnNothingToRedo(){}

    DGNPLATFORM_EXPORT virtual void _OnTxnBoundary (TxnSummaryCR);
    DGNPLATFORM_EXPORT virtual void _OnTxnReverse (TxnSummaryCR, TxnDirection isUndo);
    DGNPLATFORM_EXPORT virtual void _OnTxnReversed (TxnSummaryCR, TxnDirection isUndo);
    DGNPLATFORM_EXPORT virtual void _OnUndoRedoFinished (DgnDbR, TxnDirection isUndo);
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
    //! @name Transaction Monitors
    //@{
    //! Add a TxnMonitor. The monitor will be notified of all transaction events until it is dropped.
    //! @param monitor a monitor to add
    DGNPLATFORM_EXPORT void AddTxnMonitor (TxnMonitor& monitor);

    //! Drop a TxnMonitor.
    //! @param monitor a monitor to drop
    DGNPLATFORM_EXPORT void DropTxnMonitor (TxnMonitor& monitor);
    //@}

    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
