/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatusContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                               Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlStatusContext //: NonCopyableClass
    {
private:    
    ECSqlStatus m_status;
    Utf8String m_ecsql;
    Utf8String m_statusMessage;

    //! Gets the status message as passed to the method 
    //! @return Status message
    Utf8CP GetStatusMessage () const;
    //! Logs the current status and message and warnings.
    void Log () const;
    static Utf8String ToString (ECSqlStatus status, Utf8CP ecsql, Utf8CP statusMessage);

public:
    ECSqlStatusContext ()
        : m_status (ECSqlStatus::Success)
        {}

    explicit ECSqlStatusContext (Utf8CP ecsql)
        : m_status (ECSqlStatus::Success), m_ecsql (ecsql)
            {}
        
    ~ECSqlStatusContext () {}
    ECSqlStatusContext (ECSqlStatusContext&& rhs);
    ECSqlStatusContext& operator= (ECSqlStatusContext&& rhs);

    //! Sets the error status and message in the context.
    //! @remarks Don't call it with ECSqlStatus::Success. Call Reset instead to clear out the previous state
    //! @param[in] status ECSQL status
    //! @param[in] message Status message
    //! @return ECSQL status (same as @p status)
    ECSqlStatus SetError (ECSqlStatus status, Utf8CP message, ...);

    //! Sets an error status and message in the context.
    //! @remarks Do not call it with ECSqlStatus::Success, @p message will be ignored and the status message in the context be cleared.
    //! @param[in] status ECSQL status
    //! @param[in] message Status message
    //! @param[in] args Arguments for the placeholders in @p message
    //! @return ECSQL status (same as @p status)
    ECSqlStatus SetErrorV (ECSqlStatus status, Utf8CP message, va_list args);

    //! Maps the SQLite status to an ECSQL status and sets it in the context.
    //! @param[in] ecdb ECDb handle to retrieve SQLite error message. Can be nullptr
    //! @param[in] sqliteStatus BeSQLite status as returned from SQLite / BeSQLite call
    //! @param[in] messageHeader Message header to which SQLite error message will be appended. Can be nullptr.
    //! @return ECSQL status
    ECSqlStatus SetError (ECDbCP ecdb, DbResult sqliteStatus, Utf8CP messageHeader = nullptr);

    //! Resets the status context. All status and warning messages will be removed.
    //! @return Always ECSqlStatus::Success which is the default status after a reset
    ECSqlStatus Reset ();

    //! Indicates whether the current status is ECSqlStatus::Success or not.
    //! @return true, if current status is ECSqlStatus::Success. false otherwise
    bool IsSuccess () const;

    //! Gets the current status.
    //! @return ECSQL status
    ECSqlStatus GetStatus () const;
        
    //! Generates a verbose status message (excluding warnings) which includes
    //! the corresponding ECSQL string and the actual status value.
    //! @return Verbose status message
    Utf8String ToString () const;

    //! Maps a BeSQLite status to an ECSqlStatus. 
    //! @param[in] sqliteStatus BeSQLite status to map
    //! @return ECSqlStatus
    static ECSqlStatus MapSqliteStatus (DbResult sqliteStatus);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

