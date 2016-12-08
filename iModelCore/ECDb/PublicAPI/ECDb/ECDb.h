/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDb.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDbTypes.h>
#include <BeSQLite/BeSQLite.h>
#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECDbSchemaManager;
struct ECSqlScalarFunction;

//=======================================================================================
//! Severity of an ECDb issue
// @bsiclass                                                Krischan.Eberle      09/2015
//+===============+===============+===============+===============+===============+======
enum class ECDbIssueSeverity
    {
    Warning, //!< Warning
    Error //!< Error
    };

struct ECSqlWriteToken;

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
    //! Modes for the ECDb::Purge method.
    // @bsiclass                                                Krischan.Eberle      11/2015
    //+===============+===============+===============+===============+===============+======
    enum class PurgeMode
        {
        FileInfoOwnerships = 1, //!< Purges FileInfoOwnership instances (see also @ref ECDbFileInfo)
        All = FileInfoOwnerships
        };

    struct Impl;

    //=======================================================================================
    //! Allows clients to be notified of error or warning messages.
    //! @remarks ECDb cares for logging any error and warnings sent to listeners via BentleyApi::NativeLogging. 
    //! So implementors
    //! don't have to do that anymore.
    // @bsiclass                                                Krischan.Eberle      09/2015
    //+===============+===============+===============+===============+===============+======
    struct IIssueListener
        {
    private:
        //! Fired by ECDb whenever an issue occurred during the schema import.
        //! @param[in] severity Issue severity
        //! @param[in] message Issue message
        virtual void _OnIssueReported(ECDbIssueSeverity severity, Utf8CP message) const = 0;

    protected:
        IIssueListener() {}

    public:
        virtual ~IIssueListener() {}

#if !defined (DOCUMENTATION_GENERATOR)
        //! Called by ECDb to report an issue to clients.
        //! @param[in] severity Issue severity
        //! @param[in] message Issue message
        void ReportIssue(ECDbIssueSeverity severity, Utf8CP message) const;
#endif
        };

private:
    Impl* m_pimpl;

#if !defined (DOCUMENTATION_GENERATOR)
protected:
    ECDB_EXPORT virtual DbResult _OnDbOpening() override;
    ECDB_EXPORT virtual DbResult _OnDbCreated(CreateParams const&) override;
    ECDB_EXPORT virtual DbResult _OnBriefcaseIdChanged(BeBriefcaseId newBriefcaseId) override;
    ECDB_EXPORT virtual void _OnDbClose() override;
    ECDB_EXPORT virtual void _OnDbChangedByOtherConnection() override;
    ECDB_EXPORT virtual DbResult _VerifySchemaVersion(Db::OpenParams const&) override;
    ECDB_EXPORT virtual int _OnAddFunction(DbFunction&) const override;
    ECDB_EXPORT virtual void _OnRemoveFunction(DbFunction&) const override;

    //! If called, consumers can only execute ECSQL INSERT, UPDATE, DELETE statements
    //! with the write token returned from this method. 
    //! Subclasses can call this if they don't want consumers to modify data via ECSQL directly,
    //! and provide their own data modifying APIs.
    ECDB_EXPORT ECSqlWriteToken const& EnableECSqlWriteTokenValidation();
#endif

    virtual void _OnAfterECSchemaImport() const {}

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

    //! Gets the schema manager for this @ref ECDbFile "ECDb file". With the schema manager clients can import @ref ECN::ECSchema "ECSchemas"
    //! into or retrieve @ref ECN::ECSchema "ECSchemas" or individual @ref ECN::ECClass "ECClasses" from the %ECDb file.
    //! @return Schema manager
    ECDB_EXPORT ECDbSchemaManager const& Schemas() const;

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

#if !defined (DOCUMENTATION_GENERATOR)
    //! Clears the ECDb cache (not exported until we decided whether we need consumers to call it directly or not)
    void ClearECDbCache() const;
    Impl& GetECDbImplR() const;
    void FireAfterECSchemaImportEvent() const { _OnAfterECSchemaImport(); }
#endif
};

typedef ECDb& ECDbR;
typedef ECDb const& ECDbCR;

END_BENTLEY_SQLITE_EC_NAMESPACE
