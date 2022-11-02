/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <cstddef>
#include "DgnDb.h"

DGNPLATFORM_TYPEDEFS(TxnMonitor)
DGNPLATFORM_TYPEDEFS(DynamicChangeTracker)
DGNPLATFORM_REF_COUNTED_PTR(DynamicChangeTracker)

#define TXN_TABLE_PREFIX "txn_"
#define TXN_TABLE(name)  TXN_TABLE_PREFIX name
#define TXN_TABLE_Elements TXN_TABLE("Elements")
#define TXN_TABLE_Depend   TXN_TABLE("Depend")
#define TXN_TABLE_Models   TXN_TABLE("Models")
#define TXN_TABLE_RelationshipLinkTables TXN_TABLE("RelationshipLinkTables")

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
 @addtogroup GROUP_TxnManager Transaction Manager Module
 Types related to the transaction (txn) manager.
 The TxnManager API manages Txns. A Txn is a named, committed, undoable unit of work, stored in the DgnDb as a changeset.
 Txns may be "reversed" via an application Undo command, or "reinstated" via a corresponding Redo command.
<p>
 Every time the TxnManager is initialized, it creates a GUID for itself called a SessionId.
 Whenever an application calls DgnDb::SaveChanges(), a new Txn is created. Txns are saved in Briefcases in the DGN_TABLE_Txns
 table, along with the current SessionId. Only Txns from the current SessionId are (usually) undoable. After the completion of a
 session, all of the Txns for that SessionId may be merged together to form a "session Txn". Further, all of the session Txns
 since a Briefcase was last committed to a server may be merged together to form a Briefcase Txn. Briefcase Txns are sent between
 users and changed-merged.
*/

//! Actions that cause events to be sent to TxnTables.
enum class TxnAction {
    None = 0,  //!< not currently processing anything
    Commit,    //!< processing a Commit. Triggered by a call to DgnDb::SaveChanges
    Abandon,   //!< abandoning the current Txn. Triggered by a call to DgnDb::AbandonChanges
    Reverse,   //!< reversing a previously committed ChangeSet. Triggered by a call to TxnManager::ReverseActions
    Reinstate, //!< reinstating a previously reversed ChangeSet. Triggered by a call to TxnManager::ReinstateActions
    Merge      //!< merging a ChangeSet made by a foreign briefcase.
};

// The type of data held in a Txn entry.
// @note This value is stored in the "IsSchemaChange" column of the Txn table, that was previously declared to be of type "Boolean" that meant
// "is this a ddl string." However, we now hold an integer, so that we can distinguish a changeset that holds changes to the EcSchema tables.
// Since this is a persistent value, but since SQLite allows column-affinity (i.e. values different than the declared column type) we now allow
// 0, 1, or 2. The first two match what was previously stored (as a Boolean), and the last one
enum class TxnType : int32_t {
    Data = 0, // Txn holds a changeset with only changes to data tables
    Ddl = 1, // Txn holds a ddl change string. This is *not* a changeset. NOTE: The column type use to be boolean meaning "isDdl", so for backwards compatiblity
    EcSchema = 2, // Txn holds a changeset with changes to EC tables, plus potentially changes to other data tables
};

//=======================================================================================
//! Interface to be implemented to monitor changes to a DgnDb.
//! Call TxnManager::AddTxnMonitor to register a TxnMonitor.
// @bsiclass
//=======================================================================================
struct TxnMonitor {
    virtual ~TxnMonitor() { }
    virtual void _OnCommit(TxnManager&) {}
    virtual void _OnCommitted(TxnManager&) {}
    virtual void _OnAppliedChanges(TxnManager&) {}
    virtual void _OnUndoRedo(TxnManager&, TxnAction) {}
    virtual void _OnGeometricModelChanges(TxnManager&, BeJsConst) {}
    virtual void _OnGeometryGuidChanges(TxnManager&, bset<DgnModelId> const& modelIds) {}
};

struct RevisionChangesFileReader;
namespace dgn_TxnTable {
    struct Element;
    struct ElementDep;
    struct Model;
    struct RelationshipLinkTable;
    struct UniqueRelationshipLinkTable;
    struct MultiRelationshipLinkTable;
}

//=======================================================================================
//! An instance of a TxnTable is created for a single SQLite table of a DgnDb via a DgnDomain::TableHandler.
//! A TxnTable's sole role is to synchronize in-memory objects with persistent changes to the database through Txns.
//! That is, the TxnTable "monitors" changes to the rows in a SQLite database table for the purpose of ensuring
//! that in-memory copies of the data from its table reflect the current state on disk. The TxnTable itself has no
//! role in making or reversing changes to its table. Instead, the TxnManager orchestrates Txn commits, undo, redo
//! and change merging operations and informs the TxnTable of what happened.
// @bsiclass
//=======================================================================================
struct TxnTable : RefCountedBase {
    enum class ChangeType : int { Insert=0, Update=1, Delete=2 };
    TxnManager& m_txnMgr;
    TxnTable(TxnManager& mgr) : m_txnMgr(mgr) {}

    // look up the modelId and classId by elementId from the element table.
    DgnModelId GetModelAndClass(ECN::ECClassId& classId, DgnElementId id);

    //! Return the name of the table handled by this TxnTable.
    virtual Utf8CP _GetTableName() const = 0;

    //! Create any temp tables that will be used by _OnValidate. The results of this method will be committed before
    //! any actual changes are validated or txns are created.
    virtual void _Initialize() {}

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

    //! @name Applying change sets (undo/redo of transactions, applying/reversing/reinstating of revisions)
    //@{
    //! Called before a set of changes are applied to the Db. TxnTables that use temporary tables can prepare them in this method.
    //! After this method is called one or more _OnAppliedxxx methods will be called, followed finally by a call to _OnApplied
    virtual void _OnApply() {}

    //! Called after a set of change sets are applied to the Db. TxnTables that create temporary tables can empty them in this method.
    virtual void _OnApplied() {}

    //! Called after applying a delete record from the change set
    //! @param[in] change The change record for the delete. All data will be in the "old values" of change.
    //! @note Use m_txnMgr.GetCurrentAction() to determine the action (undo, redo, merge, etc.) that caused this call
    virtual void _OnAppliedDelete(BeSQLite::Changes::Change const& change) {}

    //! Called after applying an add record from the change set
    //! @param[in] change The change record for the add. All data will be in the "new values" of change.
    virtual void _OnAppliedAdd(BeSQLite::Changes::Change const& change) {}

    //! Called after apply an update record from the change set
    //! @param[in] change The change record for the update. The pre-changed data about the row will be in the "old values" of change,
    //! and the post-changed data will be in the "new values". Columns that are unchanged are in neither values.
    virtual void _OnAppliedUpdate(BeSQLite::Changes::Change const& change) {}
    //@}
};
typedef RefCountedPtr<TxnTable> TxnTablePtr;

//=======================================================================================
//! Manages the temp table that records all link table changes during a transaction.
//! Includes only relationships for which tracking has been requested.
//! See TxnManager::BeginTrackingRelationship for how to start tracking an ECRelationship.
//! <p>To query changes, a TxnMonitor should query the RelationshipLinkTables. Here is an example:
//! __PUBLISH_INSERT_FILE__ RelationshipLinkTableTrackingTxnMonitor_OnCommit_.sampleCode
// @bsiclass
//=======================================================================================
struct TxnRelationshipLinkTables
    {
    friend struct TxnManager;
    friend struct dgn_TxnTable::RelationshipLinkTable;
    friend struct dgn_TxnTable::UniqueRelationshipLinkTable;
    friend struct dgn_TxnTable::MultiRelationshipLinkTable;
  private:
    TxnManager& m_txnMgr;
    BeSQLite::CachedStatementPtr m_stmt;
    TxnRelationshipLinkTables(TxnManagerR t);
    BeSQLite::DbResult Insert(BeSQLite::EC::ECInstanceId relid, ECN::ECClassId relclsid, DgnElementId srcelemid, DgnElementId tgtelemid, TxnTable::ChangeType changeType);

  public:
    constexpr static Utf8CP COLNAME_ECInstanceId = "ECInstanceId"; //!< The name of the column that contains the relationship's own ID. Type = BeSQLite::EC::ECInstanceId
    constexpr static Utf8CP COLNAME_ECClassId = "ECClassId"; //!< The name of the column that contains the ID of the relationship class. Type = ECN::ECClassId
    constexpr static Utf8CP COLNAME_SourceECInstanceId = "SourceECInstanceId"; //!< The name of the column that contains the ID of the source element. Type = DgnElementId
    constexpr static Utf8CP COLNAME_TargetECInstanceId = "TargetECInstanceId"; //!< The name of the column that contains the ID of the target element. Type = DgnElementId
    constexpr static Utf8CP COLNAME_ChangeType = "ChangeType"; //!< The name of the column that identifies what kind of change this is. Type = TxnTable::ChangeType
    };


 enum class CallbackOnCommitStatus { Continue = 0, Abort };
 // Callback for testing purpose only
 typedef std::function<CallbackOnCommitStatus(TxnManager&, bool isCommit, Utf8CP operation, BeSQLite::ChangeSetCR, BeSQLite::DdlChangesCR)> T_OnCommitCallback;

//=======================================================================================
//! Interface adopted by a callback object supplied to TxnManager::EndDynamicOperation(),
//! to react to change propagation caused by dynamic operations before they are rolled back.
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DynamicTxnProcessor {
    virtual void _ProcessDynamicChanges() = 0;
};

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
struct TxnManager : BeSQLite::ChangeTracker {
    friend struct RevisionManager;
    friend struct DynamicChangeTracker;
    friend struct DgnDb;

    //=======================================================================================
    //! An identifier stored in the high 4 bytes of a TxnId. All TxnIds for a given session will have the same SessionId.
    //! Every time the TxnManager is initialized against a DgnDb, the SessionId is incremented to be one greater than the
    //! previous highest SessionId.
    // @bsiclass
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
    // @bsiclass
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
        bool operator!=(TxnId const& rhs) const {return m_id.m_64 != rhs.m_id.m_64;}
        bool operator<(TxnId const& rhs) const  {return m_id.m_64<rhs.m_id.m_64;}
        bool operator<=(TxnId const& rhs) const {return m_id.m_64<=rhs.m_id.m_64;}
        bool operator>(TxnId const& rhs) const  {return m_id.m_64>rhs.m_id.m_64;}
        bool operator>=(TxnId const& rhs) const {return m_id.m_64>=rhs.m_id.m_64;}
    };

private:
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct TxnRange
    {
    private:
        TxnId m_first;
        TxnId m_last;
    public:
        TxnRange(TxnId first, TxnId last) : m_first(first), m_last(last) {}
        TxnId GetFirst() const {return m_first;}
        TxnId GetLast() const {return m_last;}
    };

    struct UndoChangeSet : BeSQLite::ChangeSet
    {
        ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override;
    };

    struct CompareTableNames {bool operator()(Utf8CP a, Utf8CP b) const {return strcmp(a, b) < 0;}};
    typedef bmap<Utf8CP,TxnTablePtr,CompareTableNames> T_TxnTablesByName;
    typedef bvector<TxnTable*> T_TxnTables;

    DgnDbR m_dgndb;
    T_TxnTablesByName m_tablesByName;
    T_TxnTables m_tables;
    TxnId m_curr;
    TxnAction m_action;
    bvector<TxnId> m_multiTxnOp;

public:
    //=======================================================================================
    // For a single GeometricModel, the sets of geometric elements that were deleted, inserted,
    // or had their geometric properties modified during a transaction.
    // @bsistruct
    //=======================================================================================
    struct GeometricElementChanges
    {
    public:
        // Indexed by TxnTable::ChangeType.
        bset<DgnElementId> m_elements[3];

        void AddElement(DgnElementId id, TxnTable::ChangeType type)
            {
            for (auto i = 0; i < 3; i++)
                {
                auto& entries = m_elements[i];
                if (static_cast<TxnTable::ChangeType>(i) == type)
                    entries.insert(id);
                else
                    entries.erase(id);
                }
            }

        bset<DgnElementId> const& GetElements(TxnTable::ChangeType type) const { return m_elements[static_cast<int>(type)]; }
    };

    //=======================================================================================
    // A set of Ids of entities that were affected by a transaction, along with a flag for
    // each indicating whether any of the changes to the entity originated from a commit
    // (vs applying a changeset, e.g. undo/redo).
    // @bsistruct
    //=======================================================================================
    template<typename Id> struct ChangedIds
    {
        using Map = bmap<Id, bool>;
    private:
        Map m_ids;
    public:
        void Insert(Id id, bool fromCommit)
            {
            auto iter = m_ids.Insert(id, fromCommit);
            if (fromCommit && !iter.second)
                iter.first->second = true;
            }

        bool empty() const { return m_ids.empty(); }
        typename Map::const_iterator begin() const { return m_ids.begin(); }
        typename Map::const_iterator end() const { return m_ids.end(); }
        void erase(Id id) { m_ids.erase(id); }
        void clear() { m_ids.clear(); }
    };

    //=======================================================================================
    // Keeps track of changes to models, and optionally to geometric elements within those
    // models. The latter requires a writable DgnDb with BisCore 1.0.11 or newer.
    // @bsistruct
    //=======================================================================================
    struct ModelChanges
    {
        enum class Status : uint8_t
        {
            Success, //!< Operation successful; tracking is supported.
            Readonly, //!< Tracking unsupported because iModel was opened in read-only mode.
            VersionTooOld, //!< Tracking unsupported because iModel's version of BisCore pre-dates version 1.0.11
        };
    private:
        ChangedIds<DgnModelId> m_models;    // the set of models that have changes for the current transaction
        ChangedIds<DgnModelId> m_geometricModels; // the set of models that have geometric changes for the current transaction
        bmap<DgnModelId, GeometricElementChanges> m_geometryChanges; // changes to elements within geometric models
        bmap<DgnElementId, DgnModelId> m_modelsForDeletedElements; // maps Id of a deleted element to its model Id
        ChangedIds<DgnElementId> m_deletedGeometricElements; // Ids of deleted geometric elements
        TxnManager& m_mgr;
        bool m_determinedStatus = false;
        Status m_status;
        bool m_trackGeometry = false; // true if we are currently tracking changes to geometric elements

        bool IsDisabled() const { return m_determinedStatus && Status::Success != m_status; }
        void Disable()
            {
            m_determinedStatus = true;
            m_status = Status::VersionTooOld;
            ClearAll();
            }

        void Clear(bool preserveGeometryChanges)
            {
            m_models.clear();
            m_geometricModels.clear();
            m_modelsForDeletedElements.clear();
            m_deletedGeometricElements.clear();

            if (!preserveGeometryChanges)
                m_geometryChanges.clear();
            }

        void InsertGeometryChange(DgnModelId modelId, DgnElementId elementId, TxnTable::ChangeType type);
    public:
        explicit ModelChanges(TxnManager& mgr);

        void AddModel(DgnModelId modelId, bool fromCommit) { if (!IsDisabled()) m_models.Insert(modelId, fromCommit); }
        void AddGeometricElementChange(DgnModelId modelId, DgnElementId elementId, TxnTable::ChangeType type, bool fromCommit);
        void AddDeletedElement(DgnElementId elemId, DgnModelId modelId) { if (!IsDisabled()) m_modelsForDeletedElements.Insert(elemId, modelId); }
        void AddDeletedGeometricElement(DgnElementId elemId, bool fromCommit) { if (!IsDisabled()) m_deletedGeometricElements.Insert(elemId, fromCommit); }

        void Process();
        void Notify();
        void ClearAll() { Clear(false); }

        // Returns a non-empty map *only* during a call to TxnMonitor::_OnGeometricModelChanges
        bmap<DgnModelId, GeometricElementChanges> const& GetGeometryChanges() const { return m_geometryChanges; }

        // While tracking geometry changes, TxnManager collects information about which geometric elements have been inserted, deleted, or had their geometric properties modified.
        // It forwards this information to the affected GeometricModel. The model establishes a baseline state for tile generation.
        // It also forwards it to TxnMonitors. The iModel.js node addon responds by emitting notifications to the frontend.
        // When tracking is ended, this information collection ceases and the baseline model states are reset.
        bool IsTrackingGeometry() const { return m_trackGeometry; }

        // Set whether geometry changes should be tracked. They cannot be tracked if the db is read-only or
        // BisCore version < 1.0.11 because the GeometryGuid property was introduced in that version of the schema.
        DGNPLATFORM_EXPORT Status SetTrackingGeometry(bool track);

        // Return whether model changes can be tracked for the DgnDb, or why they can't.
        // Don't call this if you intend to upgrade the schemas contained in the DgnDb, until after you do so.
        DGNPLATFORM_EXPORT Status DetermineStatus();
    };

    // Set and restore indirect changes mode
    struct SetandRestoreIndirectChanges {
        ChangeTracker& m_tracker;
        SetandRestoreIndirectChanges(ChangeTracker& txn) : m_tracker(txn) { txn.SetMode(BeSQLite::ChangeTracker::Mode::Indirect); }
        ~SetandRestoreIndirectChanges() { m_tracker.SetMode(BeSQLite::ChangeTracker::Mode::Direct); }
    };

private:
    bvector<TxnRange> m_reversedTxn;
    bvector<DynamicChangeTrackerPtr> m_dynamicTxns;
    BeSQLite::StatementCache m_stmts;
    BeSQLite::SnappyFromBlob m_snappyFrom;
    BeSQLite::SnappyToBlob   m_snappyTo;
    TxnRelationshipLinkTables m_rlt;
    int m_txnErrors = 0;
    bool m_fatalValidationError;
    bool m_initTableHandlers;
    bool m_inProfileUpgrade = false;
    bool m_indirectChanges = false;
    bool m_enableRebasers;
    bvector<ECN::ECClassId> m_childPropagatesChangesToParentRels;
public:
    ModelChanges m_modelChanges;

    void ProcessModelChanges();
    void NotifyModelChanges();
    void ClearModelChanges();
    DGNPLATFORM_EXPORT void Initialize();
private:
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override;
    void _OnCommitted(bool isCommit, Utf8CP operation) override;
    TrackChangesForTable _FilterTable(Utf8CP tableName) override;
    void CallMonitors(std::function<void (TxnMonitor&)>);
    void OnBeforeUndoRedo(bool isUndo);
    void OnUndoRedo(TxnAction action);
    void OnRollback(BeSQLite::ChangeStreamCR);

    void OnValidateChanges(BeSQLite::ChangeStreamCR);
    BeSQLite::DbResult SaveTxn(BeSQLite::ChangeSetCR changeset, Utf8CP operation, TxnType);
    BeSQLite::DbResult SaveRebase(int64_t& id, BeSQLite::Rebase const& rebase);

    BeSQLite::ZipErrors ReadChanges(BeSQLite::ChangeSet& changeset, TxnId rowId);
    BeSQLite::DbResult ReadDataChanges(BeSQLite::ChangeSet&, TxnId rowid, TxnAction);

    void ApplyTxnChanges(TxnId, TxnAction);
    BeSQLite::DbResult ApplyChanges(BeSQLite::ChangeStreamCR, TxnAction txnAction, bool containsSchemaChanges, BeSQLite::Rebase* = nullptr, bool invert = false);
    BeSQLite::DbResult ApplyDdlChanges(BeSQLite::DdlChangesCR);

    void OnBeginApplyChanges();
    void OnEndApplyChanges();
    void OnChangeSetApplied(BeSQLite::ChangeStreamCR changeset, bool invert);
    void OnGeometricModelChanges();

    BentleyStatus PropagateChanges() {return DoPropagateChanges(*this);}
    BentleyStatus DoPropagateChanges(BeSQLite::ChangeTracker& tracker);
    void ReverseTxnRange(TxnRange const& txnRange);
    DgnDbStatus ReverseActions(TxnRange const& txnRange);
    void ReinstateTxn(TxnRange const&);
    DgnDbStatus ReinstateActions(TxnRange const& revTxn);

    RevisionStatus MergeRevision(DgnRevisionCR revision);
    RevisionStatus MakeDdlChangesFromRevision(DgnRevisionCR revision, RevisionChangesFileReader& revisionReader);
    RevisionStatus MergeDataChangesInRevision(DgnRevisionCR revision, RevisionChangesFileReader& revisionReader, bool containsSchemaChanges);
    RevisionStatus ReverseRevision(DgnRevisionCR revision);

    TxnTable* FindTxnTable(Utf8CP tableName) const;
    bool IsMultiTxnMember(TxnId rowid) const;
    TxnType GetTxnType(TxnId rowid) const;

    void CancelDynamics();
    BentleyStatus PatchSlowDdlChanges(Utf8StringR patchedDDL, Utf8StringCR compoundSQL);
    void NotifyOnCommit();

public:
    void CallJsTxnManager(Utf8CP methodName) {DgnDb::CallJsFunction(m_dgndb.GetJsTxns(), methodName, {}); };

    //! Add a TxnMonitor. The monitor will be notified of all transaction events until it is dropped.
    DGNPLATFORM_EXPORT static void AddTxnMonitor(TxnMonitor& monitor);
    DGNPLATFORM_EXPORT static void DropTxnMonitor(TxnMonitor& monitor);
    DGNPLATFORM_EXPORT void DeleteAllTxns();
    void DeleteFromStartTo(TxnId lastId); //!< @private
    void DeleteRebases(int64_t lastRebaseId); //!< @private
    DGNPLATFORM_EXPORT void DeleteReversedTxns(); //!< @private
    void OnBeginValidate(); //!< @private
    void OnEndValidate(); //!< @private
    void AddTxnTable(DgnDomain::TableHandler*);//!< @private
    DGNPLATFORM_EXPORT TxnManager(DgnDbR); //!< @private
    DGNPLATFORM_EXPORT BeSQLite::DbResult InitializeTableHandlers(); //!< @private
    TxnRelationshipLinkTables& GetRelationshipLinkTables() { return m_rlt; }//!< @private

    DGNPLATFORM_EXPORT static void SetOnCommitCallback(T_OnCommitCallback);

    //! A statement cache exclusively for Txn-based statements.
    BeSQLite::CachedStatementPtr GetTxnStatement(Utf8CP sql) const;

    bool IsIndirectChanges() { return m_indirectChanges; }
    bool HasFatalError() {return m_fatalValidationError;}
    int NumValidationErrors() {return m_txnErrors;}
    void LogError(bool fatal) { ++m_txnErrors; m_fatalValidationError |= fatal;}
    DGNPLATFORM_EXPORT void ReportError(bool fatal, Utf8CP errorType, Utf8CP msg);

    DGNPLATFORM_EXPORT BentleyStatus AddChildPropagatesChangesToParentRelationship(Utf8StringCR schemaName,Utf8StringCR relClassName);
    bvector<ECN::ECClassId> const& GetChildPropagatesChangesToParentRelationships() const {return m_childPropagatesChangesToParentRels;}

    //! Get the dgn_TxnTable::Element TxnTable for this TxnManager
    DGNPLATFORM_EXPORT dgn_TxnTable::Element& Elements() const;

    //! Get the dgn_TxnTable::ElementDep TxnTable for this TxnManager
    DGNPLATFORM_EXPORT dgn_TxnTable::ElementDep& ElementDependencies() const;

    //! Get the dgn_TxnTable::Model TxnTable for this TxnManager
    DGNPLATFORM_EXPORT dgn_TxnTable::Model& Models() const;

    //! Get the description of a previously committed Txn, given its TxnId.
    DGNPLATFORM_EXPORT Utf8String GetTxnDescription(TxnId txnId) const;

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
    DGNPLATFORM_EXPORT DgnDbStatus BeginMultiTxnOperation();

    //! End a multi-Txn operation
    DGNPLATFORM_EXPORT DgnDbStatus EndMultiTxnOperation();

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

    //! Returns true if the TxnManager is in the process of abandoning txns. This can be called from inside TxnTable methods only,
    //! otherwise it will always return false
    bool IsInAbandon() const { return TxnAction::Abandon == m_action; }

    //! Get the TxnId of the current Txn.
    //! @return the current TxnId. This value can be saved and later used to reverse changes that happen after this time.
    //! @see   ReverseTo CancelTo
    TxnId GetCurrentTxnId() const {return m_curr;}

    //! @private - query the ID of the last rebase blob stored by MergeRevision. Called by unit tests.
    DGNPLATFORM_EXPORT int64_t QueryLastRebaseId();

    //! @private - adds to `rebaser` all stored rebases up to and including `thruId`.
    BeSQLite::DbResult LoadRebases(BeSQLite::Rebaser& rebaser, int64_t thruId);

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
    bool IsUndoPossible() const {return GetSessionStartId() < GetCurrentTxnId();}

    //! Query if there are currently any reinstateable (redoable) changes
    //! @return True if there are currently any reinstate-able (redoable) changes
    bool IsRedoPossible() const {return !m_reversedTxn.empty();}

    // return true if there are any non-reversed txns in the db
    DGNPLATFORM_EXPORT bool HasPendingTxns() const;

    //! Returns true if there are uncommitted OR committed changes.
    bool HasLocalChanges() const {return HasChanges() || HasPendingTxns();}

    //! Reverse (undo) the most recent operation(s).
    //! @param[in] numOperations the number of operations to reverse. If this is greater than 1, the entire set of operations will
    //! be reinstated together when/if ReinstateTxn is called.
    //! @note If there are any outstanding uncommitted changes, they are reversed.
    //! @note The term "operation" is used rather than Txn, since multiple Txns can be grouped together via BeginMultiTxnOperation. So,
    //! even if numOperations is 1, multiple Txns may be reversed if they were grouped together when they were made.
    //! @note If numOperations is too large only the operations are reversible are reversed.
    DGNPLATFORM_EXPORT DgnDbStatus ReverseTxns(int numOperations);

    //! Reverse the most recent operation.
    DgnDbStatus ReverseSingleTxn() {return ReverseTxns(1);}

    //! Reverse all changes back to the beginning of the session.
    //! @param[in] prompt display a warning the user, and give an opportunity to cancel.
    DGNPLATFORM_EXPORT DgnDbStatus ReverseAll();

    //! Reverse all changes back to a previously saved TxnId.
    //! @param[in] txnId a TxnId obtained from a previous call to GetCurrentTxnId.
    //! @param[in] allowCrossSessions if No, don't reverse any Txns older than the beginning of the current session.
    //! @return DgnDbStatus::Success if the transactions were reversed, error status otherwise.
    //! @see  GetCurrentTxnId CancelTo
    DGNPLATFORM_EXPORT DgnDbStatus ReverseTo(TxnId txnId);

    //! Reverse and then cancel (make non-reinstatable) all changes back to a previous TxnId.
    //! @param[in] txnId a TxnId obtained from a previous call to GetCurrentTxnId.
    //! @param[in] allowCrossSessions if No, don't cancel any Txns older than the beginning of the current session.
    //! @return DgnDbStatus::Success if the transactions were reversed and cleared, error status otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus CancelTo(TxnId txnId);

    //! Reinstate the most recently reversed transaction. Since at any time multiple transactions can be reversed, it
    //! may take multiple calls to this method to reinstate all reversed operations.
    //! @return DgnDbStatus::Success if a reversed transaction was reinstated, error status otherwise.
    //! @note If there are any outstanding uncommitted changes, they are reversed before the Txn is reinstated.
    DGNPLATFORM_EXPORT DgnDbStatus ReinstateTxn();
    //@}

    //! Get the DgnDb for this TxnManager
    DgnDbR GetDgnDb() {return m_dgndb;}

    //! Pushes a dynamic operation onto the stack.
    //! When operating in dynamics, only temporary changes can be made to the DgnDb.
    //! This is useful for implementing tools, among other things.
    //! During a dynamic operation:
    //!  - Invoking undo or redo will roll back any dynamic changes
    //!  - Attempting to begin or end a multi-txn operation will produce an error
    //! Dynamic operations can be nested.
    DGNPLATFORM_EXPORT void BeginDynamicOperation();

    //! Pops the current dynamic transaction from the top of the stack, reverting all temporary changes made during the transaction
    //! A DynamicTxnProcessor may be supplied to capture the results of the dynamic txn
    //! In that case, if any changes exist in the current dynamic operation:
    //!  - Any indirect changes resulting from the dynamic changes will be computed; then
    //!  - The change processor will be invoked
    //! In either case, all changes made since the most recent call to BeginDynamicOperation will be rolled back before the function returns.
    DGNPLATFORM_EXPORT void EndDynamicOperation(DynamicTxnProcessor* processor=nullptr);

    //! Returns true if a dynamic transaction is in progress.
    bool InDynamicTxn() const {return !m_dynamicTxns.empty();}

    //! Tell the TxnManager to track changes to instances of the specified ECRelationship class.
    //! Relationship-specific changes will be captured in the Txn summary in different ways, depending on how the relationship was mapped.
    //! Specifically:
    //!     * Changes to ECRelationships that are mapped to foreign keys will result in element table changes only. The Txn summary will not have any separate record of a change to the ECRelationship.
    //!     * Changes to all ECRelationships that are mapped to link tables will be gathered into the TxnRelationshipLinkTables table in the Txn summary.
    //! @see EndTrackingRelationship
    //! @see TxnManager::AddTxnMonitor
    DGNPLATFORM_EXPORT void BeginTrackingRelationship(ECN::ECClassCR relClass);

#ifdef DEBUG_TxnManager_TXNS
    DGNPLATFORM_EXPORT void DumpTxns(bool verbose);
#endif

    DGNPLATFORM_EXPORT static uint64_t GetMaxReasonableTxnSize();

    void OnGeometryGuidChanges(bset<DgnModelId> const&);
};

//=======================================================================================
//! ChangeTracker used by TxnManager to process dynamic operations.
// @bsiclass
//=======================================================================================
struct DynamicChangeTracker : BeSQLite::ChangeTracker
{
private:
    TxnManager& m_txnMgr;
    DynamicChangeTracker(TxnManager& txnMgr) : m_txnMgr(txnMgr) {}
    ~DynamicChangeTracker() {}

    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override;
    TrackChangesForTable _FilterTable(Utf8CP tableName) override;
public:
    static DynamicChangeTrackerPtr Create(TxnManager& txnMgr);
};

//=======================================================================================
//! @namespace BentleyApi::Dgn::dgn_TxnTable TxnTables in the base "Dgn" domain.
//! @note Only handlers from the base "Dgn" domain belong in this namespace.
// @bsiclass
//=======================================================================================
namespace dgn_TxnTable
{
    struct Element : TxnTable
    {
        BeSQLite::Statement m_stmt;
        bool m_changes;
        bool m_haveIndexOnECClassId;
        static Utf8CP MyTableName() {return BIS_TABLE(BIS_CLASS_Element);}
        Utf8CP _GetTableName() const override {return MyTableName();}

        Element(TxnManager& mgr) : TxnTable(mgr), m_changes(false), m_haveIndexOnECClassId(false) {}

        void _Initialize() override;
        void _OnValidate() override;
        void _OnValidateAdd(BeSQLite::Changes::Change const& change) override    {AddChange(change, TxnTable::ChangeType::Insert, true);}
        void _OnValidateDelete(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Delete, true);}
        void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Update, true);}
        void _OnValidated() override;
        void _OnApply() override;
        void _OnAppliedAdd(BeSQLite::Changes::Change const&) override;
        void _OnAppliedDelete(BeSQLite::Changes::Change const&) override;
        void _OnAppliedUpdate(BeSQLite::Changes::Change const&) override;
        void _OnApplied() override;

        void AddChange(BeSQLite::Changes::Change const& change, ChangeType changeType, bool fromCommit);
        void AddElement(DgnElementId, DgnModelId, ChangeType changeType, DgnClassId, bool fromCommit);

        //! iterator for elements that are directly changed. Only valid during _PropagateChanges.
        struct Iterator : BeSQLite::DbTableIterator
        {
        public:
            Iterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR)db) {}
            struct Entry : DbTableIterator::Entry
            {
                using iterator_category=std::input_iterator_tag;
                using value_type=Entry const;
                using difference_type=std::ptrdiff_t;
                using pointer=Entry const*;
                using reference=Entry const&;

            private:
                friend struct Iterator;
                Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}

            public:
                DGNPLATFORM_EXPORT DgnModelId GetModelId() const;
                DGNPLATFORM_EXPORT DgnElementId GetElementId() const;
                DGNPLATFORM_EXPORT ChangeType GetChangeType() const;
                DGNPLATFORM_EXPORT DgnClassId GetECClassId() const;
                Entry const& operator*() const {return *this;}
            };

            typedef Entry const_iterator;
            typedef Entry iterator;
            DGNPLATFORM_EXPORT const_iterator begin() const;
            const_iterator end() const {return Entry(nullptr, false);}
        };

    bool HasChanges() const {return m_changes;}
    Iterator MakeIterator() const {return Iterator(m_txnMgr.GetDgnDb());}
    //! Make sure that the Elements table is index on the ECClassId column. Call this if you plan to query for changes by ECClassId repeatedly.
    void CreateIndexOnECClassId();
    };

    struct Geometric : TxnTable {
        Geometric(TxnManager& mgr) : TxnTable(mgr) {}
        int32_t GetFirstCol() {return 2;}
        // the last columns that hold information that indicates a geometric change
        virtual int32_t _GetLastCol() = 0;

        bool HasChangeInColumns(BeSQLite::Changes::Change const& change);
        void AddChange(BeSQLite::Changes::Change const& change, ChangeType changeType, bool fromCommit);

        void _OnValidateAdd(BeSQLite::Changes::Change const& change) override    {AddChange(change, TxnTable::ChangeType::Insert, true);}
        void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Update, true);}
        void _OnValidateDelete(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Delete, true);}
        void _OnAppliedAdd(BeSQLite::Changes::Change const& change) override    {AddChange(change, TxnTable::ChangeType::Insert, false);}
        void _OnAppliedUpdate(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Update, false);}
        void _OnAppliedDelete(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Delete, false);}
    };

    struct Geometric3d : Geometric {
        virtual int32_t _GetLastCol() override {return 16; };
        Geometric3d(TxnManager& mgr) : Geometric(mgr) {}
        static Utf8CP MyTableName() {return BIS_TABLE(BIS_CLASS_GeometricElement3d);}
        Utf8CP _GetTableName() const override {return MyTableName();}
    };

    struct Geometric2d : Geometric {
        virtual int32_t _GetLastCol() override {return 10; };
        Geometric2d(TxnManager& mgr) : Geometric(mgr) {}
        static Utf8CP MyTableName() {return BIS_TABLE(BIS_CLASS_GeometricElement2d);}
        Utf8CP _GetTableName() const override {return MyTableName();}
    };

    struct Model : TxnTable
    {
    private:
        bset<DgnModelId> m_geometryGuidChanges;
        BeSQLite::Statement m_isGeometricModelStmt;
        int m_geometryGuidColumnIndex = -1;

        bool HasGeometryGuid() const { return -1 != m_geometryGuidColumnIndex; }
        bool IsGeometryGuidChanged(DgnModelId, BeSQLite::Changes::Change const&);
    public:
        BeSQLite::Statement m_stmt;
        bool m_changes;

        Model(TxnManager& mgr) : TxnTable(mgr), m_changes(false) {}
        static Utf8CP MyTableName() {return BIS_TABLE(BIS_CLASS_Model);}
        Utf8CP _GetTableName() const override {return MyTableName();}

        void _Initialize() override;
        void _OnValidate() override;
        void _OnValidateAdd(BeSQLite::Changes::Change const& change) override    {AddChange(change, TxnTable::ChangeType::Insert);}
        void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Update);}
        void _OnValidateDelete(BeSQLite::Changes::Change const& change) override {AddChange(change, TxnTable::ChangeType::Delete);}
        void _OnValidated() override;
        void _OnApply() override;
        void _OnAppliedAdd(BeSQLite::Changes::Change const&) override;
        void _OnAppliedDelete(BeSQLite::Changes::Change const&) override;
        void _OnAppliedUpdate(BeSQLite::Changes::Change const&) override;
        void _OnApplied() override;

        void AddChange(BeSQLite::Changes::Change const& change, ChangeType changeType);

        //! iterator for models that are directly changed. Only valid during _PropagateChanges.
        struct Iterator : BeSQLite::DbTableIterator
        {
            Iterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR)db) {}
            struct Entry : DbTableIterator::Entry
            {
                using iterator_category=std::input_iterator_tag;
                using value_type=Entry const;
                using difference_type=std::ptrdiff_t;
                using pointer=Entry const*;
                using reference=Entry const&;

            private:
                friend struct Iterator;
                Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}
            public:
                DGNPLATFORM_EXPORT DgnModelId GetModelId() const;
                DGNPLATFORM_EXPORT ChangeType GetChangeType() const;
                DGNPLATFORM_EXPORT DgnClassId GetECClassId() const;
                Entry const& operator*() const {return *this;}
            };

            typedef Entry const_iterator;
            typedef Entry iterator;
            DGNPLATFORM_EXPORT const_iterator begin() const;
            const_iterator end() const {return Entry(nullptr, false);}
        };

        Iterator MakeIterator() const {return Iterator(m_txnMgr.GetDgnDb());}
        bool HasChanges() const {return m_changes;}

        void NotifyGeometryChanges();
        void ClearGeometryChanges() { m_geometryGuidChanges.clear(); }
    };

    struct ElementDep : TxnTable {
        struct DepRelData {
            BeSQLite::EC::ECInstanceKey m_relKey;
            DgnElementId m_source, m_target;
            bool m_isEDE;

            DepRelData(BeSQLite::EC::ECInstanceKey const& key, DgnElementId s, DgnElementId t, bool isEDE) : m_relKey(key), m_source(s), m_target(t), m_isEDE(isEDE) { ; }
        };

        BeSQLite::Statement m_stmt;
        DgnElementIdSet m_failedTargets;
        bvector<DepRelData> m_deletedRels;
        bool m_changes;
        static Utf8CP MyTableName() { return BIS_TABLE(BIS_REL_ElementDrivesElement); }
        Utf8CP _GetTableName() const override { return MyTableName(); }

        ElementDep(TxnManager& mgr) : TxnTable(mgr), m_changes(false) {}
        void _Initialize() override;
        void _OnValidate() override;
        void _OnValidateAdd(BeSQLite::Changes::Change const& change) override { UpdateSummary(change, TxnTable::ChangeType::Insert); }
        void _OnValidateDelete(BeSQLite::Changes::Change const& change) override { UpdateSummary(change, TxnTable::ChangeType::Delete); }
        void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override { UpdateSummary(change, TxnTable::ChangeType::Update); }
        void _PropagateChanges() override;
        void _OnValidated() override;

        void UpdateSummary(BeSQLite::Changes::Change change, ChangeType changeType);
        void AddDependency(BeSQLite::EC::ECInstanceId const&, ChangeType);
        void AddFailedTarget(DgnElementId id) { m_failedTargets.insert(id); }
        DgnElementIdSet const& GetFailedTargets() const { return m_failedTargets; }
        bool HasChanges() const { return m_changes; }
    };

    //! @private
    struct RelationshipLinkTable : TxnTable
    {
        friend struct Dgn::TxnManager;
    protected:
        bool m_changes;
        BeSQLite::Statement m_stmt;
        Utf8String m_tableName;

        Utf8CP _GetTableName() const override { return m_tableName.c_str();}
        RelationshipLinkTable(TxnManager& mgr, Utf8CP tableName): TxnTable(mgr), m_changes(false), m_tableName(tableName) {}
        virtual void _UpdateSummary(BeSQLite::Changes::Change change, ChangeType changeType) = 0;

        void _OnValidate() override { m_changes = false; }
        void _OnValidateAdd(BeSQLite::Changes::Change const& change) override {_UpdateSummary(change, TxnTable::ChangeType::Insert);}
        void _OnValidateDelete(BeSQLite::Changes::Change const& change) override {_UpdateSummary(change, TxnTable::ChangeType::Delete);}
        void _OnValidateUpdate(BeSQLite::Changes::Change const& change) override {_UpdateSummary(change, TxnTable::ChangeType::Update);}
        void _PropagateChanges() override {}
        void _OnValidated() override;

        BeSQLite::DbResult QueryTargets(DgnElementId& srcelemid, DgnElementId& tgtelemid, BeSQLite::EC::ECInstanceId relid);

      public:
        bool HasChanges() const {return m_changes;}
    };

    //! @private
    struct UniqueRelationshipLinkTable : RelationshipLinkTable
    {
        friend struct Dgn::TxnManager;
    protected:
        UniqueRelationshipLinkTable(TxnManager& mgr, Utf8CP tableName) : RelationshipLinkTable(mgr, tableName) {}
        void _UpdateSummary(BeSQLite::Changes::Change change, ChangeType changeType) override;
    };
};

//=======================================================================================
//! @ingroup GROUP_TxnManager
//! @namespace BentleyApi::Dgn::dgn_TableHandler TableHandlers in the base "Dgn" domain.
//! @note Only handlers from the base "Dgn" domain belong in this namespace.
// @bsiclass
//=======================================================================================
namespace dgn_TableHandler {
    struct Element : Dgn::DgnDomain::TableHandler {
        TABLEHANDLER_DECLARE_MEMBERS(Element, DGNPLATFORM_EXPORT)
        Dgn::TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::Element(mgr);}
    };

    struct Geometric3d : Dgn::DgnDomain::TableHandler {
        TABLEHANDLER_DECLARE_MEMBERS(Geometric3d, DGNPLATFORM_EXPORT)
        Dgn::TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::Geometric3d(mgr);}
    };

    struct Geometric2d : Dgn::DgnDomain::TableHandler {
        TABLEHANDLER_DECLARE_MEMBERS(Geometric2d, DGNPLATFORM_EXPORT)
        Dgn::TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::Geometric2d(mgr);}
    };

    struct Model : DgnDomain::TableHandler {
        TABLEHANDLER_DECLARE_MEMBERS(Model, DGNPLATFORM_EXPORT)
        TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::Model(mgr);}
    };

    struct ElementDep : DgnDomain::TableHandler {
        TABLEHANDLER_DECLARE_MEMBERS(ElementDep, DGNPLATFORM_EXPORT)
        TxnTable* _Create(TxnManager& mgr) const override {return new dgn_TxnTable::ElementDep(mgr);}
    };
};

END_BENTLEY_DGN_NAMESPACE
