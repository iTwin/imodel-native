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
#define TEMP_TABLE(name) "temp." name
#define TXN_TABLE(name)  TXN_TABLE_PREFIX name

#define TXN_TABLE_Elements TXN_TABLE("Elements")
#define TXN_TABLE_Depend   TXN_TABLE("Depend")

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
 @addtogroup TxnMgr
<h1>Transaction Manager</h1>
 The TxnManager API manages Txns. A Txn is a named, committed, undoable unit of work, stored in the DgnDb as a changeset.
 Txns may be "reversed" via an application Undo command, or "reinstated" via a corresponding Redo command.
<p>
<h2>Sessions</h2>
 Every time the TxnManager is initialized, it creates a GUID for itself called a SessionId.
 Whenever an application calls DgnDb::SaveChanges(), a Txn is created. Txns are saved in Briefcases in the DGN_TABLE_Txns
 table, along with the current SessionId. Only Txns from the current SessionId are (usually) undoable. After the completion of a
 session, all of the Txns for that SessionId may be merged together to form a "session Txn". Further, all of the session Txns
 since a Briefcase was last committed to a server may be merged together to form a Briefcase Txn. Briefcase Txns are sent between
 users and changed-merged.
 @bsiclass
+===============+===============+===============+===============+===============+======*/

enum class TxnAction {None=0, Commit, Abandon, Reverse, Reinstate, Merge};

//=======================================================================================
//! A 32 bit value to identify the group of entries that form a single transaction.
//! @ingroup TxnMgr
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
//! A rowid in the DGN_TABLE_Txns table.
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
//! Interface to be implemented to monitor changes to elements.
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

    //! @name Reversing previously committed changesets via undo/red.
    //@{
    //! Called when an add of a row in this TxnTable was reversed via undo or redo.
    //! @param[in] change The data for a previously added row that is now deleted. All data will be in the "old values" of change.
    //! @note If you wish to determine whether the action that caused this call was an undo or a redo, call m_txnMgr.GetCurrentAction()
    virtual void _OnReversedAdd(BeSQLite::Changes::Change const& change) {}

    //! Called when a delete of a row in this TxnTable was reversed via undo or redo.
    //! @param[in] change The data for a previously deleted row that is now back in place. All data will be in the "new values" of change.
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
//! The first and last entry number that forms a single operation.
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
//! To reinstate a reversed operation, we need to know the first and last entry number.
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
//! This class implements the DgnDb::Txns() object.
//!    - Reversing (undo) and Reinstating (redo) Txns
//!    - change propagation.
//!    - combining multi-step Txns into a single reversible "operation"
//!    - change merging
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
        virtual ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override;
    };

    struct CompareTableNames {bool operator()(Utf8CP a, Utf8CP b) const {return strcmp(a, b) < 0;}};
    typedef bmap<Utf8CP,TxnTablePtr,CompareTableNames> T_TxnTablesByName;
    typedef bvector<TxnTable*> T_TxnTables;
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

protected:
    DgnDbR          m_dgndb;
    T_TxnTablesByName m_tablesByName;
    T_TxnTables     m_tables;
    ChangeEntry     m_curr;
    TxnAction       m_action;
    bvector<TxnId>  m_multiTxnOp;
    bvector<RevTxn> m_reversedTxn;
    bool            m_inDynamics;
    bool            m_propagateChanges;
    BeSQLite::StatementCache    m_stmts;
    BeSQLite::SnappyFromBlob    m_snappyFrom;
    BeSQLite::SnappyToBlob      m_snappyTo;
    bvector<ValidationError> m_validationErrors; //!< Validation errors detected

private:
    void AddChangeSet(BeSQLite::ChangeSet&);
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override;
    void _OnSettingsSave() override {m_curr.m_settingsChange=true;}
    void _OnSettingsSaved() override {m_curr.m_settingsChange=false;}
    TrackChangesForTable _FilterTable(Utf8CP tableName) override;

    BeSQLite::DbResult SaveCurrentChange(BeSQLite::ChangeSet& changeset, Utf8CP operation);
    BeSQLite::DbResult ReadEntry(TxnRowId rowid, ChangeEntry& entry);
    void ReadChangeSet(UndoChangeSet&, TxnRowId rowid, TxnAction);
    TxnRowId FirstRow(TxnId id);
    TxnRowId LastRow(TxnId id);

    void TruncateChanges(TxnId id);
    void SetUndoInProgress(bool);
    void ReverseTxnRange(TxnRange& txnRange, Utf8StringP, bool);
    void ReinstateTxn(TxnRange&, Utf8StringP redoStr);
    void ApplyChanges(TxnRowId, TxnAction);
    void OnChangesetApplied(BeSQLite::ChangeSet& changeset, TxnAction);
    OnCommitStatus CancelChanges(BeSQLite::ChangeSet& changeset);
    DgnDbStatus ReinstateActions(RevTxn& revTxn);
    bool PrepareForUndo();
    DgnDbStatus ReverseActions(TxnRange& txnRange, bool multiStep, bool showMsg);
    BentleyStatus PropagateChanges();
    void DeleteReversedTxns();
    TxnTable* FindTxnTable(Utf8CP tableName) const;
    BeSQLite::DbResult ApplyChangeSet(BeSQLite::ChangeSet& changeset, TxnAction isUndo);

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

    //! DgnElementDependencyGraph methods may call this to report a validation error.
    //! If the severity of the validation error is set to ValidationErrorSeverity::Fatal, the transaction will be cancel
    //! rather than commit.
    DGNPLATFORM_EXPORT void ReportError(ValidationError&);
    //@}

    DGNPLATFORM_EXPORT dgn_TxnTable::Element&    Elements() const;
    DGNPLATFORM_EXPORT dgn_TxnTable::ElementDep& ElementDependencies() const;

    //! Get a description of the operation that would be reversed by calling ReverseTxns.
    //! This is useful for showing the name in a pulldown menu, for example
    DGNPLATFORM_EXPORT Utf8String GetUndoString();

    //! Get a description of the operation that would be reversed by calling ReinstateTxn.
    //! This is useful for showing the name in a pulldown menu, for example
    DGNPLATFORM_EXPORT Utf8String GetRedoString();

    //! @name Multi-transaction Operations
    //@{
    //! Begin a new multi-transaction operation. This can be used to cause a series of transactions, that would normally
    //! be considered separate actions for undo, to be grouped into a single undoable operation. This means that when the user issues the "undo"
    //! command, the entire group of changes are undone together. Multi Txn operations can be nested, and until the outermost operation is closed,
    //! all changes constitute a single operation.
    //! @remarks This method must \e always be paired with a call to EndMultiTxnAction.
    DGNPLATFORM_EXPORT void BeginMultiTxnOperation();

    //! End a multi-transaction operation
    DGNPLATFORM_EXPORT void EndMultiTxnOperation();

    //! Return the depth of the multi-transaction stack. Generally for diagnostic use only.
    size_t GetMultiTxnOperationDepth() {return m_multiTxnOp.size();}

    //! @return The TxnId of the the innermost multi-Transaction operation. If no multi-Transaction operation is active, the TxnId will be zero.
    DGNPLATFORM_EXPORT TxnId GetMultiTxnOperationStart();
    //@}

    //! @name Reversing and Reinstating Transactions
    //@{
    TxnAction GetCurrentAction() const {return m_action;}

    //! Query if there are currently any reversible (undoable) changes in the Transaction Manager
    //! @return true if there are currently any reversible (undoable) changes in the Transaction Manager.
    bool IsUndoPossible() {return 0 < GetCurrTxnId();}

    //! Query if there are currently any reinstateable (redoable) changes in the Transaction Manager
    //! @return True if there are currently any reinstateable (redoable) changes in the Transaction Manager.
    bool IsRedoPossible() {return !m_reversedTxn.empty();}

    //! Reverse (undo) the most recent operation(s).
    //! @param[in] numActions the number of operations to reverse. If numActions is greater than 1, the entire set of operations will
    //! be reinstated together when/if ReinstateTxn is called.
     DGNPLATFORM_EXPORT DgnDbStatus ReverseTxns(int numActions);

    //! Reverse the most recent operation.
    DgnDbStatus ReverseSingleTxn() {return ReverseTxns(1);}

    //! Reverse all element changes back to the beginning of the session.
    //! @param[in] prompt display a dialog warning the user of the severity of this action and giving an opportunity to cancel.
    DGNPLATFORM_EXPORT void ReverseAll(bool prompt);

    //! Reverse all element changes back to a previously saved TxnPos.
    //! @param[in] pos a TxnPos obtained from a previous call to GetCurrTxnPos.
    //! @return DgnDbStatus::Success if the transactions were reversed, error status otherwise.
    //! @see  GetCurrTxnPos CancelToPos
    DGNPLATFORM_EXPORT DgnDbStatus ReverseToPos(TxnId pos);

    //! Get the Id of the most recently commited transaction.
    //! @return the current TxnPos. This value can be saved and later used to reverse changes that happen after this time.
    //! @see   ReverseToPos CancelToPos
    TxnId GetCurrTxnId() {return m_curr.m_txnId;}

    //! Reverse and then cancel (make non-reinstatable) all changes back to a previous TxnPos.
    //! @param[in] pos a TxnPos obtained from a previous call to GetCurrTxnPos.
    //! @return DgnDbStatus::Success if the transactions were reversed and cleared, error status otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus CancelToPos(TxnId pos);

    //! Reinstate the most recently reversed transaction. Since at any time multiple transactions can be reversed, it
    //! may take multiple calls to this method to reinstate all reversed operations.
    //! @return DgnDbStatus::Success if a reversed transaction was reinstated, error status otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus ReinstateTxn();
    //@}

    //! Get the DgnDb of this Txns
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

        virtual void _OnValidate() override;
        virtual void _OnValidateAdd(BeSQLite::Changes::Change const& change) override    {AddChange(change, TxnTable::ChangeType::Insert);}
        virtual void _OnValidateDelete(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Delete);}
        virtual void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Update);}
        virtual void _OnValidated() override;

        virtual void _OnReversedDelete(BeSQLite::Changes::Change const&) override;
        virtual void _OnReversedAdd(BeSQLite::Changes::Change const&) override;
        virtual void _OnReversedUpdate(BeSQLite::Changes::Change const&) override;

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
        Model(TxnManager& mgr) : TxnTable(mgr) {}
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_CLASSNAME_Model);}
        Utf8CP _GetTableName() const {return MyTableName();}

        virtual void _OnReversedAdd(BeSQLite::Changes::Change const&) override;
        virtual void _OnReversedUpdate(BeSQLite::Changes::Change const&) override;
    };

    struct ElementDep : TxnTable
    {
        BeSQLite::Statement m_stmt;
        DgnElementIdSet m_failedTargets;
        bool m_changes;
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_RELNAME_ElementDrivesElement);}
        Utf8CP _GetTableName() const {return MyTableName();}

        ElementDep(TxnManager& mgr) : TxnTable(mgr), m_changes(false) {}
        virtual void _OnValidate() override;
        virtual void _OnValidateAdd(BeSQLite::Changes::Change const& change) override    {UpdateSummary(change, TxnTable::ChangeType::Insert);}
        virtual void _OnValidateDelete(BeSQLite::Changes::Change const& change) override {UpdateSummary(change, TxnTable::ChangeType::Update);}
        virtual void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override {UpdateSummary(change, TxnTable::ChangeType::Delete);}
        virtual void _PropagateChanges() override;
        virtual void _OnValidated() override;

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

        virtual void _OnValidateAdd(BeSQLite::Changes::Change const&) override;
        virtual void _OnValidateUpdate(BeSQLite::Changes::Change const&) override;
        virtual void _PropagateChanges() override;
        void CheckDirection(BeSQLite::EC::ECInstanceId);
        void SetChanges() {m_changes=true;}
        bool HasChanges() const {return m_changes;}
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
        virtual TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::Element(mgr);}
    };

    //! TableHandler for DgnModel
    struct Model : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(Model, DGNPLATFORM_EXPORT)
        virtual TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::Model(mgr);}
    };

    //! TableHandler for DgnElement dependencies
    struct ElementDep : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(ElementDep, DGNPLATFORM_EXPORT)
        virtual TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::ElementDep(mgr);}
    };

    //! TableHandler for DgnModel dependencies
    struct ModelDep : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(ModelDep, DGNPLATFORM_EXPORT)
        virtual TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::ModelDep(mgr);}
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
