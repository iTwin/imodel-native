/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/AppModelDb.h>
#include <BeSQLite/ChangesetFile.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/Logging.h>
#include <Bentley/SHA1.h>
#include <vector>
#include <memory>

USING_NAMESPACE_BENTLEY_LOGGING
#define LOG CategoryLogger("BeSQLite")
#define PROFILENAME "AppModel"

BEGIN_BENTLEY_APPMODEL_NAMESPACE

//=======================================================================================
// AppModel table names.
//=======================================================================================
#define APPMODEL_TABLE_Dgns                   "Dgns"
#define APPMODEL_TABLE_DgnStreams             "DgnStreams"
#define APPMODEL_TABLE_Models                 "Models"
#define APPMODEL_TABLE_Elements               "Elements"
#define APPMODEL_TABLE_DgnReferencesModels    "DgnReferencesModels"
#define APPMODEL_TABLE_ModelUsesDgnLibElement "ModelUsesDgnLibElement"
#define APPMODEL_TABLE_Schemas                "Schemas"
#define APPMODEL_TABLE_DgnContainsSchemas     "DgnContainsSchemas"
#define APPMODEL_TABLE_SchemaClasses          "SchemaClasses"
#define APPMODEL_TABLE_RasterImages           "RasterImages"
#define APPMODEL_TABLE_Txns                   "appmodel_txns"

//=======================================================================================
// Centralized DDL for the AppModel tables. Each macro contains the CREATE TABLE statement
// followed by its indexes so that the DDL can be shared between profile creation and any
// future profile migrators that need to add the table to an existing file.
//=======================================================================================
#define TABLEDDL_Dgns "CREATE TABLE " APPMODEL_TABLE_Dgns \
    "(ID INTEGER PRIMARY KEY AUTOINCREMENT," \
    "Name TEXT NOT NULL," \
    "Description TEXT," \
    "FileHeaderStream BLOB," \
    "SeedId INTEGER REFERENCES " APPMODEL_TABLE_Dgns "(ID)," \
    "IsDgnLib BOOLEAN NOT NULL);" \
    "CREATE INDEX idx_Dgns_SeedId ON " APPMODEL_TABLE_Dgns "(SeedId) WHERE SeedId IS NOT NULL;"

#define TABLEDDL_DgnStreams "CREATE TABLE " APPMODEL_TABLE_DgnStreams \
    "(ID INTEGER PRIMARY KEY," \
    "DgnId INTEGER NOT NULL REFERENCES " APPMODEL_TABLE_Dgns "(ID) ON DELETE CASCADE," \
    "StreamPath TEXT NOT NULL," \
    "Data BLOB NOT NULL," \
    "UNIQUE (DgnId, StreamPath));" \
    "CREATE INDEX idx_DgnStreams_DgnId_StreamPath ON " APPMODEL_TABLE_DgnStreams "(DgnId, StreamPath);"

#define TABLEDDL_Models "CREATE TABLE " APPMODEL_TABLE_Models \
    "(ID INTEGER PRIMARY KEY AUTOINCREMENT," \
    "DgnId INTEGER NOT NULL REFERENCES " APPMODEL_TABLE_Dgns "(ID) ON DELETE CASCADE," \
    "V8ModelId INTEGER NOT NULL," \
    "Type INTEGER NOT NULL," \
    "Name TEXT NOT NULL," \
    "Description TEXT," \
    "Is3d BOOLEAN NOT NULL," \
    "Gcs TEXT," \
    "UNIQUE (V8ModelId, DgnId));" \
    "CREATE INDEX idx_Models_Type ON " APPMODEL_TABLE_Models "(Type);" \
    "CREATE INDEX idx_Models_DgnId ON " APPMODEL_TABLE_Models "(DgnId);"

#define TABLEDDL_Elements "CREATE TABLE " APPMODEL_TABLE_Elements \
    "(ID INTEGER PRIMARY KEY AUTOINCREMENT," \
    "ModelId INTEGER NOT NULL REFERENCES " APPMODEL_TABLE_Models "(ID) ON DELETE CASCADE," \
    "V8ElementId INTEGER NOT NULL," \
    "Type INTEGER NOT NULL," \
    "ParentId INTEGER," \
    "DgnFragment BLOB NOT NULL," \
    "xattrs BLOB," \
    "iModelType TEXT," \
    "iModelFragment TEXT," \
    "TableKind INTEGER," \
    "TableScope INTEGER," \
    "TableEntryId INTEGER," \
    "UNIQUE (V8ElementId, ModelId, Type));" \
    "CREATE INDEX idx_Elements_ModelId ON " APPMODEL_TABLE_Elements "(ModelId);" \
    "CREATE INDEX idx_Elements_ParentId ON " APPMODEL_TABLE_Elements "(ParentId) WHERE ParentId IS NOT NULL;" \
    "CREATE INDEX idx_Elements_ModelId_Type ON " APPMODEL_TABLE_Elements "(ModelId, Type);"

#define TABLEDDL_DgnReferencesModels "CREATE TABLE " APPMODEL_TABLE_DgnReferencesModels \
    "(ID INTEGER PRIMARY KEY," \
    "DgnId INTEGER NOT NULL REFERENCES " APPMODEL_TABLE_Dgns "(ID) ON DELETE CASCADE," \
    "TargetModelId INTEGER NOT NULL REFERENCES " APPMODEL_TABLE_Models "(ID) ON DELETE CASCADE," \
    "IsVisible BOOLEAN NOT NULL," \
    "TranslateX DOUBLE NOT NULL DEFAULT 0.0," \
    "TranslateY DOUBLE NOT NULL DEFAULT 0.0," \
    "TranslateZ DOUBLE NOT NULL DEFAULT 0.0," \
    "Yaw DOUBLE NOT NULL DEFAULT 0.0," \
    "Pitch DOUBLE NOT NULL DEFAULT 0.0," \
    "Roll DOUBLE NOT NULL DEFAULT 0.0);" \
    "CREATE INDEX idx_DgnReferencesModels_DgnId ON " APPMODEL_TABLE_DgnReferencesModels "(DgnId);" \
    "CREATE INDEX idx_DgnReferencesModels_TargetModelId ON " APPMODEL_TABLE_DgnReferencesModels "(TargetModelId);"

#define TABLEDDL_ModelUsesDgnLibElement "CREATE TABLE " APPMODEL_TABLE_ModelUsesDgnLibElement \
    "(ID INTEGER PRIMARY KEY AUTOINCREMENT," \
    "LocalDgnElementId INTEGER NOT NULL," \
    "ModelId INTEGER NOT NULL REFERENCES " APPMODEL_TABLE_Models "(ID) ON DELETE CASCADE," \
    "DgnLibModelId INTEGER NOT NULL," \
    "DgnLibElementId INTEGER NOT NULL," \
    "FOREIGN KEY (DgnLibElementId) REFERENCES " APPMODEL_TABLE_Elements "(ID));" \
    "CREATE INDEX idx_ModelUsesDgnLibElement_ModelId ON " APPMODEL_TABLE_ModelUsesDgnLibElement "(ModelId);" \
    "CREATE INDEX idx_ModelUsesDgnLibElement_LibElement ON " APPMODEL_TABLE_ModelUsesDgnLibElement "(DgnLibModelId, DgnLibElementId);"

#define TABLEDDL_Schemas "CREATE TABLE " APPMODEL_TABLE_Schemas \
    "(ID INTEGER PRIMARY KEY AUTOINCREMENT," \
    "Name TEXT NOT NULL," \
    "Version TEXT NOT NULL," \
    "Alias TEXT NOT NULL," \
    "DisplayLabel TEXT," \
    "Checksum INTEGER NOT NULL," \
    "Xml TEXT NOT NULL," \
    "UNIQUE(Name, Version, Checksum));"

#define TABLEDDL_DgnContainsSchemas "CREATE TABLE " APPMODEL_TABLE_DgnContainsSchemas \
    "(DgnId INTEGER NOT NULL REFERENCES " APPMODEL_TABLE_Dgns "(ID) ON DELETE CASCADE," \
    "SchemaId INTEGER NOT NULL REFERENCES " APPMODEL_TABLE_Schemas "(ID) ON DELETE CASCADE," \
    "PRIMARY KEY (DgnId, SchemaId));"

#define TABLEDDL_SchemaClasses "CREATE TABLE " APPMODEL_TABLE_SchemaClasses \
    "(SchemaId INTEGER NOT NULL REFERENCES " APPMODEL_TABLE_Schemas "(ID) ON DELETE CASCADE," \
    "ClassName TEXT NOT NULL," \
    "IsPrimary BOOLEAN NOT NULL," \
    "PRIMARY KEY (SchemaId, ClassName));" \
    "CREATE INDEX idx_SchemaClasses_SchemaId ON " APPMODEL_TABLE_SchemaClasses "(SchemaId);"

#define TABLEDDL_RasterImages "CREATE TABLE " APPMODEL_TABLE_RasterImages \
    "(ID INTEGER PRIMARY KEY AUTOINCREMENT," \
    "Identifier TEXT NOT NULL," \
    "ImageData BLOB NOT NULL," \
    "UNIQUE(Identifier));"

// Change-tracking table. A row is written for each SaveChanges that produced changes.
// DDL and data changes are stored side-by-side in two columns (Ddl and Change) rather than
// as separate rows. Description and Props (JSON) hold caller-supplied metadata. This table is
// itself excluded from change tracking (see Txns::_FilterTable).
#define TABLEDDL_Txns "CREATE TABLE " APPMODEL_TABLE_Txns \
    "(Id INTEGER PRIMARY KEY," \
    "Type INTEGER NOT NULL," \
    "Description TEXT," \
    "Props TEXT," \
    "Ddl TEXT," \
    "Change BLOB," \
    "Time TIMESTAMP DEFAULT(julianday('now')));"

struct ProfileMigrator;

//=======================================================================================
//! Manages the AppModel profile of an AppModelDb: creating it in a new file, checking its
//! version when a file is opened, and upgrading it in place. Mirrors BeSQLiteProfileManager.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ProfileManager final
    {
private:
    ProfileManager() = delete;
    ~ProfileManager() = delete;

    //! Version of the AppModel profile expected by this build.
    static ProfileVersion GetExpectedVersion() { return AppModelDb::CurrentProfileVersion(); }
    //! Minimum version that can still be auto-upgraded to the latest profile version.
    static ProfileVersion GetMinimumSupportedVersion() { return AppModelDb::MinimumUpgradableProfileVersion(); }

    static DbResult CreateProfileTables(DbR);
    static DbResult AssignProfileVersion(DbR, bool onProfileCreation);
    static void GetMigratorSequence(std::vector<std::unique_ptr<ProfileMigrator>>&, ProfileVersion const& fileProfileVersion);

public:
    //! Checks whether the file's AppModel profile version is compatible with this build.
    static ProfileState CheckProfileVersion(DbCR);

    //! Creates the AppModel profile (stamps the version and creates the tables) in a newly-created file.
    //! Must be called inside the create transaction. Rolls back on failure.
    static DbResult CreateProfile(DbR);

    //! Upgrades the AppModel profile of the file to the latest version by running the migrator sequence.
    //! Must be called inside the open transaction. Rolls back on failure.
    static DbResult UpgradeProfile(DbR);

    //! Reads the AppModel profile version from the file.
    //! @return BE_SQLITE_OK on success, BE_SQLITE_ERROR_InvalidProfileVersion if the file has no AppModel profile.
    static DbResult ReadProfileVersion(ProfileVersion&, DbCR);
    };

//=======================================================================================
//! Base class for an in-situ AppModel profile migration step. Each concrete migrator
//! migrates a file from one profile version to the next. Mirrors BeSQLiteProfileUpgrader.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ProfileMigrator
    {
private:
    virtual DbResult _Migrate(DbR db) const = 0;

protected:
    ProfileMigrator() {}

public:
    virtual ~ProfileMigrator() {}
    DbResult Migrate(DbR db) const { return _Migrate(db); }
    };

// NOTE: There are no AppModel profile migrators yet - the profile is at its initial version.
// When the profile version is bumped, add a concrete migrator here (e.g. struct ProfileMigrator_1001)
// that performs the DDL/data migration from the prior version, and push it onto the sequence in
// ProfileManager::GetMigratorSequence, ordered from oldest to newest.

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ProfileManager::CreateProfile(DbR db)
    {
    if (!db.GetDefaultTransaction()->IsActive())
        {
        BeAssert(false && "Programmer Error. AppModelDb expects that BeSQLite::CreateNewDb keeps the default transaction active when it creates its profile.");
        return BE_SQLITE_ERROR_NoTxnActive;
        }

    DbResult stat = AssignProfileVersion(db, true);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Failed to create " PROFILENAME " profile in '%s'. Could not assign profile version. %s", db.GetDbFileName(), db.GetLastError().c_str());
        db.AbandonChanges();
        return stat;
        }

    stat = CreateProfileTables(db);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Failed to create " PROFILENAME " profile in '%s'. Could not create tables. %s", db.GetDbFileName(), db.GetLastError().c_str());
        db.AbandonChanges();
        return stat;
        }

    db.SaveChanges();
    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
ProfileState ProfileManager::CheckProfileVersion(DbCR db)
    {
    ProfileVersion fileProfileVersion(0, 0, 0, 0);
    if (BE_SQLITE_OK != ReadProfileVersion(fileProfileVersion, db))
        return ProfileState::Error();

    return Db::CheckProfileVersion(GetExpectedVersion(), fileProfileVersion, GetMinimumSupportedVersion(), PROFILENAME);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ProfileManager::UpgradeProfile(DbR db)
    {
    BeAssert(!db.IsReadonly());
    if (!db.GetDefaultTransaction()->IsActive())
        {
        BeAssert(false && "Programmer Error. AppModelDb expects that BeSQLite::OpenBeSQLiteDb keeps the default transaction active when it upgrades its profile.");
        return BE_SQLITE_ERROR_NoTxnActive;
        }

    ProfileVersion fileProfileVersion(0, 0, 0, 0);
    DbResult stat = ReadProfileVersion(fileProfileVersion, db);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Upgrade of " PROFILENAME " profile of file '%s' failed. File has no AppModel profile.", db.GetDbFileName());
        return stat;
        }

    if (fileProfileVersion == GetExpectedVersion())
        return BE_SQLITE_OK; // already up-to-date

    // Let migrators incrementally upgrade the profile to the latest state.
    std::vector<std::unique_ptr<ProfileMigrator>> migrators;
    GetMigratorSequence(migrators, fileProfileVersion);
    for (std::unique_ptr<ProfileMigrator> const& migrator : migrators)
        {
        stat = migrator->Migrate(db);
        if (BE_SQLITE_OK != stat)
            {
            db.AbandonChanges();
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }
        }

    // Stamp the new profile version after the migrators have run.
    stat = AssignProfileVersion(db, false);
    if (BE_SQLITE_OK != stat)
        {
        db.AbandonChanges();
        LOG.errorv("Upgrade of " PROFILENAME " profile of file '%s' failed. Could not assign new profile version. %s", db.GetDbFileName(), db.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.infov("Upgraded " PROFILENAME " profile of file '%s' from version %s to version %s.",
              db.GetDbFileName(), fileProfileVersion.ToString().c_str(), GetExpectedVersion().ToString().c_str());
    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ProfileManager::AssignProfileVersion(DbR db, bool onProfileCreation)
    {
    Utf8String versionStr = GetExpectedVersion().ToJson();

    if (onProfileCreation)
        {
        DbResult stat = db.SavePropertyString(AppModelDb::InitialProfileVersionProperty(), versionStr);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return db.SavePropertyString(AppModelDb::ProfileVersionProperty(), versionStr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ProfileManager::ReadProfileVersion(ProfileVersion& profileVersion, DbCR db)
    {
    profileVersion = ProfileVersion(0, 0, 0, 0);

    Utf8String versionStr;
    if (BE_SQLITE_ROW != db.QueryProperty(versionStr, AppModelDb::ProfileVersionProperty()))
        return BE_SQLITE_ERROR_InvalidProfileVersion; // no AppModel profile in this file

    if (BentleyStatus::SUCCESS != profileVersion.FromJson(versionStr.c_str()))
        return BE_SQLITE_ERROR_InvalidProfileVersion;

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ProfileManager::CreateProfileTables(DbR db)
    {
    // Order matters: tables with foreign keys must be created after the tables they reference.
    Utf8CP const ddls[] =
        {
        TABLEDDL_Dgns,
        TABLEDDL_DgnStreams,
        TABLEDDL_Models,
        TABLEDDL_Elements,
        TABLEDDL_DgnReferencesModels,
        TABLEDDL_ModelUsesDgnLibElement,
        TABLEDDL_Schemas,
        TABLEDDL_DgnContainsSchemas,
        TABLEDDL_SchemaClasses,
        TABLEDDL_RasterImages,
        TABLEDDL_Txns,
        };

    for (Utf8CP ddl : ddls)
        {
        DbResult stat = db.ExecuteDdl(ddl);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
void ProfileManager::GetMigratorSequence(std::vector<std::unique_ptr<ProfileMigrator>>& migrators, ProfileVersion const& /*fileProfileVersion*/)
    {
    migrators.clear();
    // No migrators yet - the profile is at its initial version (see the ProfileMigrator note above).
    // Example for the future:
    //   if (fileProfileVersion < ProfileVersion(1, 0, 0, 1))
    //       migrators.push_back(std::make_unique<ProfileMigrator_1001>());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
AppModelDb::AppModelDb() {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
AppModelDb::~AppModelDb() {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult AppModelDb::QueryProfileVersion(ProfileVersion& version) const { return ProfileManager::ReadProfileVersion(version, *this); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Txns& AppModelDb::GetTxns()
    {
    if (!m_txns.IsValid())
        m_txns = new Txns(*this); // the Txns ctor registers itself as this db's change tracker
    return *m_txns;
    }

//---------------------------------------------------------------------------------------
//! Attach the AppModel change tracker and enable tracking when the db is writable. Called after
//! the profile exists. Change tracking records local changes into APPMODEL_TABLE_Txns on SaveChanges.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void AppModelDb::SetupChangeTracking()
    {
    Txns& txns = GetTxns(); // creates + registers the tracker
    if (!IsReadonly())
        txns.EnableTracking(true); // record changes only on a writable db
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult AppModelDb::_OnDbCreated(CreateParams const& params)
    {
    DbResult stat = Db::_OnDbCreated(params);
    if (BE_SQLITE_OK != stat)
        return stat;

    BeAssert(!IsReadonly());
    stat = ProfileManager::CreateProfile(*this);
    if (BE_SQLITE_OK != stat)
        return stat;

    // Enable tracking only after the profile (including its tables) has been created, so that the
    // profile's own DDL is not recorded as a txn.
    SetupChangeTracking();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult AppModelDb::_OnDbOpened(OpenParams const& params)
    {
    DbResult stat = Db::_OnDbOpened(params);
    if (BE_SQLITE_OK != stat)
        return stat;

    SetupChangeTracking();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void AppModelDb::_OnDbClose()
    {
    if (m_txns.IsValid())
        {
        m_txns->EndTracking();
        SetChangeTracker(nullptr); // detach from the DbFile (drops its reference)
        m_txns = nullptr;          // drop our reference; the tracker is deleted when the last ref is gone
        }

    Db::_OnDbClose();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ProfileState AppModelDb::_CheckProfileVersion() const
    {
    // The base BeSQLite profile must be usable before we even consider the AppModel profile.
    ProfileState besqliteState = Db::_CheckProfileVersion();
    if (!besqliteState.IsUpToDate())
        return besqliteState;

    // Note: we intentionally do not use ProfileState::Merge here. Merge discards the right-hand state when it is an
    // error (and the left-hand state is up-to-date), which would let a plain BeSQLite file - one without any AppModel
    // profile - open as an AppModelDb. Returning the AppModel state directly ensures a missing profile is rejected.
    return ProfileManager::CheckProfileVersion(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult AppModelDb::_UpgradeProfile(OpenParams const& params)
    {
    DbResult stat = Db::_UpgradeProfile(params);
    if (BE_SQLITE_OK != stat)
        return stat;

    return ProfileManager::UpgradeProfile(*this);
    }

//=======================================================================================
// Header prepended to the Snappy-compressed changeset blob stored in APPMODEL_TABLE_Txns.Change.
// @bsistruct
//=======================================================================================
struct ChangesBlobHeader
    {
    static const uint32_t Signature = 0x0601;
    uint32_t m_signature;
    uint32_t m_size;
    explicit ChangesBlobHeader(uint32_t size) : m_signature(Signature), m_size(size) {}
    explicit ChangesBlobHeader(SnappyReader& in) { uint32_t actuallyRead; in._Read((Byte*)this, sizeof(*this), actuallyRead); }
    bool IsValid() const { return m_signature == Signature && m_size != 0; }
    };

//---------------------------------------------------------------------------------------
//! Construct the tracker and register it as the db's change tracker. SetChangeTracker also
//! sets this tracker's Db back-pointer and takes a reference (held in the DbFile).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Txns::Txns(AppModelDb& db) : ChangeTracker("appmodel_txns")
    {
    db.SetChangeTracker(this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ChangeTracker::TrackChangesForTable Txns::_FilterTable(Utf8CP tableName)
    {
    // Skip be_Local (derived/redundant) and the txns table itself (writing a txn must not be tracked).
    if (0 == BeStringUtilities::StricmpAscii(tableName, BEDB_TABLE_Local) ||
        0 == BeStringUtilities::StricmpAscii(tableName, APPMODEL_TABLE_Txns))
        return TrackChangesForTable::No;

    return TrackChangesForTable::Yes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ChangeTracker::OnCommitStatus Txns::_OnCommit(bool isCommit, Utf8CP operation)
    {
    // Capture the tracked DDL and data changes, then restart the tracker.
    DdlChanges ddlChanges = std::move(m_ddlChanges);

    ChangeSet dataChanges;
    if (HasDataChanges())
        {
        if (BE_SQLITE_OK != dataChanges.FromChangeTrack(*this))
            {
            LOG.error("Txns: failed to create data changeset from tracker");
            return OnCommitStatus::Abort;
            }
        Restart(); // clear the tracker now that changes are captured
        }

    if (!isCommit)
        {
        // AbandonChanges: discard the captured changes and let BeSQLite perform the rollback.
        return OnCommitStatus::NoChanges;
        }

    if (dataChanges._IsEmpty() && ddlChanges._IsEmpty())
        return OnCommitStatus::NoChanges;

    // 'operation' is the description passed to Db::SaveChanges - store it on the txn.
    if (BE_SQLITE_OK != WriteTxn(operation, ddlChanges, dataChanges))
        {
        LOG.error("Txns: failed to write txn");
        return OnCommitStatus::Abort;
        }

    return OnCommitStatus::Commit;
    }

//---------------------------------------------------------------------------------------
//! Store one txn row holding both the DDL and the data changeset in separate columns.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult Txns::WriteTxn(Utf8CP description, DdlChanges const& ddlChanges, ChangeSet const& dataChanges)
    {
    AppModelDb& db = GetAppModelDb();

    TxnType type = ddlChanges._IsEmpty() ? TxnType::Data : TxnType::Schema;
    Utf8String ddlStr = ddlChanges._IsEmpty() ? Utf8String() : ddlChanges.ToString();

    enum Column : int { Type = 1, Description = 2, Props = 3, Ddl = 4, Change = 5 };
    Statement stmt;
    DbResult rc = stmt.Prepare(db, "INSERT INTO " APPMODEL_TABLE_Txns "(Type,Description,Props,Ddl,Change) VALUES(?,?,?,?,?)");
    if (BE_SQLITE_OK != rc)
        return rc;

    stmt.BindInt(Column::Type, (int) type);
    if (!Utf8String::IsNullOrEmpty(description))
        stmt.BindText(Column::Description, description, Statement::MakeCopy::No);
    if (!m_pendingProps.empty())
        stmt.BindText(Column::Props, m_pendingProps, Statement::MakeCopy::No);
    if (!ddlStr.empty())
        stmt.BindText(Column::Ddl, ddlStr, Statement::MakeCopy::No);

    // Compress the data changeset (if any) into the Change blob column, reusing this tracker's SnappyToBlob.
    SnappyToBlob& snappyTo = m_snappyToBlob;
    uint32_t zipSize = 0;
    if (!dataChanges._IsEmpty())
        {
        snappyTo.Init();
        ChangesBlobHeader header((uint32_t) dataChanges.GetSize());
        snappyTo.Write((Byte const*) &header, sizeof(header));
        for (auto const& chunk : dataChanges.m_data.m_chunks)
            snappyTo.Write(chunk.data(), (uint32_t) chunk.size());

        zipSize = snappyTo.GetCompressedSize();
        if (0 < zipSize)
            {
            if (1 == snappyTo.GetCurrChunk())
                rc = stmt.BindBlob(Column::Change, snappyTo.GetChunkData(0), zipSize, Statement::MakeCopy::No);
            else
                rc = stmt.BindZeroBlob(Column::Change, zipSize); // multi-chunk: write via SaveToRow after insert
            if (BE_SQLITE_OK != rc)
                return rc;
            }
        }

    if (BE_SQLITE_DONE != stmt.Step())
        return BE_SQLITE_ERROR;

    if (0 < zipSize && 1 != snappyTo.GetCurrChunk())
        {
        rc = snappyTo.SaveToRow(db, APPMODEL_TABLE_Txns, "Change", db.GetLastInsertRowId());
        if (BE_SQLITE_OK != rc)
            return rc;
        }

    m_pendingProps.clear();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult Txns::ReadTxnDataChanges(ChangeSet& dataChanges, int64_t txnId) const
    {
    SnappyFromBlob snappyFrom;
    if (ZIP_SUCCESS != snappyFrom.Init(GetAppModelDb(), APPMODEL_TABLE_Txns, "Change", txnId))
        return BE_SQLITE_ERROR;

    ChangesBlobHeader header(snappyFrom);
    if (!header.IsValid())
        return BE_SQLITE_ERROR;

    return (ZIP_SUCCESS == snappyFrom.ReadToChunkedArray(dataChanges.m_data, header.m_size)) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool Txns::HasPendingTxns() const
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetAppModelDb(), "SELECT 1 FROM " APPMODEL_TABLE_Txns " LIMIT 1"))
        return false;

    return BE_SQLITE_ROW == stmt.Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String Txns::GetParentChangesetId() const
    {
    Utf8String changesetId;
    return (BE_SQLITE_ROW == GetAppModelDb().QueryBriefcaseLocalValue(changesetId, "appmodel_ParentChangesetId")) ? changesetId : Utf8String();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult Txns::SetParentChangesetId(Utf8StringCR changesetId)
    {
    // be_Local is excluded from change tracking, so this does not produce a txn.
    return GetAppModelDb().SaveBriefcaseLocalValue("appmodel_ParentChangesetId", changesetId);
    }

//---------------------------------------------------------------------------------------
//! Begin creating a changeset: combine all recorded txns into a single AppModel changeset file and fill @p props
//! with the resulting changeset's properties (parent id, computed SHA1 id, db guid, file, type). The captured
//! txns are remembered so EndCreateChangeset can delete them and advance the changeset id once the file is safely
//! persisted. Modeled on TxnManager::WriteChangesToFile in iModelPlatform.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult Txns::BeginCreateChangeset(ChangesetProps& props, BeFileNameCR pathname)
    {
    AppModelDb& db = GetAppModelDb();

    DdlChanges ddlChanges;
    ChangeGroup dataChangeGroup(db);

    Statement stmt;
    DbResult rc = stmt.Prepare(db, "SELECT Id,Ddl,(Change IS NOT NULL) FROM " APPMODEL_TABLE_Txns " ORDER BY Id");
    if (BE_SQLITE_OK != rc)
        return rc;

    bool hasAnyTxn = false;
    bool hasSchemaChange = false;
    int64_t lastTxnId = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        hasAnyTxn = true;
        int64_t txnId = stmt.GetValueInt64(0);
        lastTxnId = txnId; // rows are ordered by Id, so this ends as the highest captured txn id

        if (!stmt.IsColumnNull(1))
            {
            ddlChanges.AddDDL(stmt.GetValueText(1));
            hasSchemaChange = true;
            }

        if (0 != stmt.GetValueInt(2)) // Change blob present
            {
            ChangeSet dataChanges;
            rc = ReadTxnDataChanges(dataChanges, txnId);
            if (BE_SQLITE_OK != rc)
                {
                LOG.errorv("Txns: failed to read data changes for txn %lld", (long long) txnId);
                return rc;
                }
            // Data changes from all captured txns are squashed into one changegroup. This relies on schema changes
            // being additive and applied only during migration (see the class @note on Txns): a table's column
            // count must not change in between two data changes to that same table within one changeset window, or
            // AddToChangeGroup would return BE_SQLITE_SCHEMA.
            rc = dataChanges.AddToChangeGroup(dataChangeGroup);
            if (BE_SQLITE_OK != rc)
                return rc;
            }
        }

    if (!hasAnyTxn)
        return BE_SQLITE_ERROR; // nothing to write

    ChangesetFileWriter writer(pathname, false /*containsEcSchemaChanges*/, ddlChanges, &db, LzmaEncoder::LzmaParams(), ChangesetKind::AppModel);
    rc = writer.Initialize();
    if (BE_SQLITE_OK != rc)
        return rc;

    rc = writer.FromChangeGroup(dataChangeGroup);
    if (BE_SQLITE_OK != rc)
        return rc;

    // Compute the changeset properties: parent is this db's current changeset id, the new id is the
    // SHA1 of (parent id + file contents).
    Utf8String parentId = GetParentChangesetId();
    Utf8String changesetId = ChangesetProps::ComputeChangesetId(parentId, pathname);
    if (changesetId.empty())
        return BE_SQLITE_ERROR;

    props = ChangesetProps(changesetId, parentId, db.GetDbGuid().ToString(),
                           pathname, hasSchemaChange ? ChangesetType::Schema : ChangesetType::Regular);

    // Remember what EndCreateChangeset must finalize: the new changeset id and the highest captured txn id.
    m_creatingChangesetId = changesetId;
    m_creatingChangesetLastTxnId = lastTxnId;
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
//! Finalize the changeset started by BeginCreateChangeset: delete the captured txns and advance this db's current
//! changeset id, then commit. Txns recorded after BeginCreateChangeset (Id greater than the captured high-water
//! mark) are preserved for the next changeset.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult Txns::EndCreateChangeset()
    {
    if (m_creatingChangesetId.empty() || 0 == m_creatingChangesetLastTxnId)
        return BE_SQLITE_ERROR; // no BeginCreateChangeset in progress

    AppModelDb& db = GetAppModelDb();

    // Delete the txns captured by BeginCreateChangeset. The txns table is excluded from change tracking (see
    // _FilterTable), so this is not recorded as a new txn. Newer txns (Id > high-water mark) are left in place.
    // Scope the statement so it is finalized before the commit below.
    DbResult rc;
    {
    Statement stmt;
    rc = stmt.Prepare(db, "DELETE FROM " APPMODEL_TABLE_Txns " WHERE Id<=?");
    if (BE_SQLITE_OK != rc)
        return rc;

    stmt.BindInt64(1, m_creatingChangesetLastTxnId);
    if (BE_SQLITE_DONE != stmt.Step())
        return BE_SQLITE_ERROR;
    }

    // Advance this db's current changeset id so the next changeset chains onto the one just created.
    rc = SetParentChangesetId(m_creatingChangesetId);
    if (BE_SQLITE_OK != rc)
        {
        db.AbandonChanges();
        return rc;
        }

    rc = db.SaveChanges();
    if (BE_SQLITE_OK != rc)
        {
        db.AbandonChanges(); // leave the pending create in place so the caller can retry
        return rc;
        }

    m_creatingChangesetId.clear();
    m_creatingChangesetLastTxnId = 0;
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
//! Apply the DDL then the data changes from an AppModel changeset file, with change tracking
//! suspended so the applied changes are not recorded as new txns.
//---------------------------------------------------------------------------------------
// Reader for applying an AppModel changeset file. Overrides conflict handling to fail on ANY
// conflict for now. This is the extension point for future, more nuanced conflict resolution
// (e.g. skipping benign FK cascade cases like iModel's ChangesetFileReader does).
// @bsistruct
//---------------------------------------------------------------------------------------
struct ChangesetReader : ChangesetFileReaderBase
    {
    explicit ChangesetReader(BeFileNameCR pathname, Db const* db)
        : ChangesetFileReaderBase({pathname}, db, ChangesetKind::AppModel) {}

    protected:
        ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override
            {
            // Fail on any conflict. Future: inspect 'cause'/'iter' and resolve selectively.
            return ChangeSet::ConflictResolution::Abort;
            }
    };

//---------------------------------------------------------------------------------------
//! Apply the DDL then the data changes from an AppModel changeset file, with change tracking
//! suspended so the applied changes are not recorded as new txns. Conflicts abort the apply.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult Txns::ApplyChangesetFile(BeFileNameCR pathname)
    {
    AppModelDb& db = GetAppModelDb();

    ChangesetReader reader(pathname, &db);

    // Read the DDL (schema) changes carried by the changeset.
    bool containsSchemaChanges = false;
    DdlChanges ddlChanges;
    if (BE_SQLITE_OK != reader.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges))
        return BE_SQLITE_ERROR;

    const bool wasTracking = IsTracking();
    EnableTracking(false); // do not record the applied changes as new txns

    DbResult rc = BE_SQLITE_OK;
    if (!ddlChanges._IsEmpty())
        {
        for (Utf8StringCR ddl : ddlChanges.GetDDLs())
            {
            rc = db.ExecuteDdl(ddl.c_str());
            if (BE_SQLITE_OK != rc)
                break;
            }
        }

    if (BE_SQLITE_OK == rc)
        rc = reader.ApplyChanges(db, ApplyChangesArgs()); // ChangesetReader::_OnConflict aborts on any conflict

    if (wasTracking)
        EnableTracking(true);

    return rc;
    }

//---------------------------------------------------------------------------------------
// SHA1 changeset-id generator. Feeds the parent id (as bytes), the changeset file prefix and the
// changeset body through a SHA1 hash. Mirrors TxnManager's ChangesetIdGenerator.
// @bsistruct
//---------------------------------------------------------------------------------------
struct ChangesetIdGenerator : ChangeStream
    {
    SHA1 m_hash;

    static int HexCharToInt(char c)
        {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        BeAssert(false);
        return 0;
        }

    void AddStringToHash(Utf8StringCR hashString)
        {
        Byte hashValue[SHA1::HashBytes];
        if (hashString.empty())
            {
            memset(hashValue, 0, SHA1::HashBytes);
            }
        else
            {
            BeAssert(hashString.length() == SHA1::HashBytes * 2);
            for (int ii = 0; ii < SHA1::HashBytes; ii++)
                hashValue[ii] = (Byte)(16 * HexCharToInt(hashString.at(2 * ii)) + HexCharToInt(hashString.at(2 * ii + 1)));
            }
        m_hash.Add(hashValue, SHA1::HashBytes);
        }

    DbResult _Append(Byte const* pData, int nData) override { m_hash.Add(pData, nData); return BE_SQLITE_OK; }
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause, Changes::Change) override { return ChangeSet::ConflictResolution::Abort; }
    bool _IsEmpty() const override final { return false; }
    RefCountedPtr<Changes::Reader> _GetReader() const override { return nullptr; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String ChangesetProps::ComputeChangesetId(Utf8StringCR parentId, BeFileNameCR changesetFile)
    {
    if (!parentId.empty() && parentId.length() != SHA1::HashBytes * 2)
        return ""; // invalid parent id: must be empty or a SHA1 hex string

    if (!changesetFile.DoesPathExist())
        return "";

    ChangesetIdGenerator idGen;
    idGen.AddStringToHash(parentId);

    ChangesetFileReaderBase fs({changesetFile}, nullptr, ChangesetKind::AppModel);
    auto reader = fs.MakeReader();

    DbResult result;
    Utf8StringCR prefix = reader->GetPrefix(result);
    if (BE_SQLITE_OK != result)
        return "";

    if (!prefix.empty())
        idGen._Append((Byte const*) prefix.c_str(), (int) prefix.SizeInBytes());

    if (BE_SQLITE_OK != idGen.ReadFrom(*reader))
        return "";

    return idGen.m_hash.GetHashString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ChangesetProps::ValidateContent(AppModelDb const& db) const
    {
    if (m_dbGuid != db.GetDbGuid().ToString())
        return BE_SQLITE_MISMATCH; // changeset did not originate from this db

    if (!m_fileName.DoesPathExist())
        return BE_SQLITE_ERROR;

    Utf8String computedId = ComputeChangesetId(m_parentId, m_fileName);
    if (computedId.empty() || !computedId.EqualsIAscii(m_id))
        return BE_SQLITE_ERROR_InvalidChangeSetVersion; // incorrect id for changeset

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult Txns::MergeChangeset(ChangesetProps const& props)
    {
    AppModelDb& db = GetAppModelDb();

    // The changeset must chain onto our current changeset.
    if (GetParentChangesetId() != props.GetParentId())
        return BE_SQLITE_MISMATCH;

    // Verify db guid + recompute and compare the SHA1 id.
    DbResult rc = props.ValidateContent(db);
    if (BE_SQLITE_OK != rc)
        return rc;

    rc = ApplyChangesetFile(props.GetFileName());
    if (BE_SQLITE_OK != rc)
        {
        db.AbandonChanges();
        return rc;
        }

    rc = SetParentChangesetId(props.GetChangesetId());
    if (BE_SQLITE_OK != rc)
        {
        db.AbandonChanges();
        return rc;
        }

    rc = db.SaveChanges();
    if (BE_SQLITE_OK != rc)
        db.AbandonChanges(); // mirror the paths above: never leave the applied changes + advanced parent id pending

    return rc;
    }

END_BENTLEY_APPMODEL_NAMESPACE
