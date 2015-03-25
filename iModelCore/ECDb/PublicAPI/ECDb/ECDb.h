/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDb.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeSQLite/BeSQLite.h>
#include <ECDb/ECInstanceId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECDbSchemaManager;
struct ECSqlScalarFunction;

//=======================================================================================
//! ECDb is the %EC API used to access %EC data in an @ref ECDbFile "ECDb file".
//! 
//! It is used to create, open, close @ref ECDbFile "ECDb files" (see ECDb::CreateNewDb, ECDb::OpenBeSQLiteDb,
//! ECDb::CloseDb) and gives access to the %EC data.
//!
//! @see @ref ECDbOverview, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                Ramanujam.Raman      03/2012
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECDb : Db
{
public:
    struct Impl;

private:
    Impl* m_pimpl;

#if !defined (DOCUMENTATION_GENERATOR)
protected:
    ECDB_EXPORT virtual DbResult _OnDbOpened () override;
    //! Fired when an ECDb file was created. Creates the %EC profile.
    ECDB_EXPORT virtual DbResult _OnDbCreated (CreateParams const& params) override;
    ECDB_EXPORT virtual DbResult _OnRepositoryIdChanged (BeRepositoryId newRepositoryId) override;
    ECDB_EXPORT virtual void _OnDbClose () override;
    ECDB_EXPORT virtual void _OnDbChangedByOtherConnection () override;
    ECDB_EXPORT virtual DbResult _VerifySchemaVersion (Db::OpenParams const& params) override;
    ECDB_EXPORT virtual int _OnAddScalarFunction(ScalarFunction& func) const override;
    ECDB_EXPORT virtual void _OnRemoveFunction(DbFunction& func) const override;
#endif

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
    ECDB_EXPORT virtual ~ECDb ();

    //! Gets the schema manager for this @ref ECDbFile "ECDb file". With the schema manager clients can import @ref ECN::ECSchema "ECSchemas"
    //! into or retrieve @ref ECN::ECSchema "ECSchemas" or individual @ref ECN::ECClass "ECClasses" from the %ECDb file.
    //! @return Schema manager
    ECDB_EXPORT ECDbSchemaManager const& GetSchemaManager () const;

    //! Gets the schema locator for schemas stored in this ECDb file.
    //! @return This ECDb file's schema locater
    ECDB_EXPORT ECN::IECSchemaLocaterR GetSchemaLocater () const;

    //! Gets the ECClass locator for ECClasses whose schemas are stored in this ECDb file.
    //! @return This ECDb file's ECClass locater
    ECDB_EXPORT ECN::IECClassLocaterR GetClassLocater () const;

    //! Clears the ECDb cache
    ECDB_EXPORT void ClearECDbCache() const;

    //__PUBLISH_SECTION_END__
    Impl& GetECDbImplR () const;
    //__PUBLISH_SECTION_START__
};

//=======================================================================================
//! A user-defined scalar ECSQL function.
//! @remarks ECSQL functions right now only support primitive arguments and primitive return types -
//! where primitive types even exclude Point2D and Point3D.
//! Register an ECSQL function via ECDb::AddECSqlFunction.
//! The function object must survive as long as the ECDb to which it is added survives, or until it is removed.
//! It holds a pointer to an @ref BeSQLite::ScalarFunction::IScalar "IScalar" implementation. 
//! See discussion of scalar functions at http://www.sqlite.org/capi3ref.html#sqlite3_create_function.
//=======================================================================================
struct ECSqlScalarFunction : BeSQLite::ScalarFunction
    {
private:
    ECN::PrimitiveType m_returnType;

public:
    //! Initializes a new ECSqlFunction.
    //! @param[in] name Function name
    //! @param[in] argCount Number of arguments to the function
    //! @param[in] returnType Return type of the function
    //! @param[in] scalar The IScalar implementation called by SQLite when executing the function.
    ECSqlScalarFunction(Utf8CP name, int argCount, ECN::PrimitiveType returnType, BeSQLite::ScalarFunction::IScalar* scalar = nullptr)
        : BeSQLite::ScalarFunction(name, argCount, scalar), m_returnType(returnType) {}
   
    virtual ~ECSqlScalarFunction() {}

    //! Gets the return type of the function
    //! @return Function return type
    ECN::PrimitiveType GetReturnType() const { return m_returnType; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
