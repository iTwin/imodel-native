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

//  temp table names used by TxnSummary and ElementGraphTxnMonitor
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

//! Identifies the transaction direction
//! @ingroup TxnMgr
enum class TxnDirection {Backwards=0, Forward=1};

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
    virtual void _OnTxnCommit(TxnSummaryCR) = 0;
    virtual void _OnTxnReverse(TxnSummaryCR) = 0;
    virtual void _OnTxnReversed(TxnSummaryCR) = 0;
    virtual void _OnUndoRedoFinished(DgnDbR, TxnDirection isUndo) {}
};

namespace dgn_TxnTable {struct Element; struct ElementDep;}
//=======================================================================================
//! A summary of all of the Element-based changes that occurred during a Txn. TxnMonitors are supplied with a
//! TxnSummary so they can determine what Elements were affected by a Txn.
//! @ingroup TxnMgr
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
    TxnDirection m_direction;
    bvector<ValidationError> m_validationErrors; //!< Validation errors detected on the last boundary check

public:
    enum class ChangeType : int {Insert, Update, Delete};

    DGNPLATFORM_EXPORT explicit TxnSummary(DgnDbR db, TxnDirection);
    DGNPLATFORM_EXPORT ~TxnSummary();

    DGNPLATFORM_EXPORT dgn_TxnTable::Element&    Elements() const;
    DGNPLATFORM_EXPORT dgn_TxnTable::ElementDep& ElementDependencies() const;

    void AddChangeSet(BeSQLite::ChangeSet&);

    //! Get the DgnDb for this TxnSummary
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! TxnMonitors may call this to report a validation error. If the severity of the validation error is set to ValidationErrorSeverity::Fatal, 
    //! then the transaction will be cancelled.
    DGNPLATFORM_EXPORT void ReportError(ValidationError&);

    //! Query the validation errors that were reported during the last boundary check.
    bvector<ValidationError> const& GetErrors() const {return m_validationErrors;}

    //! Query if any Fatal validation errors were reported during the last boundary check.
    DGNPLATFORM_EXPORT bool HasFatalErrors() const;
};

//=======================================================================================
//! An instance of a TxnTable is created for a single SQLite table via a DgnDomain::TableHandler.
//! A TxnTable's sole role is to synchronize in-memory objects with persistent changes to the database through Txns.
//! That is, the TxnTable "monitors" changes to the rows in a SQLite database table for the purpose of ensuring
//! that in-memory copies of the data reflect the current state on disk. The TxnTable itself has no 
//! role in making or reversing changes to its table. Instead, the TxnManager orchestrates Txn commits, undo, redo
//! and change merging operations and informs the TxnTable of what happened.
//! <p>
//!
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct TxnTable : RefCountedBase
    {
    virtual Utf8CP _GetTableName() const = 0;
    virtual void _OnTxnSummaryStart(TxnSummary&) = 0;
    virtual void _OnTxnSummaryEnd(TxnSummary&) = 0;
    virtual void _PropagateChanges(TxnSummary&) = 0;
    virtual void _OnChangesetApplied(TxnSummary&) = 0;
    virtual void _OnAdd(TxnSummary&, BeSQLite::Changes::Change const&) = 0;
    virtual void _OnDelete(TxnSummary&, BeSQLite::Changes::Change const&) = 0;
    virtual void _OnUpdate(TxnSummary&, BeSQLite::Changes::Change const&) = 0;
    };
typedef RefCountedPtr<TxnTable> TxnTablePtr;

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
        DgnDb& m_dgndb;
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_CLASSNAME_Element);}
        Utf8CP _GetTableName() const {return MyTableName();}

        Element(DgnDb& dgndb) : m_dgndb(dgndb) {}
        virtual void _OnTxnSummaryStart(TxnSummary&) override;
        virtual void _OnTxnSummaryEnd(TxnSummary& summary) override;
        virtual void _PropagateChanges(TxnSummary&) override {}
        virtual void _OnChangesetApplied(TxnSummary&) override;
        virtual void _OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) override {AddChange(summary, change, TxnSummary::ChangeType::Insert);}
        virtual void _OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) override {AddChange(summary, change, TxnSummary::ChangeType::Delete);}
        virtual void _OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) override {AddChange(summary, change, TxnSummary::ChangeType::Update);}
        void AddChange(TxnSummary& summary, BeSQLite::Changes::Change const& change, TxnSummary::ChangeType changeType);
        void AddElement(DgnElementId, DgnModelId, double lastMod, TxnSummary::ChangeType changeType);

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
                DGNPLATFORM_EXPORT TxnSummary::ChangeType GetChangeType() const;
                DGNPLATFORM_EXPORT double GetLastMod() const;
                Entry const& operator*() const {return *this;}
            };

            typedef Entry const_iterator;
            typedef Entry iterator;
            DGNPLATFORM_EXPORT const_iterator begin() const;
            const_iterator end() const {return Entry(nullptr, false);}
        };

    Iterator MakeIterator() const {return Iterator(m_dgndb);}
    };

    struct Model : TxnTable
    {
        bool m_initialized;
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_CLASSNAME_Model);}
        Utf8CP _GetTableName() const {return MyTableName();}
        Model(DgnDb& dgndb) {m_initialized=false;}
        virtual void _OnTxnSummaryStart(TxnSummary&) override;
        virtual void _OnTxnSummaryEnd(TxnSummary& summary) override;
        virtual void _PropagateChanges(TxnSummary&) override {}
        virtual void _OnChangesetApplied(TxnSummary&) override {}
        virtual void _OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) override {}
        virtual void _OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) override {}
        virtual void _OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) override {}
    };

    struct ElementDep : TxnTable
    {
        BeSQLite::Statement m_stmt;
        DgnDbR m_dgndb;
        DgnElementIdSet m_failedTargets;
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_RELNAME_ElementDrivesElement);}
        Utf8CP _GetTableName() const {return MyTableName();}

        ElementDep(DgnDb& dgndb) : m_dgndb(dgndb) {}
        virtual void _OnTxnSummaryStart(TxnSummary&) override;
        virtual void _OnTxnSummaryEnd(TxnSummary& summary) override;
        virtual void _PropagateChanges(TxnSummary&) override;
        virtual void _OnChangesetApplied(TxnSummary&) override {}
        virtual void _OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) override    {UpdateSummary(summary,change,TxnSummary::ChangeType::Insert);}
        virtual void _OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) override {UpdateSummary(summary,change,TxnSummary::ChangeType::Update);}
        virtual void _OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) override {UpdateSummary(summary,change,TxnSummary::ChangeType::Delete);}
        void UpdateSummary(TxnSummary& summary, BeSQLite::Changes::Change change, TxnSummary::ChangeType changeType);
        void AddDependency(BeSQLite::EC::ECInstanceId const&, TxnSummary::ChangeType);
        void AddFailedTarget(DgnElementId id) {m_failedTargets.insert(id);}
        DgnElementIdSet const& GetFailedTargets() const {return m_failedTargets;}
    };

    struct ModelDep : TxnTable
    {
        bool m_changes;
        static Utf8CP MyTableName() {return DGN_TABLE(DGN_RELNAME_ModelDrivesModel);}
        Utf8CP _GetTableName() const {return MyTableName();}
        ModelDep (DgnDb& dgndb) : m_changes(false) {}
        virtual void _OnTxnSummaryStart(TxnSummary&) override {m_changes=false;}
        virtual void _OnTxnSummaryEnd(TxnSummary&) override {}
        virtual void _PropagateChanges(TxnSummary& summary) override;
        virtual void _OnChangesetApplied(TxnSummary&) override {}
        virtual void _OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) override;
        virtual void _OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) override;
        virtual void _OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) override {SetChanges();}
        void CheckDirection(TxnSummary&, BeSQLite::EC::ECInstanceId);
        void SetChanges() {m_changes=true;}
        bool HasChanges() const {return m_changes;}
    };
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
//! @ingroup TxnMgr
// @bsiclass
//=======================================================================================
struct TxnManager : BeSQLite::ChangeTracker
{
    friend struct TxnSummary;

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

    struct CompareTableNames {bool operator()(Utf8CP a, Utf8CP b) const {return strcmp(a, b) < 0;}};
    typedef bmap<Utf8CP,TxnTablePtr,CompareTableNames> T_TxnTablesByName;
    typedef bvector<TxnTable*> T_TxnTables;

protected:
    DgnDbR          m_dgndb;
    T_TxnTablesByName m_tablesByName;
    T_TxnTables     m_tables;
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
    TxnTable* FindTxnTable(Utf8CP tableName);

public:
    void AddTxnTable(DgnDomain::TableHandler*);
    void SetPropagateChanges(bool val) {m_propagateChanges = val;}
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
    //! @see ReinstateTxn, ClearReversedTxns
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
    //! @see  GetCurrTxnPos, CancelToPos
    DGNPLATFORM_EXPORT StatusInt ReverseToPos(TxnId pos);

    //! Get the Id of the most recently commited transaction.
    //! @return the current TxnPos. This value can be saved and later used to reverse changes that happen after this time.
    //! @see ReverseToPos, CancelToPos
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

//=======================================================================================
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
        virtual TxnTable* _Create(DgnDb& db) const override {return new dgn_TxnTable::Element(db);}
    };

    //! TableHandler for DgnModel
    struct Model : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(Model, DGNPLATFORM_EXPORT)
        virtual TxnTable* _Create(DgnDb& db) const override {return new dgn_TxnTable::Model(db);}
    };

    //! TableHandler for DgnElement dependencies
    struct ElementDep : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(ElementDep, DGNPLATFORM_EXPORT)
        virtual TxnTable* _Create(DgnDb& db) const override {return new dgn_TxnTable::ElementDep(db);}
    };

    //! TableHandler for DgnModel dependencies
    struct ModelDep : DgnDomain::TableHandler
    {
        TABLEHANDLER_DECLARE_MEMBERS(ModelDep, DGNPLATFORM_EXPORT)
        virtual TxnTable* _Create(DgnDb& db) const override {return new dgn_TxnTable::ModelDep(db);}
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
