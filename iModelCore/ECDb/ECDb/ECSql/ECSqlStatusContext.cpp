/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatusContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatusContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
ECSqlStatusContext::ECSqlStatusContext (ECSqlStatusContext&& rhs)
    : m_status (std::move (rhs.m_status)), m_ecsql (std::move (rhs.m_ecsql)), 
      m_statusMessage (std::move (rhs.m_statusMessage))
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
ECSqlStatusContext& ECSqlStatusContext::operator= (ECSqlStatusContext&& rhs)
    {
    if (this != &rhs)
        {
        m_status = std::move (rhs.m_status);
        m_ecsql = std::move (rhs.m_ecsql);
        m_statusMessage = std::move (rhs.m_statusMessage);
        }

    return *this;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatusContext::SetError (ECSqlStatus status, Utf8CP fmt, ...)
    {
    va_list args;
    va_start(args, fmt);
    const auto stat = SetErrorV (status, fmt, args);
    va_end (args);
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatusContext::SetErrorV (ECSqlStatus status, Utf8CP fmt, va_list args)
    {
    BeAssert (status != ECSqlStatus::Success && "ECSqlStatusContext::SetErrorV should not be called with ECSqlStatus::Success");
    if (status == ECSqlStatus::Success)
        m_statusMessage.clear ();
    else
        m_statusMessage.VSprintf(fmt, args);

    m_status = status; 

    Log ();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatusContext::SetError (ECDbCP ecdb, DbResult sqliteStatus, Utf8CP messageHeader)
    {
    const auto stat = MapSqliteStatus (sqliteStatus);
    BeAssert (stat != ECSqlStatus::Success && "ECSqlStatusContext::SetError should not be called in case of success.");
    if (stat == ECSqlStatus::Success)
        return Reset ();
    
    Utf8String fullMessage (Utf8String::IsNullOrEmpty (messageHeader) ? "SQLite error" : messageHeader);
    fullMessage.append (" ").append (ECDb::InterpretDbResult (sqliteStatus));
    
    if (ecdb != nullptr)
        fullMessage.append (": ").append (ecdb->GetLastError ());
    
    return SetError (stat, fullMessage.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatusContext::Reset ()
    {
    if (m_status != ECSqlStatus::Success)
        {
        m_status = ECSqlStatus::Success;
        m_statusMessage.clear ();
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
bool ECSqlStatusContext::IsSuccess () const 
    { 
    return m_status == ECSqlStatus::Success;
    } 

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatusContext::GetStatus () const
    {
    return m_status;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatusContext::GetStatusMessage () const 
    { 
    return m_statusMessage.c_str ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
Utf8String ECSqlStatusContext::ToString () const
    {
    return ToString (m_status, m_ecsql.c_str (), GetStatusMessage ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
//static 
Utf8String ECSqlStatusContext::ToString (ECSqlStatus status, Utf8CP ecsql, Utf8CP statusMessage)
    {
    Utf8String str;
    switch (status)
        {
        case ECSqlStatus::Success:
            return "";

        case ECSqlStatus::InvalidECSql:
            {
            str = "Invalid ECSQL";
            break;
            }

        case ECSqlStatus::NotYetSupported:
            {
            Utf8String str ("Not yet supported: ");
            str.append (statusMessage);                
            return str;
            }

        case ECSqlStatus::ProgrammerError:
            {
            str = "Programmer error";
            break;
            }

        default:
            break;
        }

    if (!Utf8String::IsNullOrEmpty (ecsql))
        {
        if (!str.empty ())
            str.append (" ");

        str.append ("'");
        str.append (ecsql);
        str.append ("'");
        }

    if (!Utf8String::IsNullOrEmpty (statusMessage))
        {
        if (!str.empty ())
            str.append (": ");

        str.append (statusMessage);
        }

    return str;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
void ECSqlStatusContext::Log () const
    {
    if (m_status != ECSqlStatus::Success && LOG.isSeverityEnabled (NativeLogging::LOG_ERROR))
        LOG.error (ToString ().c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
//static 
ECSqlStatus ECSqlStatusContext::MapSqliteStatus (DbResult sqliteStatus)
    {
    if (BE_SQLITE_OK == sqliteStatus)
        return ECSqlStatus::Success;

    if (BE_SQLITE_CONSTRAINT_BASE == (sqliteStatus & BE_SQLITE_CONSTRAINT_BASE))
        return ECSqlStatus::ConstraintViolation;
    
    switch (sqliteStatus)
        {
        case BE_SQLITE_RANGE:
            return ECSqlStatus::IndexOutOfBounds;

        //These are considered programmer errors. We might refine this while seeing situations where a SQLite failure
        //is not an internal programmer error
        case BE_SQLITE_INTERNAL:
        case BE_SQLITE_MISMATCH:
        case BE_SQLITE_MISUSE:
        case BE_SQLITE_NOLFS:
        case BE_SQLITE_SCHEMA:
        //these should never show up here as they are handled by the Step logic
        case BE_SQLITE_DONE:
        case BE_SQLITE_ROW:
            return ECSqlStatus::ProgrammerError;

        default:
            return ECSqlStatus::InvalidECSql;
        }
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
