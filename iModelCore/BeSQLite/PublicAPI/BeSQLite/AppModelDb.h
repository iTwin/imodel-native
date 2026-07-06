/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "BeSQLite.h"
#include "ChangeSet.h"

//! be_Prop namespace under which the AppModel profile properties are stored.
#define APPMODEL_PROPSPEC_NAMESPACE "appmodel_Db"

//! Macros for the BentleyApi::BeSQLite::AppModel namespace, following the same pattern EC uses for
//! BentleyApi::BeSQLite::EC (see BEGIN_BENTLEY_SQLITE_EC_NAMESPACE).
#define BEGIN_BENTLEY_APPMODEL_NAMESPACE   BEGIN_BENTLEY_SQLITE_NAMESPACE namespace AppModel {
#define END_BENTLEY_APPMODEL_NAMESPACE     } END_BENTLEY_SQLITE_NAMESPACE
#define USING_NAMESPACE_BENTLEY_APPMODEL   using namespace BentleyApi::BeSQLite::AppModel;

BEGIN_BENTLEY_APPMODEL_NAMESPACE

//=======================================================================================
//! Version digits for the AppModel profile.
//! @see AppModelDb::CurrentProfileVersion, AppModelDb::MinimumUpgradableProfileVersion
// @bsiclass
//=======================================================================================
enum SchemaValues
    {
    APPMODEL_CURRENT_VERSION_Major = 1,
    APPMODEL_CURRENT_VERSION_Minor = 0,
    APPMODEL_CURRENT_VERSION_Sub1  = 0,
    APPMODEL_CURRENT_VERSION_Sub2  = 0,

    //! Oldest version of the AppModel profile that the current API can still upgrade in-situ.
    APPMODEL_SUPPORTED_VERSION_Major = APPMODEL_CURRENT_VERSION_Major,
    APPMODEL_SUPPORTED_VERSION_Minor = APPMODEL_CURRENT_VERSION_Minor,
    APPMODEL_SUPPORTED_VERSION_Sub1  = 0,
    APPMODEL_SUPPORTED_VERSION_Sub2  = 0,
    };

//=======================================================================================
//! The kind of change recorded by a single AppModel txn (see APPMODEL_TABLE_Txns.Type).
// @bsiclass
//=======================================================================================
enum class TxnType
    {
    Data = 0,   //!< The txn contains data changes only.
    Schema = 1, //!< The txn contains schema (DDL) changes, possibly along with data changes.
    };

//=======================================================================================
//! The kind of change carried by an AppModel changeset file.
// @bsiclass
//=======================================================================================
enum class ChangesetType
    {
    Regular = 0, //!< Contains data changes only.
    Schema  = 1, //!< Contains schema (DDL) changes (possibly along with data changes).
    };

struct AppModelDb;

//=======================================================================================
//! Properties that describe an AppModel changeset file: its SHA1 id, the id of its parent
//! changeset, the guid of the db it originated from, the on-disk file, and its kind.
//!
//! The changeset id is a SHA1 hash computed from the parent changeset id and the contents of the
//! changeset file, exactly as iModel changesets are identified. This lets a changeset be validated
//! (its id recomputed and compared) before it is merged, and forms a parent/child chain of changesets.
// @bsiclass
//=======================================================================================
struct ChangesetProps
    {
    Utf8String m_id;
    Utf8String m_parentId;
    Utf8String m_dbGuid;
    BeFileName m_fileName;
    ChangesetType m_changesetType;

    ChangesetProps() : m_changesetType(ChangesetType::Regular) {}
    ChangesetProps(Utf8StringCR id, Utf8StringCR parentId, Utf8StringCR dbGuid, BeFileNameCR fileName, ChangesetType changesetType)
        : m_id(id), m_parentId(parentId), m_dbGuid(dbGuid), m_fileName(fileName), m_changesetType(changesetType) {}

    Utf8StringCR GetChangesetId() const { return m_id; }
    Utf8StringCR GetParentId() const { return m_parentId; }
    Utf8StringCR GetDbGuid() const { return m_dbGuid; }
    BeFileNameCR GetFileName() const { return m_fileName; }
    ChangesetType GetChangesetType() const { return m_changesetType; }
    bool ContainsSchemaChanges() const { return m_changesetType == ChangesetType::Schema; }

    //! Verify that this changeset originated from @p db (matching db guid) and that its id is the correct
    //! SHA1 of its parent id + file contents.
    //! @return BE_SQLITE_OK if valid; BE_SQLITE_MISMATCH if the db guid differs; BE_SQLITE_ERROR_InvalidChangeSetVersion
    //! if the id does not match the file; BE_SQLITE_ERROR if the file is missing/corrupt.
    BE_SQLITE_EXPORT DbResult ValidateContent(AppModelDb const& db) const;

    //! Compute the SHA1 changeset id from a parent changeset id and an AppModel changeset file.
    //! @param[in] parentId the parent changeset id (empty for the first changeset), or a 40-char SHA1 hex string.
    //! @param[in] changesetFile the AppModel changeset file whose contents are hashed.
    //! @return the 40-char SHA1 hex id, or an empty string if the file could not be read.
    BE_SQLITE_EXPORT static Utf8String ComputeChangesetId(Utf8StringCR parentId, BeFileNameCR changesetFile);
    };

struct Txns;

//=======================================================================================
//! A BeSQLite @ref Db that carries an "AppModel" profile on top of the base BeSQLite profile.
//!
//! %AppModelDb manages its profile the same way ECDb does. When a new file is created, the current
//! AppModel profile version is stamped into the be_Prop table and the AppModel's own set of tables
//! is created. When an existing file is opened, the stored profile version is validated against the
//! version expected by the software and, if the file is older but upgradable, it is upgraded in place
//! by a sequence of internal profile migrators.
//!
//! The AppModel schema (the set of tables) and the profile migrators are internal to this
//! implementation - consumers simply create/open an %AppModelDb and get a fully set-up file.
//!
//! @note %AppModelDb currently lives in BeSQLite. It is expected to move into its own component in the
//! future, so it deliberately depends only on the base BeSQLite @ref Db API.
//!
//! @ingroup GROUP_BeSQLite
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AppModelDb : Db
{
private:
    RefCountedPtr<Txns> m_txns;

    void SetupChangeTracking();

protected:
    //! @name Profile hooks (forwarded from Db)
    //! @{
    BE_SQLITE_EXPORT DbResult _OnDbCreated(CreateParams const& params) override;
    BE_SQLITE_EXPORT DbResult _OnDbOpened(OpenParams const& params) override;
    BE_SQLITE_EXPORT void _OnDbClose() override;
    BE_SQLITE_EXPORT ProfileState _CheckProfileVersion() const override;
    BE_SQLITE_EXPORT DbResult _UpgradeProfile(OpenParams const& params) override;
    //! @}

public:
    BE_SQLITE_EXPORT AppModelDb();
    BE_SQLITE_EXPORT ~AppModelDb();

    //! The AppModel profile version expected by this build of the software.
    static ProfileVersion CurrentProfileVersion() { return ProfileVersion(APPMODEL_CURRENT_VERSION_Major, APPMODEL_CURRENT_VERSION_Minor, APPMODEL_CURRENT_VERSION_Sub1, APPMODEL_CURRENT_VERSION_Sub2); }

    //! The oldest AppModel profile version that can still be upgraded in-situ to the current version.
    //! Files with an older version cannot be upgraded.
    static ProfileVersion MinimumUpgradableProfileVersion() { return ProfileVersion(APPMODEL_SUPPORTED_VERSION_Major, APPMODEL_SUPPORTED_VERSION_Minor, APPMODEL_SUPPORTED_VERSION_Sub1, APPMODEL_SUPPORTED_VERSION_Sub2); }

    //! be_Prop spec under which the current AppModel profile version is stored.
    static PropertySpec ProfileVersionProperty() { return PropertySpec("SchemaVersion", APPMODEL_PROPSPEC_NAMESPACE); }
    //! be_Prop spec under which the AppModel profile version that the file was originally created with is stored.
    static PropertySpec InitialProfileVersionProperty() { return PropertySpec("InitialSchemaVersion", APPMODEL_PROPSPEC_NAMESPACE); }

    //! Gets the AppModel profile version stored in this (open) file.
    //! @param[out] version the profile version read from the file.
    //! @return BE_SQLITE_OK if the version was read, BE_SQLITE_ERROR_InvalidProfileVersion if the file has no AppModel profile.
    BE_SQLITE_EXPORT DbResult QueryProfileVersion(ProfileVersion& version) const;

    //! The change tracker for this db. All change-tracking, changeset creation and merge operations live on
    //! Txns, e.g. GetTxns().BeginCreateChangeset(...) and GetTxns().MergeChangeset(...).
    //! @note The tracker records changes only when the db is open read-write; on a readonly db it exists but
    //! tracking is disabled.
    BE_SQLITE_EXPORT Txns& GetTxns();
};

//=======================================================================================
//! Change tracker for an AppModelDb. Records local changes and, on each SaveChanges that produced changes,
//! writes a single row into the APPMODEL_TABLE_Txns table capturing both the DDL (schema) changes and the data
//! changes. It also creates AppModel changeset files from the recorded txns and merges changeset files from
//! other AppModelDbs.
//!
//! @note This tracker does not implement undo/redo. Recorded txns are a durable log used to produce changeset
//! files; they are not reversed/reinstated.
//!
//! @note Schema (DDL) changes are assumed to be *additive only* (for example adding a table or a column, never
//! dropping or redefining one), and the application is expected to perform DDL only during a migration step that
//! adds a new feature - never as part of normal, changeset-tracked data operations. Because all recorded txns are
//! combined into a single changeset, changing a table's column count in between two data changes to that same
//! table within one changeset window is not supported; the additive + migration-only-DDL assumptions keep normal
//! operation clear of that case.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE Txns : ChangeTracker
    {
private:
    Utf8String m_pendingProps;
    SnappyToBlob m_snappyToBlob; //!< reused compressor for writing txn change blobs (mirrors TxnManager::m_snappyTo)
    Utf8String m_creatingChangesetId; //!< id of the changeset produced by BeginCreateChangeset, finalized by EndCreateChangeset
    int64_t m_creatingChangesetLastTxnId = 0; //!< highest APPMODEL_TABLE_Txns.Id captured by BeginCreateChangeset (0 = none in progress)

    AppModelDb& GetAppModelDb() const { return *static_cast<AppModelDb*>(const_cast<Txns*>(this)->GetDb()); }

    DbResult WriteTxn(Utf8CP description, DdlChanges const& ddlChanges, ChangeSet const& dataChanges);
    DbResult ReadTxnDataChanges(ChangeSet& dataChanges, int64_t txnId) const;

    //! Apply the DDL and data changes from an AppModel changeset file to this db, without recording them as
    //! new txns (change tracking is suspended for the duration). Conflicts abort the apply (see ChangesetReader).
    DbResult ApplyChangesetFile(BeFileNameCR pathname);

    //! Set the id of this db's current (parent) changeset. Private: it is advanced by MergeChangeset once a
    //! changeset has been successfully applied. Stored in be_Local (which is excluded from change tracking).
    DbResult SetParentChangesetId(Utf8StringCR changesetId);

protected:
    //! Skip be_Local (redundant/derived) and the AppModel txns table itself (writing txns must not be tracked).
    BE_SQLITE_EXPORT TrackChangesForTable _FilterTable(Utf8CP tableName) override;

    //! On commit, capture the tracked DDL + data changes and store them as one row in APPMODEL_TABLE_Txns.
    //! @note @p operation is the description passed to Db::SaveChanges; it is stored as the txn's Description.
    BE_SQLITE_EXPORT OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override;

public:
    //! Construct a tracker for @p db and register it as the db's change tracker.
    BE_SQLITE_EXPORT explicit Txns(AppModelDb& db);

    Txns(Txns const&) = delete;
    Txns& operator=(Txns const&) = delete;
    Txns(Txns&&) = delete;
    Txns& operator=(Txns&&) = delete;

    //! Stage a JSON properties string to store on the next committed txn. Cleared after the txn is written.
    void SetTxnProps(Utf8CP propsJson) { m_pendingProps.AssignOrClear(propsJson); }

    //! Determine whether any txns have been recorded in this file (i.e. there are rows in APPMODEL_TABLE_Txns).
    BE_SQLITE_EXPORT bool HasPendingTxns() const;

    //! The id of this db's current changeset ("" if none yet). This is the parent id used when creating or
    //! validating the next changeset.
    BE_SQLITE_EXPORT Utf8String GetParentChangesetId() const;

    //! Begin creating an AppModel changeset: combine all recorded txns into a single AppModel changeset file that
    //! can be applied to another AppModelDb, and fill @p props with the changeset's properties: its parent changeset
    //! id (this db's current changeset id), a freshly computed SHA1 changeset id, this db's guid, the file, and the
    //! changeset type.
    //! @note This only writes the file; it does not yet remove the captured txns or advance this db's changeset id.
    //! Call EndCreateChangeset once the changeset file has been safely persisted to finalize the operation.
    //! @param[out] props the properties of the created changeset.
    //! @param[in] pathname the full path of the changeset file to create.
    //! @return BE_SQLITE_OK on success, or an error if there are no txns or the file could not be written.
    BE_SQLITE_EXPORT DbResult BeginCreateChangeset(ChangesetProps& props, BeFileNameCR pathname);

    //! Finalize the changeset started by BeginCreateChangeset: delete the txns that were captured into it (so the next
    //! changeset contains only subsequent changes) and advance this db's current changeset id to the newly created
    //! changeset (so the next changeset chains onto it), then commit. Txns recorded after BeginCreateChangeset are
    //! preserved. Call this only after the changeset file produced by BeginCreateChangeset has been safely persisted.
    //! @return BE_SQLITE_OK on success; BE_SQLITE_ERROR if there is no changeset creation in progress.
    BE_SQLITE_EXPORT DbResult EndCreateChangeset();

    //! Apply (merge) an AppModel changeset onto this db. First validates that the changeset originated from this
    //! db (matching guid), that its parent id matches this db's current changeset id, and that its SHA1 id is
    //! correct. The changes are applied without being recorded as new txns; any conflict aborts the merge. On
    //! success this db's parent changeset id is advanced to the merged changeset's id and the change is committed.
    //! @param[in] props the properties of the changeset to merge (its m_fileName must point at the file).
    //! @return BE_SQLITE_OK on success; BE_SQLITE_MISMATCH if it did not originate from this db or its parent does
    //! not match; BE_SQLITE_ERROR_InvalidChangeSetVersion if its id is incorrect; error otherwise.
    BE_SQLITE_EXPORT DbResult MergeChangeset(ChangesetProps const& props);
    };

END_BENTLEY_APPMODEL_NAMESPACE
