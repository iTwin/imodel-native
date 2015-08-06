/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/TxnManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"

DGNPLATFORM_TYPEDEFS(TxnMonitor)

#define TXN_TABLE_PREFIX "txn_"
#define TXN_TABLE(name)  TXN_TABLE_PREFIX name
#define TXN_TABLE_Elements TXN_TABLE("Elements")
#define TXN_TABLE_Depend   TXN_TABLE("Depend")
#define TXN_TABLE_Models   TXN_TABLE("Models")

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
 @addtogroup TxnMgr
<h1>Transaction Manager</h1>
 The TxnManager API manages Txns. A Txn is a named, committed, undoable unit of work, stored in the DgnDb as a changeset.
 Txns may be "reversed" via an application Undo command, or "reinstated" via a corresponding Redo command.
<p>
<h2>Sessions</h2>
 Every time the TxnManager is initialized, it creates a GUID for itself called a SessionId.
 Whenever an application calls DgnDb::SaveChanges(), a new Txn is created. Txns are saved in Briefcases in the DGN_TABLE_Txns
 table, along with the current SessionId. Only Txns from the current SessionId are (usually) undoable. After the completion of a
 session, all of the Txns for that SessionId may be merged together to form a "session Txn". Further, all of the session Txns
 since a Briefcase was last committed to a server may be merged together to form a Briefcase Txn. Briefcase Txns are sent between
 users and changed-merged.
 @bsiclass
+===============+===============+===============+===============+===============+======*/

//! Actions that cause events to be sent to TxnTables.
enum class TxnAction
{
    None = 0,   //!< not currently processing anything
    Commit,     //!< processing a Commit. Triggered by a call to DgnDb::SaveChanges
    Abandon,    //!< abandoning the current Txn. Triggered by a call to DgnDb::AbandonChanges
    Reverse,    //!< reversing a previously committed ChangeSet. Triggered by a call to TxnManager::ReverseActions
    Reinstate,  //!< reinstating a previously reversed ChangeSet. Triggered by a call to TxnManager::ReinstateActions
    Merge       //!< merging a ChangeSet made by a foreign repository.
};

//=======================================================================================
//! Interface to be implemented to monitor changes to a DgnDb.
//! @ingroup TxnMgr
// @bsiclass                                                      Keith.Bentley   10/07
//=======================================================================================
struct TxnMonitor
{
    virtual void _OnCommit(TxnManager&) {}
    virtual void _OnReversedChanges(TxnManager&) {}
    virtual void _OnUndoRedo(TxnManager&, TxnAction) {}
};

namespace dgn_TxnTable {struct Element; struct ElementDep;}

//=======================================================================================
//! An instance of a TxnTable is created for a single SQLite table of a DgnDb via a DgnDomain::TableHandler.
//! A TxnTable's sole role is to synchronize in-memory objects with persistent changes to the database through Txns.
//! That is, the TxnTable "monitors" changes to the rows in a SQLite database table for the purpose of ensuring
//! that in-memory copies of the data from its table reflect the current state on disk. The TxnTable itself has no
//! role in making or reversing changes to its table. Instead, the TxnManager orchestrates Txn commits, undo, redo
//! and change merging operations and informs the TxnTable of what happened.
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct TxnTable : RefCountedBase
{
    enum class ChangeType : int {Insert, Update, Delete};
    TxnManager& m_txnMgr;
    TxnTable(TxnManager& mgr) : m_txnMgr(mgr) {}

    //! Return the name of the table handled by this TxnTable.
    virtual Utf8CP _GetTableName() const = 0;

    //! @name Validating Direct Changes
    //@{
    //! Called before a Txn is committed (or explicitly to propagate changes within a Txn).
    //! Some TxnTable implementations use temporary tables to hold the full set of changes in a Txn in memory
    //! for change propagation. This method can be used to create those temporary tables.
    //! After this method is called one or more _OnValidatexxx methods will be called.
    virtual void _OnValidate() {}

    //! Called for every newly added row of this table in the Txn being validated.
    //! @param[in] change The data for the newly added row. All data will be in the "new values" of change.
    virtual void _OnValidateAdd(BeSQLite::Changes::Change const& change) {}

    //! Called for every deleted row of this table in the Txn being validated.
    //! @param[in] change The data for the deleted row. The data about the row that was deleted will be in the
    //! "old values" of change.
    virtual void _OnValidateDelete(BeSQLite::Changes::Change const& change) {}

    //! Called for every updated row of this table in the Txn being validated.
    //! @param[in] change The data for the updated row. The pre-changed data about the row will be in the
    //! "old values" of change, and the post-changed data will be in the "new values". Columns that are unchanged are in
    //! neither values.
    virtual void _OnValidateUpdate(BeSQLite::Changes::Change const& change) {}

    //! Called after all added/deleted/updated rows have been sent to the _OnValidatexxx methods to propagate changes to dependents.
    //! This is the only method on TxnTable that may make changes to the database.
    virtual void _PropagateChanges() {}

    //! Called after validation is complete. TxnTables that create temporary tables can empty them in this method.
    virtual void _OnValidated() {}
    //@}

    //! @name Reversing previously committed changesets via undo/redo.
    //@{
    //! Called when an add of a row in this TxnTable was reversed via undo or redo.
    //! @param[in] change The data for a previously added row that is now deleted. All data will be in the "old values" of change.
    //! @note If you wish to determine whether the action that caused this call was an undo or a redo, call m_txnMgr.GetCurrentAction()
    virtual void _OnReversedAdd(BeSQLite::Changes::Change const& change) {}

    //! Called when a delete of a row in this TxnTable was reversed via undo or redo.
    //! @param[in] change The data for a previously deleted row that is now back. All data will be in the "new values" of change.
    virtual void _OnReversedDelete(BeSQLite::Changes::Change const& change) {}

    //! Called when a delete of a row in this TxnTable was reversed via undo or redo.
    //! @param[in] change The data for a previously deleted row that is now back in place. The pre-changed data about the
    //! row will be in the "old values" of change, and the post-changed data will be in the "new values".
    //! Columns that are unchanged are in neither values.
    virtual void _OnReversedUpdate(BeSQLite::Changes::Change const& change) {}
    //@}
};
typedef RefCountedPtr<TxnTable> TxnTablePtr;

//=======================================================================================
//! This class implements the DgnDb::Txns() object. It is a BeSQLite::ChangeTracker.
//! It handles:
//!    - validating Txns on Db::SaveChanges
//!    - notifying TxnTables of changes
//!    - change propagation.
//!    - Reversing (undo) and Reinstating (redo) Txns
//!    - combining multi-step Txns into a single reversible "operation". See BeginMultiTxnOperation
//!    - change merging
// @bsiclass
//=======================================================================================
struct TxnManager : BeSQLite::ChangeTracker
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

    //=======================================================================================
    //! An identifier stored in the high 4 bytes of a TxnId. All TxnIds for a given session will have the same SessionId.
    //! Every time the TxnManager is initialized against a DgnDb, the SessionId is incremented to be one greater than the
    //! previous highest SessionId.
    // @bsiclass                                                    Keith.Bentley   08/15
    //=======================================================================================
    struct SessionId
    {
    private:
        uint32_t m_value;
    public:
        explicit SessionId(uint32_t val=(uint32_t) -1) : m_value(val) {}
        uint32_t GetValue() const {return m_value;}
        bool IsValid() const {return ((uint32_t) -1) != m_value;}
    };

    //=======================================================================================
    //! The primary key for the DGN_TABLE_Txns table. The value is comprised of two parts:
    //! the SessionId stored in the high 4 bytes and a counter in the low 4 bytes. Later Txns stored in the
    //! table will always have higher TxnIds than earlier ones, though the values of TxnId may not be sequential.
    //! The methods TxnManager::QueryNextTxnId and TxnManager::QueryPreviousTxnId may be used to iterate forward
    //! and backwards through committed Txns in the table.
    // @bsiclass                                                    Keith.Bentley   08/15
    //=======================================================================================
    struct TxnId
    {
        friend struct TxnManager;
    private:
        union{uint64_t m_64; uint32_t m_32[2];} m_id;
        void Increment() {++m_id.m_32[0];}

    public:
        TxnId(uint64_t val=(uint64_t) -1) {m_id.m_64 = val;}
        TxnId(SessionId session, uint32_t txn) {m_id.m_32[1]=session.GetValue(); m_id.m_32[0]=txn;}
        uint64_t GetValue() const {return m_id.m_64;}
        SessionId GetSession() const {return SessionId(m_id.m_32[1]);}//!< Get the SessionId from this TxnId
        bool IsValid() const {return GetSession().IsValid();}
        bool operator==(TxnId const& rhs) const {return m_id.m_64==rhs.m_id.m_64;}
        bool operator<(TxnId const& rhs) const  {return m_id.m_64<rhs.m_id.m_64;}
        bool operator<=(TxnId const& rhs) const {return m_id.m_64<=rhs.m_id.m_64;}
        bool operator>(TxnId const& rhs) const  {return m_id.m_64>rhs.m_id.m_64;}
        bool operator>=(TxnId const& rhs) const {return m_id.m_64>=rhs.m_id.m_64;}
    };

private:
    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   08/15
    //=======================================================================================
    struct TxnRange
    {
    private:
        TxnId m_first;
        TxnId m_last;
    public:
        TxnRange(TxnId first, TxnId last) : m_first(first), m_last(last) {}
        TxnId GetFirst() {return m_first;}
        TxnId GetLast() {return m_last;}
    };

    struct UndoChangeSet : BeSQLite::ChangeSet
    {
        ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override;
    };

    struct CompareTableNames {bool operator()(Utf8CP a, Utf8CP b) const {return strcmp(a, b) < 0;}};
    typedef bmap<Utf8CP,TxnTablePtr,CompareTableNames> T_TxnTablesByName;
    typedef bvector<TxnTable*> T_TxnTables;

    DgnDbR          m_dgndb;
    T_TxnTablesByName m_tablesByName;
    T_TxnTables     m_tables;
    TxnId           m_curr;
    TxnAction       m_action;
    bvector<TxnId> m_multiTxnOp;
    bvector<TxnRange> m_reversedTxn;
    bool            m_inDynamics;
    bool            m_propagateChanges;
    BeSQLite::StatementCache m_stmts;
    BeSQLite::SnappyFromBlob m_snappyFrom;
    BeSQLite::SnappyToBlob   m_snappyTo;
    bvector<ValidationError> m_validationErrors;

    void AddChangeSet(BeSQLite::ChangeSet&);
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override;
    TrackChangesForTable _FilterTable(Utf8CP tableName) override;
    BeSQLite::DbResult SaveCurrentChange(BeSQLite::ChangeSet& changeset, Utf8CP operation);
    void ReadChangeSet(UndoChangeSet&, TxnId rowid, TxnAction);
    void ReverseTxnRange(TxnRange& txnRange, Utf8StringP);
    void ReinstateTxn(TxnRange&, Utf8StringP redoStr);
    void ApplyChanges(TxnId, TxnAction);
    void OnChangesetApplied(BeSQLite::ChangeSet& changeset, TxnAction);
    OnCommitStatus CancelChanges(BeSQLite::ChangeSet& changeset);
    DgnDbStatus ReinstateActions(TxnRange& revTxn);
    bool PrepareForUndo();
    DgnDbStatus ReverseActions(TxnRange& txnRange, bool showMsg);
    BentleyStatus PropagateChanges();
    void DeleteReversedTxns();
    TxnTable* FindTxnTable(Utf8CP tableName) const;
    BeSQLite::DbResult ApplyChangeSet(BeSQLite::ChangeSet& changeset, TxnAction isUndo);
    bool IsMultiTxnMember(TxnId rowid);

public:
    void OnBeginValidate(); //!< @private
    void OnEndValidate(); //!< @private
    void AddTxnTable(DgnDomain::TableHandler*);//!< @private
    DGNPLATFORM_EXPORT TxnManager(DgnDbR); //!< @private

    //! A statement cache exclusively for Txn-based statements.
    BeSQLite::CachedStatementPtr GetTxnStatement(Utf8CP sql) const;

    //! @name Validation Errors
    //@{
    //! Query if any Fatal validation errors were reported during change propagation.
    //! @note this method may only be called from within TxnMonitor::_OnCommit methods.
    DGNPLATFORM_EXPORT bool HasFatalErrors() const;

    //! Query the validation errors that were reported during change propagation
    //! @note this method may only be called from within TxnMonitor::_OnCommit methods.
    bvector<ValidationError> const& GetErrors() const {return m_validationErrors;}

    //! TxnTable methods may call this to report a validation error.
    //! If the severity of the validation error is set to ValidationError::Severity::Fatal, the transaction will cancel
    //! rather than commit.
    DGNPLATFORM_EXPORT void ReportError(ValidationError&);
    //@}

    //! Get the dgn_TxnTable::Element TxnTable for this TxnManager
    DGNPLATFORM_EXPORT dgn_TxnTable::Element& Elements() const;

    //! Get the dgn_TxnTable::ElementDep TxnTable for this TxnManager
    DGNPLATFORM_EXPORT dgn_TxnTable::ElementDep& ElementDependencies() const;

    //! Get the description of a previously committed Txn, given its TxnId.
    DGNPLATFORM_EXPORT Utf8String GetTxnDescription(TxnId txnId);

    //! Get a description of the operation that would be reversed by calling ReverseTxns(1).
    //! This is useful for showing the operation that would be undone, for example in a pull-down menu.
    DGNPLATFORM_EXPORT Utf8String GetUndoString();

    //! Get a description of the operation that would be reinstated by calling ReinstateTxn.
    //! This is useful for showing the operation that would be redone, in a pull-down menu for example.
    DGNPLATFORM_EXPORT Utf8String GetRedoString();

    //! @name Multi-Txn Operations
    //@{
    //! Begin a new multi-Txn operation. This can be used to cause a series of Txns, that would normally
    //! be considered separate actions for undo, to be grouped into a single undoable operation. This means that when ReverseTxns(1) is called,
    //! the entire group of changes are undone together. Multi-Txn operations can be nested, and until the outermost operation is closed,
    //! all changes constitute a single operation.
    //! @remarks This method must \e always be paired with a call to EndMultiTxnAction.
    DGNPLATFORM_EXPORT void BeginMultiTxnOperation();

    //! End a multi-Txn operation
    DGNPLATFORM_EXPORT void EndMultiTxnOperation();

    //! Return the depth of the multi-Txn stack. Generally for diagnostic use only.
    size_t GetMultiTxnOperationDepth() {return m_multiTxnOp.size();}

    //! @return The TxnId of the the innermost multi-Txn operation. If no multi-Txn operation is active, the TxnId will be zero.
    DGNPLATFORM_EXPORT TxnId GetMultiTxnOperationStart();
    //@}

    //! @name Reversing and Reinstating Transactions
    //@{
    //! Get the current Action being processed by the TxnManager. This can be called from inside TxnTable methods only,
    //! otherwise it will always return TxnAction::None
    TxnAction GetCurrentAction() const {return m_action;}

    //! Get the TxnId of the current Txn.
    //! @return the current TxnId. This value can be saved and later used to reverse changes that happen after this time.
    //! @see   ReverseTo CancelTo
    TxnId GetCurrentTxnId() {return m_curr;}

    //! Get the current SessionId.
    SessionId GetCurrentSessionId() const {return m_curr.GetSession();}

    //! Get the TxnId of the first Txn for this session (though it may not be committed yet.)
    //! All TxnIds with a value lower than this are from a previous session.
    TxnId GetSessionStartId() const {return TxnId(m_curr.GetSession(), 0);}

    //! Given a TxnId, query for TxnId of the immediately previous committed Txn, if any.
    //! @param[in] currentTxnId The current TxnId.
    //! @return The previous TxnId. Will be invalid if currentTxnId is the first one.
    //! @note The TxnId may be from a previous session.
    //! @note The TxnIds are not necessarily sequential.
    DGNPLATFORM_EXPORT TxnId QueryPreviousTxnId(TxnId currentTxnId) const;

    //! Given a TxnId, query for TxnId of the next committed Txn, if any.
    //! @param[in] currentTxnId The current TxnId.
    //! @return The next TxnId. Will be invalid if currentTxnId is the last one.
    //! @note The TxnIds are not necessarily sequential.
    DGNPLATFORM_EXPORT TxnId QueryNextTxnId(TxnId currentTxnId) const;

    //! Query if there are currently any reversible (undoable) changes
    //! @return true if there are currently any reversible (undoable) changes.
    bool IsUndoPossible() {return GetSessionStartId() < GetCurrentTxnId();}

    //! Query if there are currently any reinstateable (redoable) changes
    //! @return True if there are currently any reinstate-able (redoable) changes
    bool IsRedoPossible() {return !m_reversedTxn.empty();}

    enum class AllowCrossSessions {Yes=1, No=0};
    //! Reverse (undo) the most recent operation(s).
    //! @param[in] numOperations the number of operations to reverse. If this is greater than 1, the entire set of operations will
    //! be reinstated together when/if ReinstateTxn is called.
    //! @param[in] crossSessions if No, don't reverse any Txns older than the beginning of the current session.
    //! @note If there are any outstanding uncommitted changes, they are reversed.
    //! @note The term "operation" is used rather than Txn, since multiple Txns can be grouped together via BeginMultiTxnOperation. So,
    //! even if numOperations is 1, multiple Txns may be reversed if they were grouped together when they were made.
    DGNPLATFORM_EXPORT DgnDbStatus ReverseTxns(int numOperations, AllowCrossSessions crossSessions=AllowCrossSessions::No);

    //! Reverse the most recent operation.
    DgnDbStatus ReverseSingleTxn() {return ReverseTxns(1);}

    //! Reverse all changes back to the beginning of the session.
    //! @param[in] prompt display a warning the user, and give an opportunity to cancel.
    DGNPLATFORM_EXPORT DgnDbStatus ReverseAll(bool prompt);

    //! Reverse all changes back to a previously saved TxnId.
    //! @param[in] txnId a TxnId obtained from a previous call to GetCurrentTxnId.
    //! @return DgnDbStatus::Success if the transactions were reversed, error status otherwise.
    //! @see  GetCurrentTxnId CancelTo
    DGNPLATFORM_EXPORT DgnDbStatus ReverseTo(TxnId txnId);

    //! Reverse and then cancel (make non-reinstatable) all changes back to a previous TxnId.
    //! @param[in] txnId a TxnId obtained from a previous call to GetCurrentTxnId.
    //! @return DgnDbStatus::Success if the transactions were reversed and cleared, error status otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus CancelTo(TxnId txnId);

    //! Reinstate the most recently reversed transaction. Since at any time multiple transactions can be reversed, it
    //! may take multiple calls to this method to reinstate all reversed operations.
    //! @return DgnDbStatus::Success if a reversed transaction was reinstated, error status otherwise.
    //! @note If there are any outstanding uncommitted changes, they are reversed before the Txn is reinstated.
    DGNPLATFORM_EXPORT DgnDbStatus ReinstateTxn();
    //@}

    //! Get a summary of changes from all transactions starting with the specified TxnId. 
    //! @remarks Errors out if there are any uncommited changes in the current transaction. i.e., @ref HasChanges() has to 
    //! return false. Call @ref BeSQLite::Db::SaveChanges() or @ref BeSQLite::Db::AbandonChanges() before calling this method. 
    DGNPLATFORM_EXPORT DgnDbStatus GetChangeSummary(BeSQLite::EC::ChangeSummary& changeSummary, TxnId startTxnId);

    //! Get the DgnDb for this TxnManager
    DgnDbR GetDgnDb() {return m_dgndb;}
};

//=======================================================================================
//! @namespace BentleyApi::Dgn::dgn_TxnTable TxnTables in the base "Dgn" domain.
//! @note Only handlers from the base "Dgn" domain belong in this namespace.
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
namespace dgn_TxnTable
{
    struct Element : TxnTable
    {
        BeSQLite::Statement m_stmt;
        bool m_changes;
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_CLASSNAME_Element);}
        Utf8CP _GetTableName() const {return MyTableName();}

        Element(TxnManager& mgr) : TxnTable(mgr) {}

        void _OnValidate() override;
        void _OnValidateAdd(BeSQLite::Changes::Change const& change) override    {AddChange(change, TxnTable::ChangeType::Insert);}
        void _OnValidateDelete(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Delete);}
        void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Update);}
        void _OnValidated() override;
        void _OnReversedDelete(BeSQLite::Changes::Change const&) override;
        void _OnReversedAdd(BeSQLite::Changes::Change const&) override;
        void _OnReversedUpdate(BeSQLite::Changes::Change const&) override;

        void AddChange(BeSQLite::Changes::Change const& change, ChangeType changeType);
        void AddElement(DgnElementId, DgnModelId, ChangeType changeType);

        //! iterator for elements that are directly changed. Only valid during _PropagateChanges.
        struct Iterator : BeSQLite::DbTableIterator
        {
        public:
            Iterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR)db) {}
            struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
                friend struct Iterator;
                Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}

            public:
                DGNPLATFORM_EXPORT DgnModelId GetModelId() const;
                DGNPLATFORM_EXPORT DgnElementId GetElementId() const;
                DGNPLATFORM_EXPORT ChangeType GetChangeType() const;
                Entry const& operator*() const {return *this;}
            };

            typedef Entry const_iterator;
            typedef Entry iterator;
            DGNPLATFORM_EXPORT const_iterator begin() const;
            const_iterator end() const {return Entry(nullptr, false);}
        };

    Iterator MakeIterator() const {return Iterator(m_txnMgr.GetDgnDb());}
    };

    struct Model : TxnTable
    {
        BeSQLite::Statement m_stmt;
        bool m_changes;

        Model(TxnManager& mgr) : TxnTable(mgr) {}
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_CLASSNAME_Model);}
        Utf8CP _GetTableName() const {return MyTableName();}

        void _OnValidate() override;
        void _OnValidateAdd(BeSQLite::Changes::Change const& change) override    {AddChange(change, TxnTable::ChangeType::Insert);}
        void _OnValidateDelete(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Delete);}
        void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Update);}
        void _OnValidated() override;
        void _OnReversedAdd(BeSQLite::Changes::Change const&) override;
        void _OnReversedUpdate(BeSQLite::Changes::Change const&) override;

        void AddChange(BeSQLite::Changes::Change const& change, ChangeType changeType);
    };

    struct ElementDep : TxnTable
    {
        BeSQLite::Statement m_stmt;
        DgnElementIdSet m_failedTargets;
        bool m_changes;
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_RELNAME_ElementDrivesElement);}
        Utf8CP _GetTableName() const {return MyTableName();}

        ElementDep(TxnManager& mgr) : TxnTable(mgr), m_changes(false) {}
        void _OnValidate() override;
        void _OnValidateAdd(BeSQLite::Changes::Change const& change) override    {UpdateSummary(change, TxnTable::ChangeType::Insert);}
        void _OnValidateDelete(BeSQLite::Changes::Change const& change) override {UpdateSummary(change, TxnTable::ChangeType::Update);}
        void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override {UpdateSummary(change, TxnTable::ChangeType::Delete);}
        void _PropagateChanges() override;
        void _OnValidated() override;

        void UpdateSummary(BeSQLite::Changes::Change change, ChangeType changeType);
        void AddDependency(BeSQLite::EC::ECInstanceId const&, ChangeType);
        void AddFailedTarget(DgnElementId id) {m_failedTargets.insert(id);}
        DgnElementIdSet const& GetFailedTargets() const {return m_failedTargets;}
    };

    struct ModelDep : TxnTable
    {
        bool m_changes;
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_RELNAME_ModelDrivesModel);}
        Utf8CP _GetTableName() const {return MyTableName();}
        ModelDep(TxnManager& mgr) : TxnTable(mgr), m_changes(false) {}

        void _OnValidateAdd(BeSQLite::Changes::Change const&) override;
        void _OnValidateUpdate(BeSQLite::Changes::Change const&) override;
        void _PropagateChanges() override;
        void CheckDirection(BeSQLite::EC::ECInstanceId);
        void SetChanges() {m_changes=true;}
        bool HasChanges() const {return m_changes;}
    };

    struct BeProperties : TxnTable
    {
        static Utf8CP MyTableName() {return BEDB_TABLE_Property;}
        Utf8CP _GetTableName() const {return MyTableName();}
        BeProperties(TxnManager& mgr) : TxnTable(mgr) {}
        void _OnReversedUpdate(BeSQLite::Changes::Change const&) override;
    };
};

//=======================================================================================
//! @ingroup TxnMgr
//! @namespace BentleyApi::Dgn::dgn_TableHandler TableHandlers in the base "Dgn" domain.
//! @note Only handlers from the base "Dgn" domain belong in this namespace.
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
namespace dgn_TableHandler
{
    //! TableHandler for DgnElement
    struct Element : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(Element, DGNPLATFORM_EXPORT)
        TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::Element(mgr);}
    };

    //! TableHandler for DgnModel
    struct Model : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(Model, DGNPLATFORM_EXPORT)
        TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::Model(mgr);}
    };

    //! TableHandler for DgnElement dependencies
    struct ElementDep : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(ElementDep, DGNPLATFORM_EXPORT)
        TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::ElementDep(mgr);}
    };

    //! TableHandler for DgnModel dependencies
    struct ModelDep : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(ModelDep, DGNPLATFORM_EXPORT)
        TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::ModelDep(mgr);}
    };

    //! TableHandler for BeProperties
    struct BeProperties : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(BeProperties, DGNPLATFORM_EXPORT)
        TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::BeProperties(mgr);}
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
