/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementImpl.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatementImpl.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//********************************************************** 
// ECSqlStatement::Impl
//**********************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
//static
NativeLogging::ILogger* ECSqlStatement::Impl::s_prepareDiagnosticsLogger = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        01/16
//---------------------------------------------------------------------------------------
ECSqlStatement::Impl::~Impl()
    {
    ECDb const* ecdb = GetECDb();
    if (ecdb != nullptr)
        UnregisterFromRegistry(*ecdb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::_Prepare (ECDbCR ecdb, Utf8CP ecsql)
    {
    Diagnostics diag (ecsql, GetPrepareDiagnosticsLogger (), true);
    
    ECSqlStatus stat = ECSqlStatementBase::_Prepare(ecdb, ecsql);
    if (stat.IsSuccess())
        ecdb.GetECDbImplR().GetStatementRegistry().Add(*this);

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlPrepareContext ECSqlStatement::Impl::_InitializePrepare (ECDbCR ecdb, Utf8CP ecsql)
    {
    return ECSqlPrepareContext(ecdb, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2016
//---------------------------------------------------------------------------------------
void ECSqlStatement::Impl::Finalize(bool removeFromRegistry)
    {
    if (removeFromRegistry)
        {
        ECDb const* ecdb = GetECDb();
        if (ecdb != nullptr)
            UnregisterFromRegistry(*ecdb);
        }

    ECSqlStatementBase::_Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2016
//---------------------------------------------------------------------------------------
void ECSqlStatement::Impl::_Finalize()
    {
    Finalize(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::Reprepare()
    {
    if (!IsPrepared())
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    ECDb const* ecdb = GetECDb();
    BeAssert(ecdb != nullptr);
    Utf8String ecsql = GetECSql();
    //finalize the statement, but don't remove it from the registry as we will reprepare it
    Finalize(false);

    ECSqlStatus stat = Prepare(*ecdb, ecsql.c_str());
    if (!stat.IsSuccess())
        {
        UnregisterFromRegistry(*ecdb);
        ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning, "Repreparation of the ECSqlStatement (%s) failed. ECSqlStatement was finalized.", ecsql.c_str());
        }
    else
        LOG.infov("Reprepared ECSqlStatement (%s).", ecsql.c_str());

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2016
//---------------------------------------------------------------------------------------
void ECSqlStatement::Impl::UnregisterFromRegistry(ECDbCR ecdb)
    {
    ecdb.GetECDbImplR().GetStatementRegistry().Remove(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
//static
NativeLogging::ILogger& ECSqlStatement::Impl::GetPrepareDiagnosticsLogger ()
    {
    if (s_prepareDiagnosticsLogger == nullptr)
        s_prepareDiagnosticsLogger = NativeLogging::LoggingManager::GetLogger (L"Diagnostics.ECSqlStatement.Prepare");

    BeAssert (s_prepareDiagnosticsLogger != nullptr);
    return *s_prepareDiagnosticsLogger;
    }


//********************************************************** 
// ECSqlStatement::Impl::Diagnostics
//**********************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
ECSqlStatement::Impl::Diagnostics::Diagnostics (Utf8CP ecsql, NativeLogging::ILogger& logger, bool startTimer)
: m_logger (logger), m_timer (nullptr), m_ecsql (ecsql)
    {
    if (startTimer && CanLog ())
        m_timer = std::unique_ptr<StopWatch> (new StopWatch (true));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
void ECSqlStatement::Impl::Diagnostics::Log ()
    {
    try // as Log is called in destructor make sure that exceptions from log4cxx are caught
        {
        if (m_timer != nullptr)
            {
            m_timer->Stop ();
            //use well-distinguishable delimiter to simplify loading the log output into a spread sheet
            //timing unit is not logged to simplify importing the diagnostics into a spreadsheet or DB.
            m_logger.messagev (LOG_SEVERITY, "%s | %.4f", m_ecsql, m_timer->GetElapsedSeconds () * 1000.0);
            }
        else
            m_logger.message (LOG_SEVERITY, m_ecsql);
        }
    catch (...)
        {
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
bool ECSqlStatement::Impl::Diagnostics::CanLog () const
    {
    return m_logger.isSeverityEnabled (LOG_SEVERITY);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
