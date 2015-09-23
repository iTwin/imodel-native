/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECSqlStatus.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeSQLite/BeSQLite.h>
#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! The status codes returned by ECSqlStatement
//! @ingroup ECDbGroup
// @bsienum                                                      Krischan.Eberle   07/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlStatus
    {
public:
    enum class Status : uint8_t
        {
        Success = SUCCESS, //!< Success
        Error, //!< Error which is not further specified
        InvalidECSql, //!< An invalid ECSQL (e.g. because of a syntax or semantic error in the ECSQL text) was passed to ECSqlStatement::Prepare.
        SQLiteError //!< A SQLite error occurred
        };

    ECDB_EXPORT static const ECSqlStatus Success;
    ECDB_EXPORT static const ECSqlStatus Error;
    ECDB_EXPORT static const ECSqlStatus InvalidECSql;

private:
    Status m_status;
    DbResult m_sqliteError;

public:
    ECSqlStatus() : m_status(Status::Success), m_sqliteError(BE_SQLITE_OK) {}
    explicit ECSqlStatus(Status stat) : m_status(stat), m_sqliteError(stat == Status::SQLiteError ? BE_SQLITE_ERROR : BE_SQLITE_OK) {};
    explicit ECSqlStatus(DbResult sqliteError) : m_status(Status::SQLiteError), m_sqliteError(sqliteError) { BeAssert(sqliteError != BE_SQLITE_OK && "Use default ctor in this case"); };

    bool operator==(ECSqlStatus const& rhs) const { return m_status == rhs.m_status && m_sqliteError == rhs.m_sqliteError; }
    bool operator!=(ECSqlStatus const& rhs) const { return !(*this == rhs); }
    operator ECSqlStatus::Status() const { return m_status; }

    //! Gets the ECSqlStatus value.
    Status Get() const { return m_status; }

    //! Indicates whether the ECSqlStatus object represents ECSqlStatus::Status::Success or not.
    bool IsSuccess() const { return m_status == Status::Success; }

    //! Indicates whether this is a SQLite error
    //! If yes, you can call ECSqlStatus::GetSQLiteError to get the detailed SQLite error code
    bool IsSQLiteError() const { return m_status == Status::SQLiteError; }
    //! Gets the detailed SQLite error if this ECSqlStatus is ECSqlStatus::Status::SQLiteError
    DbResult GetSQLiteError() const { return m_sqliteError; }

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
