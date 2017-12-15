/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDb.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECInstanceId.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>
#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct SchemaManager;

struct ECCrudWriteToken;
struct SchemaImportToken;

//=======================================================================================
//! Enum which mirrors the ECEnumeration OpCode in the ECDbChange ECSchema.
//! The enum can be used when programmatically binding values to the OpCode in an ECSQL
//! against the ECDbChange ECSchema.
//! @see @ref ECDbChange
// @bsienum                                              Krischan.Eberle      11/2017
//+===============+===============+===============+===============+===============+======
enum class ChangeOpCode
    {
    //Its values must always match the ECEnumeration
    Insert = 1,
    Update = 2,
    Delete = 4
    };

//=======================================================================================
//! The enum represents the values for the ChangedValueState argument of the ECSQL function
//! @b Changes.
//! The enum can be used when programmatically binding values to the ChangedValueState argument
//! in an ECSQL using the Changes ECSQL function.
//! @see @ref ECDbChange
// @bsienum                                                             11/2017
//+===============+===============+===============+===============+===============+======
enum class ChangedValueState
    {
    AfterInsert = 1, //!< query for the property values of an inserted row
    BeforeUpdate = 2, //!< query for the changed property values of an updated row before the update
    AfterUpdate = 3, //!< query for the changed property values of an updated row after the update
    BeforeDelete = 4 //!< query for the property values of a deleted row before the delete
    };

//=======================================================================================
//! @ingroup ECDbGroup
// @bsiclass                                                            12/2017
//+===============+===============+===============+===============+===============+======
struct ChangeSetArg final
    {
    struct User final
        {
        Utf8String m_id;
        Utf8String m_name;

        bool IsValid() const { return !m_id.empty(); }
        };

    private:
        BeSQLite::IChangeSet& m_changeSet;

        Utf8String m_changeSetId;
        Utf8String m_parentChangeSetId;
        DateTime m_pushDate;
        User m_createdBy;

    public:
        ChangeSetArg(BeSQLite::IChangeSet& changeSet) : m_changeSet(changeSet) {}

        BeSQLite::IChangeSet& GetChangeSet() const { return m_changeSet; }
        Utf8StringCR GetId() const { return m_changeSetId; }
        Utf8StringCR GetParentId() const { return m_parentChangeSetId; }
        DateTime const& GetPushDate() const { return m_pushDate; }
        User const& GetCreatedBy() const { return m_createdBy; }
        ChangeSetArg& SetId(Utf8StringCR id) { m_changeSetId.assign(id); return *this; }
        ChangeSetArg& SetParentId(Utf8StringCR parentId) { m_parentChangeSetId.assign(parentId); return *this; }
        ChangeSetArg& SetPushDate(DateTimeCR pushDate) { m_pushDate = pushDate; return *this; }
        ChangeSetArg& SetCreatedBy(User const& createdBy) { m_createdBy = createdBy; return *this; }
    };

//=======================================================================================
//! ECDb is the %EC API used to access %EC data in an @ref ECDbFile "ECDb file".
//! 
//! It is used to create, open, close @ref ECDbFile "ECDb files" (see ECDb::CreateNewDb, ECDb::OpenBeSQLiteDb,
//! ECDb::CloseDb) and gives access to the %EC data.
//!
//! An ECDb object is generally thread-safe. However an ECDb connection must be closed in the same thread in which is was opened.
//! @see @ref ECDbOverview, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                            03/2012
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECDb : Db
{
public:
    //=======================================================================================
    //! Options that control what to do with the Change cache file when opening an ECDb file
    //! @see ECDb::OpenParams @see ECDb::AttachChangeCache  @see @ref ECDbChange
    // @bsienum                                                    11/17
    //=======================================================================================
    enum class ChangeCacheMode
        {
        AttachAndCreateIfNotExists,
        AttachIfExists,
        DoNotAttach
        };

    //=======================================================================================
    // @bsiclass                                                    11/17
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE OpenParams : BeSQLite::Db::OpenParams
        {
        private:
            ChangeCacheMode m_changeCacheMode = ChangeCacheMode::DoNotAttach;

        public:
            explicit OpenParams(OpenMode openMode, BeSQLite::DefaultTxn startDefaultTxn = BeSQLite::DefaultTxn::Yes) : Db::OpenParams(openMode, startDefaultTxn) {}
            OpenParams(OpenMode openMode, ChangeCacheMode changeCacheMode) : Db::OpenParams(openMode, BeSQLite::DefaultTxn::Yes), m_changeCacheMode(changeCacheMode) {}
            virtual ~OpenParams() {}

            //! Sets the ChangeCacheMode in the params
            //!@note If a profile upgrade has to be performed while opening a file,
            //!the specified mode is ignored and ChangeCacheMode::AttachAndCreateIfNotExists is used.
            //! @param[in] mode ChangeCacheMode to set
            //! @return the params object itself for fluid API calls
            OpenParams& Set(ChangeCacheMode mode) { m_changeCacheMode = mode; return *this; }

            //! Gets the ChangeCacheMode
            //!@note If a profile upgrade has to be performed while opening a file,
            //!the specified mode is ignored and ChangeCacheMode::AttachAndCreateIfNotExists is used.
            //! @return ChangeCacheMode
            ChangeCacheMode GetChangeCacheMode() const { return m_changeCacheMode; }
        };

    //=======================================================================================
    //! Compile-time Settings that subclasses can set.
    // @bsiclass                                                    02/2017
    //+===============+===============+===============+===============+===============+======
    struct Settings final
        {
        private:
            bool m_requiresECCrudWriteToken = false;
            bool m_requiresECSchemaImportToken = false;
            bool m_allowChangesetMergingIncompatibleSchemaImport = true;

        public:
#if !defined (DOCUMENTATION_GENERATOR)
            //not inlined as ctors are only needed internally
            Settings();
            Settings(bool requiresECCrudWriteToken, bool m_requiresECSchemaImportToken, bool allowChangesetMergingIncompatibleSchemaImport);
#endif
            bool RequiresECCrudWriteToken() const { return m_requiresECCrudWriteToken; }
            bool RequiresECSchemaImportToken() const { return m_requiresECSchemaImportToken; }
            bool AllowChangesetMergingIncompatibleSchemaImport() const { return m_allowChangesetMergingIncompatibleSchemaImport; }
        };

    struct SettingsManager final
        {
    private:
        Settings m_settings;
        ECCrudWriteToken const* m_crudWriteToken = nullptr;
        SchemaImportToken const* m_schemaImportToken = nullptr;
        
    public:
#if !defined (DOCUMENTATION_GENERATOR)
        //not inlined as ctors are only needed internally
        SettingsManager();
        void ApplySettings(bool requireECCrudWriteToken, bool requireECSchemaImportToken, bool allowChangesetMergingIncompatibleECSchemaImport);
#endif
        ECDB_EXPORT ~SettingsManager();

        Settings const& GetSettings() const { return m_settings; }
        //! Consumers can only execute EC CRUD operations like ECSQL INSERT, UPDATE or DELETE statements
        ECCrudWriteToken const* GetCrudWriteToken() const { return m_crudWriteToken; }
        //! Consumers can only import ECSchemas with the token
        SchemaImportToken const* GetSchemaImportToken() const { return m_schemaImportToken; }
        };

    //=======================================================================================
    // @bsiclass                                                                11/2017
    //+===============+===============+===============+===============+===============+======
    struct ChangeSummaryExtractOptions final
        {
        private:
            bool m_includeRelationshipInstances = true;

        public:
            explicit ChangeSummaryExtractOptions(bool includeRelationshipInstances = true) : m_includeRelationshipInstances(includeRelationshipInstances) {}
            bool IncludeRelationshipInstances() const { return m_includeRelationshipInstances; }
        };

    //=======================================================================================
    //! Modes for the ECDb::Purge method.
    // @bsiclass                                                        11/2015
    //+===============+===============+===============+===============+===============+======
    enum class PurgeMode
        {
        FileInfoOwnerships = 1, //!< Purges FileInfoOwnership instances (see also @ref ECDbFileInfo)
        All = FileInfoOwnerships
        };

    //=======================================================================================
    //! Allows clients to be notified of error messages.
    //! @remarks ECDb cares for logging any error sent to listeners via BentleyApi::NativeLogging. 
    //! So implementors don't have to do that anymore.
    // @bsiclass                                                        09/2015
    //+===============+===============+===============+===============+===============+======
    struct IIssueListener
        {
    private:
        //! Fired by ECDb whenever an issue occurred during the schema import.
        //! @param[in] message Issue message
        virtual void _OnIssueReported(Utf8CP message) const = 0;

    protected:
        IIssueListener() {}

    public:
        virtual ~IIssueListener() {}

#if !defined (DOCUMENTATION_GENERATOR)
        //! Called by ECDb to report an issue to clients.
        //! @param[in] message Issue message
        void ReportIssue(Utf8CP message) const;
#endif
        };

    struct Impl;

private:
    Impl* m_pimpl;

#if !defined (DOCUMENTATION_GENERATOR)
protected:
    //! To be called during construction of the ECDb subclass.
    //This is only a separate method because DgnDb is not a direct subclass of ECDb, but of RefCounted<ECDb>. So DgnDb's ctor cannot call ECDb's ctor.
    ECDB_EXPORT void ApplyECDbSettings(bool requireECCrudWriteToken, bool requireECSchemaImportToken, bool allowChangesetMergingIncompatibleECSchemaImport);

    ECDB_EXPORT DbResult _OnDbOpening() override;
    ECDB_EXPORT DbResult _OnDbOpened(Db::OpenParams const&) override;
    ECDB_EXPORT DbResult _OnDbCreated(CreateParams const&) override;
    ECDB_EXPORT DbResult _OnBriefcaseIdAssigned(BeBriefcaseId newBriefcaseId) override;
    ECDB_EXPORT void _OnDbClose() override;
    ECDB_EXPORT void _OnDbChangedByOtherConnection() override;
    ECDB_EXPORT DbResult _VerifyProfileVersion(Db::OpenParams const&) override;
    ECDB_EXPORT DbResult _OnDbAttached(Utf8CP fileName, Utf8CP dbAlias) const override;
    ECDB_EXPORT DbResult _OnDbDetached(Utf8CP dbAlias) const override;
    ECDB_EXPORT int _OnAddFunction(DbFunction&) const override;
    ECDB_EXPORT void _OnRemoveFunction(DbFunction&) const override;

    //! Resets ECDb's ECInstanceId sequence to the current maximum ECInstanceId for the specified BriefcaseId.
    //! @param[in] briefcaseId BriefcaseId to which the sequence will be reset
    //! @param[in] ecClassIgnoreList List of ids of ECClasses whose ECInstanceIds should be ignored when
    //!            computing the maximum ECInstanceId. Subclasses of the specified classes will be ignored as well.
    //!            If nullptr, no ECClass will be ignored.
    //! SUCCESS or ERROR
    ECDB_EXPORT BentleyStatus ResetInstanceIdSequence(BeBriefcaseId briefcaseId, IdSet<ECN::ECClassId> const* ecClassIgnoreList = nullptr);

    //! Returns the settings manager to subclasses which gives access to the various access tokens
    ECDB_EXPORT SettingsManager const& GetECDbSettingsManager() const;
#endif

    virtual void _OnAfterSchemaImport() const {}

public:
    //! This method @b must be called once per process before any other ECDb method is called.
    //! @remarks This method is a convenience wrapper around the individual initialization routines
    //!          needed for ECDb. This includes calls to BeSQLiteLib::Initialize and to
    //!          ECN:ECSchemaReadContext::Initialize
    //! @param[in] ecdbTempDir Directory where ECDb stores SQLite's temporary files.
    //!            Must not be an existing directory!
    //! @param[in] hostAssetsDir Directory to where the application has deployed assets 
    //!            that come with this API, e.g. standard ECSchemas.
    //!            In the assets directory the standard ECSchemas have to be located in @b ECSchemas/Standard/.
    //!            The standard ECSchemas are needed when importing ECSchemas into the ECDb file
    //!            or when creating a new and empty ECDb file.
    //!            If nullptr is passed, the host assets dir does not get initialized, and
    //!            standard schemas cannot be located by ECDb.
    //! @param[in] logSqliteErrors If Yes, then SQLite error messages are logged. Note that some SQLite errors are intentional. Turn this option on only for limited debuging purposes.
    //! @return ::BE_SQLITE_OK in case of success, error code otherwise, e.g. if @p ecdbTempDir does not exist
    ECDB_EXPORT static DbResult Initialize(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir = nullptr, BeSQLiteLib::LogErrors logSqliteErrors=BeSQLiteLib::LogErrors::No);

    //! Initializes a new instance of the ECDb class.
    ECDB_EXPORT ECDb();
    ECDB_EXPORT virtual ~ECDb();

    //! Gets the settings with which the ECDb object was constructed.
    Settings const& GetECDbSettings() const { return GetECDbSettingsManager().GetSettings(); }

    //! Checks the file's ECDb profile compatibility to be opened with the current version of the ECDb API.
    //!
    //! @remarks The caller must ensure that a transaction is active as this method needs to execute a SQL statement.
    //! All SQL statements, even SELECTs, require an active transaction in SQLite.
    //!
    //! @see BeSQLite::Db::OpenBeSQLiteDb for the compatibility contract for Bentley SQLite profiles.
    //! @param[out] fileIsAutoUpgradable Returns true if the file's ECDb profile version indicates that it is old, but auto-upgradeable.
    //!             false otherwise.
    //!             This method does @b not perform auto-upgrades. The out parameter just indicates to calling code
    //!             whether it has to perform the auto-upgrade or not.
    //! @param[in]  openModeIsReadonly true if the file is going to be opened in read-only mode. false if
    //!             the file is going to be opened in read-write mode.
    //! @return     BE_SQLITE_OK if ECDb profile can be opened in the requested mode, i.e. the compatibility contract is matched.
    //!             BE_SQLITE_ERROR_NoTxnActive if no transaction is active
    //!             BE_SQLITE_Error_ProfileTooOld if file's ECDb profile is too old to be opened by this API.
    //!             This error code is also returned if the file is old but not too old to be auto-upgraded.
    //!             Check @p fileIsAutoUpgradable to tell whether the file is auto-upgradeable and not.
    //!             BE_SQLITE_Error_ProfileTooNew if file's profile is too new to be opened by this API.
    //!             BE_SQLITE_Error_ProfileTooNewForReadWrite if file's profile is too new to be opened read-write, i.e. @p openModeIsReadonly is false
    ECDB_EXPORT DbResult CheckECDbProfileVersion(bool& fileIsAutoUpgradable, bool openModeIsReadonly) const;

    //! Gets the schema manager for this @ref ECDbFile "ECDb file". With the schema manager clients can import @ref ECN::ECSchema "ECSchemas"
    //! into or retrieve @ref ECN::ECSchema "ECSchemas" or individual @ref ECN::ECClass "ECClasses" from the %ECDb file.
    //! @return Schema manager
    ECDB_EXPORT SchemaManager const& Schemas() const;

    //! Gets the schema locator for schemas stored in this ECDb file.
    //! @return This ECDb file's schema locater
    ECDB_EXPORT ECN::IECSchemaLocaterR GetSchemaLocater() const;

    //! Gets the ECClass locator for ECClasses whose schemas are stored in this ECDb file.
    //! @return This ECDb file's ECClass locater
    ECDB_EXPORT ECN::IECClassLocaterR GetClassLocater() const;

    //! @name History
    //! @{

    //! Determines whether the Changes cache file is attached to this %ECDb file or not.
    //! @return true if the Changes cache file is attached and valid. false otherwise
    //! @see @ref ECDbChange
    ECDB_EXPORT bool IsChangeCacheAttached() const;

    //! Attaches the Changes cache file to this %ECDb file (if it isn't attached already).
    //! If it does not exist, a new one is created and attached.
    //! @note Attaching a file means that any open transactions are committed first (see BentleyApi::BeSQLite::Db::AttachDb).
    //! 
    //! Alternatively you can specify a corresponding ECDb::ChangeCacheMode so that the cache
    //! is attached at opening time already.
    //!
    //! @return BE_SQLITE_OK in case of success, error codes otherwise
    //! @see @ref ECDbChange
    //! @see BentleyApi::BeSQLite::EC::ECDb::OpenParams
    //! @see BentleyApi::BeSQLite::EC::ECDb::ChangeCacheMode
    ECDB_EXPORT DbResult AttachChangeCache() const;

    //! Gets the file path to the Change cache file for this %ECDb file.
    //! @return Path to Change cache file
    //! @see @ref ECDbChange
    ECDB_EXPORT BeFileName GetChangeCachePath() const;

    //! Gets the file path to the Change cache file for the specified %ECDb path.
    //! @param[in] ecdbPath Path to %ECDb file for which Change cache path is returned
    //! @return Path to Changes cache file
    //! @see @ref ECDbChange
    ECDB_EXPORT static BeFileName GetChangeCachePath(BeFileNameCR ecdbPath);

    //! Extracts and generates the change summary from the specified change set.
    //! @remarks The change summary is persisted as as instance of the ECClass @b ECDbChange.ChangeSummary and its related classes
    //! @b ECDbChange.InstanceChange and @b ECDbChange.PropertyValueChange.
    //! The method returns the ECInstanceKey of the generated ChangeSummary which serves as input to any query into the changes
    //! using the @b ECDbChange ECClasses or using the ECSQL function @b Changes.
    //!
    //! @note The change summaries are persisted in a separate cache file. Before extracting you must make sure
    //! the cache exists and has been attached. Either call ECDb::AttachChangeCache first or specify the appropriate ECDb::ChangeCacheMode
    //! when opening the %ECDb file. If the cache is not attached, the method returns ERROR.
    //!
    //! @param[out] changeSummaryKey Key of the generated change summary (of the ECClass @b ECDbChange.ChangeSummary)
    //! @param[in] changeSetInfo Change set and additional information about the change set to generate the summary from
    //! @param[in] options Extraction options
    //! @return SUCCESS or ERROR
    //! @see @ref ECDbChange
    ECDB_EXPORT BentleyStatus ExtractChangeSummary(ECInstanceKey& changeSummaryKey, ChangeSetArg const& changeSetInfo, ChangeSummaryExtractOptions const& options = ChangeSummaryExtractOptions()) const;
    
    //! @}

    //! Deletes orphaned ECInstances left over from operations specified by @p mode.
    //! @param[in] mode Purge mode
    //! @return SUCCESS or ERROR
    ECDB_EXPORT BentleyStatus Purge(PurgeMode mode) const;

    //! Adds a listener that listens to issues reported by this ECDb object.
    //! @remarks Only one listener can be added at a time.
    //! @param[in] issueListener Issue listener. The listener must remain valid
    //! while this ECDb is valid, or until it is removed via RemoveIssueListener.
    //! @return SUCCESS or ERROR if another lister was already added before.
    ECDB_EXPORT BentleyStatus AddIssueListener(IIssueListener const& issueListener);
    ECDB_EXPORT void RemoveIssueListener();


    ECDB_EXPORT void AddAppData(AppData::Key const& key, AppData* appData, bool deleteOnClearCache) const;
    using Db::AddAppData;

    //! Opens a Blob for incremental I/O for the specified ECProperty.
    //! @remarks The caller is responsible for closing/releasing the @p blobIO handle again.
    //! @param[in,out] blobIO The handle to open
    //! @param[in] ecClass The ECClass holding the Blob ECProperty
    //! @param[in] propertyAccessString The access string in @p ecClass to the ECProperty holding the blob to be opened.
    //! @param[in] ecInstanceId The ECInstanceId of the instance holding the blob.
    //! @param[in] writable If true, blob is opened for read/write access, otherwise it is opened readonly.
    //! @param[in] writeToken Token required if @p writable is true and if 
    //!            the ECDb file was set-up with the option "ECSQL write token validation".
    //!            If @p writable is false or if the option is not set, nullptr can be passed for @p writeToken.
    //! @return SUCCESS in case of success. ERROR in these cases:
    //!     - @p blobIO is already opened
    //!     - @p ecClass is not mapped to a table
    //!     - No ECProperty found in @p ecClass for @p propertyAccessString
    //!     - ECProperty is not primitive and not of type ECN::PRIMITIVETYPE_Binary or ECN::PRIMITIVETYPE_IGeometry
    //!     - ECProperty is not mapped to a column at all
    //!     - Write token validation failed
    //! @see BeSQLite::BlobIO
    BentleyStatus OpenBlobIO(BlobIO& blobIO, ECN::ECClassCR ecClass, Utf8CP propertyAccessString, BeInt64Id ecInstanceId, bool writable, ECCrudWriteToken const* writeToken = nullptr) const
        {
        return OpenBlobIO(blobIO, nullptr, ecClass, propertyAccessString, ecInstanceId, writable, writeToken);
        }

    //! Opens a Blob for incremental I/O for the specified ECProperty.
    //! @remarks The caller is responsible for closing/releasing the @p blobIO handle again.
    //! @param[in,out] blobIO The handle to open
    //! @param[in] tableSpace Table space containing the class - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one).
    //! @param[in] ecClass The ECClass holding the Blob ECProperty
    //! @param[in] propertyAccessString The access string in @p ecClass to the ECProperty holding the blob to be opened.
    //! @param[in] ecInstanceId The ECInstanceId of the instance holding the blob.
    //! @param[in] writable If true, blob is opened for read/write access, otherwise it is opened readonly.
    //! @param[in] writeToken Token required if @p writable is true and if 
    //!            the ECDb file was set-up with the option "ECSQL write token validation".
    //!            If @p writable is false or if the option is not set, nullptr can be passed for @p writeToken.
    //! @return SUCCESS in case of success. ERROR in these cases:
    //!     - @p blobIO is already opened
    //!     - @p ecClass is not mapped to a table
    //!     - No ECProperty found in @p ecClass for @p propertyAccessString
    //!     - ECProperty is not primitive and not of type ECN::PRIMITIVETYPE_Binary or ECN::PRIMITIVETYPE_IGeometry
    //!     - ECProperty is not mapped to a column at all
    //!     - Write token validation failed
    //! @see BeSQLite::BlobIO
    ECDB_EXPORT BentleyStatus OpenBlobIO(BlobIO& blobIO, Utf8CP tableSpace, ECN::ECClassCR ecClass, Utf8CP propertyAccessString, BeInt64Id ecInstanceId, bool writable, ECCrudWriteToken const* writeToken = nullptr) const;

    //! Clears the ECDb cache
    ECDB_EXPORT void ClearECDbCache(Utf8CP tableSpace = nullptr) const;

#if !defined (DOCUMENTATION_GENERATOR)
    Impl& GetImpl() const;
    bool HasImpl() const;
    void FireAfterSchemaImportEvent() const { _OnAfterSchemaImport(); }
#endif
};

typedef ECDb& ECDbR;
typedef ECDb const& ECDbCR;

END_BENTLEY_SQLITE_EC_NAMESPACE
