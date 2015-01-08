/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/ECDb/ECDb.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ECDb/ECDbTypes.h>
#include <BeSQLite/ECDb/ECDbStore.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECDb is the %EC API used to access %EC data in an @ref ECDbFile "ECDb file".
//! 
//! It is used to create, open, close @ref ECDbFile "ECDb files" (see ECDb::CreateNewDb, ECDb::OpenBeSQLiteDb,
//! ECDb::CloseDb) and gives access to the %EC data via ECDb::GetEC.
//!
//! @see @ref ECDbOverview, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                Ramanujam.Raman      03/2012
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECDb : Db
{
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(Db);
//__PUBLISH_SECTION_START__

private:
    ECDbStorePtr m_ecDbStore;

#if !defined (DOCUMENTATION_GENERATOR)
protected:
    ECDB_EXPORT virtual DbResult _OnDbOpened () override;
    //! Fired when an ECDb file was created. Creates the %EC profile.
    ECDB_EXPORT virtual DbResult _OnDbCreated (CreateParams const& params) override;
    ECDB_EXPORT virtual DbResult _OnRepositoryIdChanged (BeRepositoryId newRepositoryId) override;
    ECDB_EXPORT virtual void _OnDbClose () override;
    ECDB_EXPORT virtual void _OnDbChangedByOtherConnection () override;
    ECDB_EXPORT virtual DbResult _VerifySchemaVersion (Db::OpenParams const& params) override;
#endif

//__PUBLISH_SECTION_END__
    //! Get the handle to access this database through the %EC API
    //! @remarks Externally (i.e. in subclasses), this method is only to be
    //!          used for sharing the same ECDbStore instance
    //! across multiple ECDb instances
    ECDB_EXPORT ECDbStorePtr GetECDbStore () const;
//__PUBLISH_SECTION_START__

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
    ECDB_EXPORT static DbResult Initialize (BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir = nullptr, BeSQLiteLib::LogErrors logSqliteErrors=BeSQLiteLib::LogErrors::No);

    //! Initializes a new instance of the ECDb class.
    ECDB_EXPORT ECDb ();

//__PUBLISH_SECTION_END__
    //! Initializes a new instance of the ECDb class with a shared ECDbStore instance.
    //! @param[in] sharedECDbStore ECDbStore instance to be used by this ECDb instance
    ECDB_EXPORT explicit ECDb (ECDbStorePtr sharedECDbStore);
//__PUBLISH_SECTION_START__

    ECDB_EXPORT virtual ~ECDb ();

    //! Gets the handle to the %EC API
    //! @return %EC API handle
    ECDB_EXPORT ECDbStoreCR GetEC () const;

    //__PUBLISH_SECTION_END__
    ECDbStore& GetECR () const;
    //__PUBLISH_SECTION_START__

};

END_BENTLEY_SQLITE_EC_NAMESPACE
