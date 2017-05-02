/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementImpl.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************************************************** 
// ECSqlStatement::Impl
//**********************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
//static
NativeLogging::ILogger* ECSqlStatement::Impl::s_prepareDiagnosticsLogger = nullptr;

#ifdef ECSQLPREPAREDSTATEMENT_REFACTOR

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::Prepare(ECDbCR ecdb, Utf8CP ecsql, ECCrudWriteToken const* writeToken)
    {
    if (IsPrepared())
        {
        LOG.error("ECSQL statement has already been prepared.");
        return ECSqlStatus::Error;
        }

    if (Utf8String::IsNullOrEmpty(ecsql))
        {
        LOG.error("ECSQL string is empty.");
        return ECSqlStatus::InvalidECSql;
        }

    BeMutexHolder lock(ecdb.GetECDbImplR().GetMutex());
    Diagnostics diag(ecsql, GetPrepareDiagnosticsLogger(), true);

    //Step 1: parse the ECSQL
    ECSqlParser parser;
    std::unique_ptr<Exp> exp = parser.Parse(ecdb, ecsql);
    if (exp == nullptr)
        {
        Finalize();
        return ECSqlStatus::InvalidECSql;
        }

    //Step 2: translate into SQLite SQL and prepare SQLite statement
    IECSqlPreparedStatement& preparedStatement = CreatePreparedStatement(ecdb, *exp);

    Policy policy = PolicyManager::GetPolicy(ECCrudPermissionPolicyAssertion(ecdb, preparedStatement.GetType() != ECSqlType::Select, writeToken));
    if (!policy.IsSupported())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(policy.GetNotSupportedMessage().c_str());
        Finalize();
        return ECSqlStatus::Error;
        }

    ECSqlPrepareContext ctx(preparedStatement);
    ECSqlStatus stat = preparedStatement.Prepare(ctx, *exp, ecsql);
    if (!stat.IsSuccess())
        Finalize();

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
IECSqlBinder& ECSqlStatement::Impl::GetBinder(int parameterIndex) const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call binding API on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return NoopECSqlBinder::Get();

    //Reports errors (not prepared yet, index out of bounds) and uses no-op binder in case of error
    return GetPreparedStatementP()->GetBinder(parameterIndex);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
int ECSqlStatement::Impl::GetParameterIndex(Utf8CP parameterName) const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call binding API on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return -1;

    //Reports errors (not prepared yet, index out of bounds) and uses no-op binder in case of error
    return GetPreparedStatementP()->GetParameterIndex(parameterName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::ClearBindings()
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call ClearBindings on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return stat;

    return GetPreparedStatementP()->ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
DbResult ECSqlStatement::Impl::Step()
    {
    if (!FailIfNotPrepared("Cannot call Step on an unprepared ECSQL statement.").IsSuccess())
        return BE_SQLITE_ERROR;

    //for performance reasons ECSqlPreparedStatement::Step is not polymorphic (anymore). Cost
    //of virtual dispatch was eliminated by taking cost of caller having to downcast to each subclass type
    //and call non-virtual Step.
    const ECSqlType ecsqlType = GetPreparedStatementP()->GetType();
    switch (ecsqlType)
        {
            case ECSqlType::Select:
                return GetPreparedStatementP<ECSqlSelectPreparedStatement>()->Step();

            case ECSqlType::Insert:
            {
            ECInstanceKey key;
            return GetPreparedStatementP<ECSqlInsertPreparedStatement>()->Step(key);
            }

            case ECSqlType::Update:
                return GetPreparedStatementP<ECSqlUpdatePreparedStatement>()->Step();

            case ECSqlType::Delete:
                return GetPreparedStatementP<ECSqlDeletePreparedStatement>()->Step();

            default:
                BeAssert(false && "Unhandled ECSqlType in ECSqlStatement::Step.");
                return BE_SQLITE_ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        11/13
//---------------------------------------------------------------------------------------
DbResult ECSqlStatement::Impl::Step(ECInstanceKey& ecInstanceKey)
    {
    if (!FailIfWrongType(ECSqlType::Insert, "Only call Step(ECInstanceKey&) on an ECSQL INSERT statement.").IsSuccess())
        return BE_SQLITE_ERROR;

    return GetPreparedStatementP<ECSqlInsertPreparedStatement>()->Step(ecInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::Reset()
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call Reset on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return stat;

    return GetPreparedStatementP()->Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
int ECSqlStatement::Impl::GetColumnCount() const
    {
    if (FailIfWrongType(ECSqlType::Select, "Cannot call query result API on an unprepared or non-SELECT ECSqlStatement.") != ECSqlStatus::Success)
        return -1;

    return GetPreparedStatementP<ECSqlSelectPreparedStatement>()->GetColumnCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/14
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlStatement::Impl::GetValue(int columnIndex) const
    {
    if (FailIfWrongType(ECSqlType::Select, "Cannot call query result API on an unprepared or non-SELECT ECSqlStatement.") != ECSqlStatus::Success)
        return NoopECSqlValue::GetSingleton();

    //Reports errors (not prepared yet, index out of bounds) and uses no-op value in case of error
    return GetPreparedStatementP<ECSqlSelectPreparedStatement>()->GetValue(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        06/14
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::Impl::GetECSql() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetECSql on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return GetPreparedStatementP()->GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::Impl::GetNativeSql() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetNativeSql on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return GetPreparedStatementP()->GetNativeSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        02/14
//---------------------------------------------------------------------------------------
ECDb const* ECSqlStatement::Impl::GetECDb() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetECDb on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return &GetPreparedStatementP()->GetECDb();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::FailIfNotPrepared(Utf8CP errorMessage) const
    {
    if (!IsPrepared())
        {
        LOG.error(errorMessage);
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::FailIfWrongType(ECSqlType expectedType, Utf8CP errorMessage) const
    {
    ECSqlStatus stat = FailIfNotPrepared(errorMessage);
    if (!stat.IsSuccess())
        return stat;

    if (GetPreparedStatementP()->GetType() != expectedType)
        {
        GetECDb()->GetECDbImplR().GetIssueReporter().Report(errorMessage);
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
IECSqlPreparedStatement& ECSqlStatement::Impl::CreatePreparedStatement(ECDbCR ecdb, Exp const& exp)
    {
    switch (exp.GetType())
        {
            case Exp::Type::Select:
                m_preparedStatement = std::make_unique<ECSqlSelectPreparedStatement>(ecdb);
                break;

            case Exp::Type::Insert:
                m_preparedStatement = std::make_unique<ECSqlInsertPreparedStatement>(ecdb);
                break;

            case Exp::Type::Update:
                m_preparedStatement = std::make_unique<ECSqlUpdatePreparedStatement>(ecdb);
                break;

            case Exp::Type::Delete:
                m_preparedStatement = std::make_unique<ECSqlDeletePreparedStatement>(ecdb);
                break;

            default:
                BeAssert(false && "ECSqlParseTree is expected to only be of type Select, Insert, Update, Delete");
                break;
        }

    return *m_preparedStatement;
    }
#else
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::_Prepare(ECSqlPrepareContext& ctx, Utf8CP ecsql)
    {
    // NB: This mutex is protecting some bmap in the schema cache. Move it there?
    BeMutexHolder lock(ctx.GetECDb().GetECDbImplR().GetMutex());
    Diagnostics diag(ecsql, GetPrepareDiagnosticsLogger(), true);
    return ECSqlStatementBase::_Prepare(ctx, ecsql);
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
//static
NativeLogging::ILogger& ECSqlStatement::Impl::GetPrepareDiagnosticsLogger()
    {
    if (s_prepareDiagnosticsLogger == nullptr)
        s_prepareDiagnosticsLogger = NativeLogging::LoggingManager::GetLogger(L"Diagnostics.ECSqlStatement.Prepare");

    BeAssert(s_prepareDiagnosticsLogger != nullptr);
    return *s_prepareDiagnosticsLogger;
    }

//********************************************************** 
// ECSqlStatement::Impl::Diagnostics
//**********************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
ECSqlStatement::Impl::Diagnostics::Diagnostics(Utf8CP ecsql, NativeLogging::ILogger& logger, bool startTimer)
    : m_logger(logger), m_timer(nullptr), m_ecsql(ecsql)
    {
    if (startTimer && CanLog())
        m_timer = std::unique_ptr<StopWatch>(new StopWatch(true));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
void ECSqlStatement::Impl::Diagnostics::Log()
    {
    try // as Log is called in destructor make sure that exceptions from log4cxx are caught
        {
        if (m_timer != nullptr)
            {
            m_timer->Stop();
            //use well-distinguishable delimiter to simplify loading the log output into a spread sheet
            //timing unit is not logged to simplify importing the diagnostics into a spreadsheet or DB.
            m_logger.messagev(LOG_SEVERITY, "%s | %.4f", m_ecsql, m_timer->GetElapsedSeconds() * 1000.0);
            }
        else
            m_logger.message(LOG_SEVERITY, m_ecsql);
        }
    catch (...)
        {
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
bool ECSqlStatement::Impl::Diagnostics::CanLog() const
    {
    return m_logger.isSeverityEnabled(LOG_SEVERITY);
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
