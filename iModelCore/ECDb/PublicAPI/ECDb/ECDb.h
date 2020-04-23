/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECInstanceId.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>
#include <ECObjects/ECObjectsAPI.h>
#include <json/json.h>
#include <ECDb/ConcurrentQueryManager.h>
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
    private:
        BeSQLite::IChangeSet& m_changeSet;
        Utf8String m_extendedPropertiesJson;

    public:
        //!Constructs a ChangeSetArg
        //!@param[in] changeSet SQLite Changeset
        explicit ChangeSetArg(BeSQLite::IChangeSet& changeSet) : m_changeSet(changeSet) {}
        //!Constructs a ChangeSetArg
        //!@param[in] changeSet SQLite Changeset
        //!@param[in] extendedPropertiesJson JSON additional properties about the ChangeSet which are inserted
        //!into the ChangeSummary::ExtendedProperties property.
        ChangeSetArg(BeSQLite::IChangeSet& changeSet, Utf8StringCR extendedPropertiesJson) : m_changeSet(changeSet), m_extendedPropertiesJson(extendedPropertiesJson) {}
        BeSQLite::IChangeSet& GetChangeSet() const { return m_changeSet; }
        Utf8String const& GetExtendedPropertiesJson() const { return m_extendedPropertiesJson; }
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
    //! Compile-time Settings that subclasses can set.
    // @bsiclass                                                    02/2017
    //+===============+===============+===============+===============+===============+======
    struct Settings final
        {
        private:
            bool m_requiresECCrudWriteToken = false;
            bool m_requiresECSchemaImportToken = false;

        public:
#if !defined (DOCUMENTATION_GENERATOR)
            //not inlined as ctors are only needed internally
            Settings();
            Settings(bool requiresECCrudWriteToken, bool m_requiresECSchemaImportToken);
#endif
            bool RequiresECCrudWriteToken() const { return m_requiresECCrudWriteToken; }
            bool RequiresECSchemaImportToken() const { return m_requiresECSchemaImportToken; }
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
        void ApplySettings(bool requireECCrudWriteToken, bool requireECSchemaImportToken);
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
    ECDB_EXPORT void ApplyECDbSettings(bool requireECCrudWriteToken, bool requireECSchemaImportToken);

    ECDB_EXPORT DbResult _OnDbOpening() override;
    ECDB_EXPORT DbResult _OnDbCreated(CreateParams const&) override;
    ECDB_EXPORT void _OnAfterSetBriefcaseId() override;
    ECDB_EXPORT void _OnDbClose() override;
    ECDB_EXPORT void _OnDbChangedByOtherConnection() override;
    ECDB_EXPORT ProfileState _CheckProfileVersion() const override;
    ECDB_EXPORT DbResult _UpgradeProfile() override;
    ECDB_EXPORT DbResult _OnDbAttached(Utf8CP fileName, Utf8CP dbAlias) const override;
    ECDB_EXPORT DbResult _OnDbDetached(Utf8CP dbAlias) const override;
    ECDB_EXPORT int _OnAddFunction(DbFunction&) const override;
    ECDB_EXPORT void _OnRemoveFunction(DbFunction&) const override;
    ECDB_EXPORT virtual DbResult _AfterSchemaChangeSetApplied() const;
    ECDB_EXPORT virtual DbResult _AfterDataChangeSetApplied();
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

    //! When ECDb::ClearECDbCache is called, this call back is called before the actual
    //! caches are cleared.
    //! This gives subclasses the opportunity to free anything that relies on the ECDb caches, e.g.
    //! ECSqlStatements or ECObjects entities.
    virtual void _OnBeforeClearECDbCache() const {}

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

    //! Check if the ECDb::Initialize() method was successfully called for current process or not.
    //! @return return true if ECDb::Initialize() method was successfully called.
    ECDB_EXPORT static bool IsInitialized();

    //! Initializes a new instance of the ECDb class.
    ECDB_EXPORT ECDb();
    ECDB_EXPORT virtual ~ECDb();

    //! Gets the version of the ECDb profile of this file.
    ECDB_EXPORT ProfileVersion const& GetECDbProfileVersion() const;

    //! Gets the current version of the ECDb profile
    static ProfileVersion CurrentECDbProfileVersion() { return ProfileVersion(4, 0, 0, 2); }
    //! Gets the minimum version of the ECDb profile for which in-situ upgrades are possible.
    //! Files with an older version cannot be upgraded in-situ.
    static ProfileVersion MinimumUpgradableECDbProfileVersion() { return ProfileVersion(4, 0, 0, 0); }

    //! Gets the settings with which the ECDb object was constructed.
    Settings const& GetECDbSettings() const { return GetECDbSettingsManager().GetSettings(); }

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

    //! @name EC Changes
    //! @{

    //! Determines whether the Change Cache file is attached to this %ECDb file or not.
    //! @return true if the Change Cache file is attached and valid. false otherwise
    //! @see @ref ECDbChange
    ECDB_EXPORT bool IsChangeCacheAttached() const;

    //! Attaches the Change cache file to this %ECDb file.
    //! If it does not exist, a new one is created and attached.
    //! If a Change Cache file is already attached, an error is returned.
    //! @note Attaching a file means that any open transactions are committed first (see BentleyApi::BeSQLite::Db::AttachDb).
    //! @param[in] changeCachePath Path to the Change Cache file.
    //! @return BE_SQLITE_OK in case of success, error codes otherwise
    //! @see @ref ECDbChange
    ECDB_EXPORT DbResult AttachChangeCache(BeFileNameCR changeCachePath) const;

    //! Detaches the Change Cache filefrom this %ECDb file, if it was attached.
    //! @note Detaching commits any open transactions first (see BentleyApi::BeSQLite::Db::DetachDb).
    //! @return BE_SQLITE_OK in case of success, error codes otherwise
    //! @see @ref ECDbChange
    ECDB_EXPORT DbResult DetachChangeCache() const;

    //! Creates a new Change Cache file for this %ECDb file but does not attach it.
    //! @remarks This method will return an error, if the cache file already exists.
    //!
    //! This method can also be used if further set-up on the Change Cache file is needed by the client.
    //! For example, higher layer code can create the Change Cache file using this method and import another schema.
    //! Note though that any of this is ignored by ECDb and must be maintained by the code that added it.
    //! @param[out] changeCacheFile Created Change Cache file which remains open
    //! @param[in] changeCacheFilePath Path for the Change Cache file. You can use ECDb::GetDefaultChangeCachePath
    //! to get the default path.
    //! @return BE_SQLITE_OK in case of success, error codes otherwise
    //! @see @ref ECDbChange
    ECDB_EXPORT DbResult CreateChangeCache(ECDb& changeCacheFile, BeFileNameCR changeCacheFilePath) const;

    //! Return a instance of concurrent query manager. It still require initialization.
    ECDB_EXPORT ConcurrentQueryManager& GetConcurrentQueryManager() const;

    //! Extracts and generates the change summary from the specified change set.
    //! @remarks The change summary is persisted as an instance of the ECClass @b ECDbChange.ChangeSummary and its related classes
    //! @b ECDbChange.InstanceChange and @b ECDbChange.PropertyValueChange.
    //! The method returns the ECInstanceKey of the generated ChangeSummary which serves as input to any query into the changes
    //! using the @b ECDbChange ECClasses or using the ECSQL function @b Changes.
    //!
    //! @note The change summaries are persisted in a separate Change Cache file. Before extracting you must make sure
    //! the Change Cache file exists and is attached. Call ECDb::AttachChangeCache first.
    //! If the Change Cache file does not exist or is not attached, the method returns ERROR.
    //!
    //! @param[out] changeSummaryKey Key of the generated change summary (of the ECClass @b ECDbChange.ChangeSummary)
    //! @param[in] changeSetArg Change set and additional information about the change set to generate the summary from
    //! @param[in] options Extraction options
    //! @return SUCCESS or ERROR
    //! @see @ref ECDbChange
    ECDB_EXPORT BentleyStatus ExtractChangeSummary(ECInstanceKey& changeSummaryKey, ChangeSetArg const& changeSetArg, ChangeSummaryExtractOptions const& options = ChangeSummaryExtractOptions()) const;

    //! Extracts and generates the change summary from the specified change set.
    //! @remarks The change summary is persisted as an instance of the ECClass @b ECDbChange.ChangeSummary and its related classes
    //! @b ECDbChange.InstanceChange and @b ECDbChange.PropertyValueChange.
    //! The method returns the ECInstanceKey of the generated ChangeSummary which serves as input to any query into the changes
    //! using the @b ECDbChange ECClasses or using the ECSQL function @b Changes.
    //!
    //! @note The change summaries are persisted in a separate Change Cache file file, specified by @p changeCacheFile.
    //! Before extracting you must make sure the Change Cache file exists. Either call ECDb::CreateChangeCache or ECDb::AttachChangeCache first.
    //! The Change Cache file doesn't have to be attached though.
    //! If the Change Cache file does not exist, the method returns ERROR.
    //!
    //! @param[out] changeSummaryKey Key of the generated change summary (of the ECClass @b ECDbChange.ChangeSummary)
    //! @param[in] changeCacheFile Open read-write connection to the Change Cache file
    //! @param[in] primaryFile Open connection to the primary ECDb file to which the changes refer (can be read-only)
    //! @param[in] changeSetArg Change set and additional information about the change set to generate the summary from
    //! @param[in] options Extraction options
    //! @return SUCCESS or ERROR
    //! @see @ref ECDbChange
    ECDB_EXPORT static BentleyStatus ExtractChangeSummary(ECInstanceKey& changeSummaryKey, ECDb& changeCacheFile, ECDb const& primaryFile, ChangeSetArg const& changeSetArg, ChangeSummaryExtractOptions const& options = ChangeSummaryExtractOptions());

    //! Gets the default file path to the Change Cache file for the specified %ECDb path.
    //! The default file path is: @p ecdbPath + ".ecchanges"
    //! @param[in] ecdbPath Path to %ECDb file for which Change Cache file path is returned
    //! @return Default path to Change Cache file
    //! @see @ref ECDbChange
    ECDB_EXPORT static BeFileName GetDefaultChangeCachePath(Utf8CP ecdbPath);

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

    //! Try to get a registered sql function by name
    //! @param[in,out] function return pointer to the sql function.
    //! @param[in] name function identifier
    //! @param[in] argCount number of arguments that the sql function takes.
    //! @return true if function successfully  find a function with the signature (name, argCount)
    ECDB_EXPORT bool TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const;

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
    //!
    //! ECDb caches various things in memory once they have been loaded from the ECDb file from disk.
    //! This includes publicly exposed objects like the ECSchema elements like ECClasses.
    //! Once this method has been called, <b>the objects that were previously held by the cache must
    //! no longer be used</b>.
    //! Likewise any objects that have referenced the cached information may no longer be used anymore either.
    //! This includes ECSqlStatements and anything that uses them like the ECSQL adapters or an ECSqlStatementCache.
    //!
    //! @note ECDb also calls this method internally, e.g. within ECDb::ImportSchemas, ECDb::DetachChangeCache or BeSQLite::Db::DetachDb.
    //! The same rules described above therefore apply after having called those methods.
    //!
    //! @note For subclass implementors: calling this method fires the _OnBeforeClearECDbCache callback. This allows subclasses
    //! to free any items that referred to the objects in the cache.
    ECDB_EXPORT void ClearECDbCache() const;

    BeSQLite::DbResult AfterSchemaChangeSetApplied() const { return _AfterSchemaChangeSetApplied(); }
    BeSQLite::DbResult AfterDataChangeSetApplied() { return _AfterDataChangeSetApplied(); }

#if !defined (DOCUMENTATION_GENERATOR)
    Impl& GetImpl() const;
#endif
};

typedef ECDb& ECDbR;
typedef ECDb const& ECDbCR;

END_BENTLEY_SQLITE_EC_NAMESPACE
