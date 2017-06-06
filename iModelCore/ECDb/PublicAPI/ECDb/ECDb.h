/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDb.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDbTypes.h>
#include <BeSQLite/BeSQLite.h>
#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct SchemaManager;

struct ECCrudWriteToken;
struct SchemaImportToken;

//=======================================================================================
//! ECDb is the %EC API used to access %EC data in an @ref ECDbFile "ECDb file".
//! 
//! It is used to create, open, close @ref ECDbFile "ECDb files" (see ECDb::CreateNewDb, ECDb::OpenBeSQLiteDb,
//! ECDb::CloseDb) and gives access to the %EC data.
//!
//! The following SQLite functions are built into ECDb (and can be used in SQL and ECSQL):
//!     * TEXT BlobToBase64(BLOB blob) Encodes a BLOB as its Base64 string representation
//!     * BLOB Base64ToBlob(TEXT base64Str) Decodes a Base64 string to a BLOB
//!
//! An ECDb object is generally thread-safe. However an ECDb connection must be closed in the same thread in which is was opened.
//! @see @ref ECDbOverview, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                Ramanujam.Raman      03/2012
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECDb : Db
{
public:
    //=======================================================================================
    // @bsiclass                                                Krischan.Eberle      02/2017
    //+===============+===============+===============+===============+===============+======
    struct Settings final
        {
    private:
        ECCrudWriteToken const* m_crudWriteToken = nullptr;
        SchemaImportToken const* m_schemaImportToken = nullptr;
        bool m_allowChangesetMergingIncompatibleSchemaImport = true;

    public:
#if !defined (DOCUMENTATION_GENERATOR)
        //not inlined as ctors are only needed internally
        Settings();
        Settings(ECCrudWriteToken const*, SchemaImportToken const*, bool allowChangesetMergingIncompatibleSchemaImport);
#endif
        //! Consumers can only execute EC CRUD operations like ECSQL INSERT, UPDATE or DELETE statements
        ECCrudWriteToken const* GetCrudWriteToken() const { return m_crudWriteToken; }
        //! Consumers can only import ECSchemas with the token
        SchemaImportToken const* GetSchemaImportToken() const { return m_schemaImportToken; }

        bool AllowChangesetMergingIncompatibleSchemaImport() const { return m_allowChangesetMergingIncompatibleSchemaImport; }
        };

    //=======================================================================================
    //! Modes for the ECDb::Purge method.
    // @bsiclass                                                Krischan.Eberle      11/2015
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
    // @bsiclass                                                Krischan.Eberle      09/2015
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
    ECDB_EXPORT DbResult _OnDbCreated(CreateParams const&) override;
    ECDB_EXPORT DbResult _OnBriefcaseIdAssigned(BeBriefcaseId newBriefcaseId) override;
    ECDB_EXPORT void _OnDbClose() override;
    ECDB_EXPORT void _OnDbChangedByOtherConnection() override;
    ECDB_EXPORT DbResult _VerifyProfileVersion(Db::OpenParams const&) override;
    ECDB_EXPORT int _OnAddFunction(DbFunction&) const override;
    ECDB_EXPORT void _OnRemoveFunction(DbFunction&) const override;

    ECDB_EXPORT Settings const& GetECDbSettings() const;
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
    ECDB_EXPORT BentleyStatus OpenBlobIO(BlobIO& blobIO, ECN::ECClassCR ecClass, Utf8CP propertyAccessString, BeInt64Id ecInstanceId, bool writable, ECCrudWriteToken const* writeToken = nullptr) const;

    //! Clears the ECDb cache
    ECDB_EXPORT void ClearECDbCache() const;

#if !defined (DOCUMENTATION_GENERATOR)
    Impl& GetECDbImplR() const;
    void FireAfterSchemaImportEvent() const { _OnAfterSchemaImport(); }
#endif
};

typedef ECDb& ECDbR;
typedef ECDb const& ECDbCR;

END_BENTLEY_SQLITE_EC_NAMESPACE
