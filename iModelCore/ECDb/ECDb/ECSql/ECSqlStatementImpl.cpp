/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//**********************************************************
// ECSqlStatement::Impl
//**********************************************************
#ifndef NDEBUG
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
NativeLogging::CategoryLogger s_logger("Diagnostics.ECSqlStatement.Prepare");
NativeLogging::CategoryLogger* ECSqlStatement::Impl::s_prepareDiagnosticsLogger = &s_logger;
#endif
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::Prepare(ECDbCR ecdb, Db const* dataSourceECDb, Utf8CP ecsql, ECCrudWriteToken const* writeToken, bool logErrors)
    {
    m_hash64 = nullptr;
    auto filterAction = logErrors ?  IssueDataSource::FilterAction::Forward : IssueDataSource::FilterAction::Ignore;
    IssueDataSource::FilterScope filteredScope(ecdb.GetImpl().Issues(), [=](ECN::IssueSeverity, ECN::IssueCategory, ECN::IssueType, Utf8CP) { return filterAction; });

    //Verify that dataSourceECDb meets all necessary conditions
    if (dataSourceECDb != nullptr && dataSourceECDb != &ecdb)
        {
        if (!dataSourceECDb->IsDbOpen())
            {
            filteredScope.Source().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Failed to prepare '%s'. The data source ECDb (parameter 'dataSourceECDb') is not open.", ecsql);
            return ECSqlStatus::Error;
            }

        if (!dataSourceECDb->IsReadonly())
            {
            filteredScope.Source().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Failed to prepare '%s'. The data source ECDb (parameter 'dataSourceECDb') is not a read-only connection.", ecsql);
            return ECSqlStatus::Error;
            }

        if (dataSourceECDb->GetDbGuid() != ecdb.GetDbGuid() || BeStringUtilities::Stricmp(dataSourceECDb->GetDbFileName(), ecdb.GetDbFileName()) != 0)
            {
            filteredScope.Source().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Failed to prepare '%s'. The data source ECDb (parameter 'dataSourceECDb') must be a connection to the same ECDb file as the ECSQL parsing ECDb connection (parameter 'ecdb').", ecsql);
            return ECSqlStatus::Error;
            }
        }

    if (IsPrepared())
        {
        filteredScope.Source().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "ECSQL statement has already been prepared.");
        return ECSqlStatus::Error;
        }

    if (Utf8String::IsNullOrEmpty(ecsql))
        {
        filteredScope.Source().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "ECSQL string is empty.");
        return ECSqlStatus::InvalidECSql;
        }

#ifndef NDEBUG
    Diagnostics diag(ecsql, GetPrepareDiagnosticsLogger(), true);
#endif
    //Step 1: parse the ECSQL
    ECSqlParser parser;
    std::unique_ptr<Exp> exp = parser.Parse(ecdb, ecsql, filteredScope.Source());
    if (exp == nullptr)
        {
        Finalize();
        return ECSqlStatus::InvalidECSql;
        }

    //Step 2: translate into SQLite SQL and prepare SQLite statement
    IECSqlPreparedStatement& preparedStatement = CreatePreparedStatement(ecdb, *exp);

    Policy policy = PolicyManager::GetPolicy(ECCrudPermissionPolicyAssertion(ecdb, preparedStatement.GetType() != ECSqlType::Select  && preparedStatement.GetType() != ECSqlType::Pragma , writeToken));
    if (!policy.IsSupported())
        {
        filteredScope.Source().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, policy.GetNotSupportedMessage().c_str());
        Finalize();
        return ECSqlStatus::Error;
        }

    //if dataSourceECDb is nullptr, the primary ECDb is used
    ECSqlPrepareContext ctx(preparedStatement, dataSourceECDb != nullptr ? *dataSourceECDb : ecdb, filteredScope.Source());
    ECSqlStatus stat = preparedStatement.Prepare(ctx, *exp, ecsql);
    if (!stat.IsSuccess())
        Finalize();

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlStatement::Impl::IsReadonly() const {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call IsReadonly on on unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return sal_False;

    if (GetPreparedStatementP()->GetType() == ECSqlType::Select) {
        return GetPreparedStatementP<ECSqlSelectPreparedStatement>()->GetSqliteStatement().IsReadonly();
    }
    return false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint64_t ECSqlStatement::Impl::GetHashCode() const {
    if (m_hash64.IsNull()){
        if (m_preparedStatement != nullptr && m_preparedStatement->GetECSql() != nullptr) {
            FNV1HashBuilder builder;
            builder.UpdateString(m_preparedStatement->GetECSql());
            m_hash64 = builder.GetHashCode();
        } else {
            m_hash64 = UINT64_C(0);
        }
    }
    return m_hash64.Value();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint64_t ECSqlStatement::Impl::GetHashCode(Utf8CP ecsql)
    {
    if (ecsql == nullptr)
        return UINT64_C(0);

    FNV1HashBuilder builder;
    builder.UpdateCharCP(ecsql);
    return builder.GetHashCode();
    }
//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
int ECSqlStatement::Impl::TryGetParameterIndex(Utf8CP parameterName) const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call binding API on an unprepared ECSqlStatement.");
    return stat.IsSuccess() ? GetPreparedStatementP()->TryGetParameterIndex(parameterName) : -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::ClearBindings()
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call ClearBindings on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return stat;

    return GetPreparedStatementP()->ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

            case ECSqlType::Pragma:
                return GetPreparedStatementP<PragmaECSqlPreparedStatement>()->Step();

            default:
                BeAssert(false && "Unhandled ECSqlType in ECSqlStatement::Step.");
                return BE_SQLITE_ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ECSqlStatement::Impl::Step(ECInstanceKey& ecInstanceKey)
    {
    if (!FailIfWrongType(ECSqlType::Insert, "Only call Step(ECInstanceKey&) on an ECSQL INSERT statement.").IsSuccess())
        return BE_SQLITE_ERROR;

    return GetPreparedStatementP<ECSqlInsertPreparedStatement>()->Step(ecInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::Reset()
    {
    if(IsPrepared() && ECSqlType::Pragma == GetPreparedStatementP()->GetType()) {
        return GetPreparedStatementP<PragmaECSqlPreparedStatement>()->Reset();
    }

    ECSqlStatus stat = FailIfNotPrepared("Cannot call Reset on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return stat;

    return GetPreparedStatementP()->Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ECSqlStatement::Impl::GetColumnCount() const
    {
    if(IsPrepared() && ECSqlType::Pragma == GetPreparedStatementP()->GetType()) {
        return GetPreparedStatementP<PragmaECSqlPreparedStatement>()->GetColumnCount();
    }

    if (FailIfWrongType(ECSqlType::Select, "Cannot call query result API on an unprepared or non-SELECT ECSqlStatement.") != ECSqlStatus::Success)
        return -1;

    return GetPreparedStatementP<ECSqlSelectPreparedStatement>()->GetColumnCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlStatement::Impl::GetValue(int columnIndex) const
    {
    if(IsPrepared() && ECSqlType::Pragma == GetPreparedStatementP()->GetType()) {
        return GetPreparedStatementP<PragmaECSqlPreparedStatement>()->GetValue(columnIndex);
    }

    if (FailIfWrongType(ECSqlType::Select, "Cannot call query result API on an unprepared or non-SELECT ECSqlStatement.") != ECSqlStatus::Success)
        return NoopECSqlValue::GetSingleton();

    //Reports errors (not prepared yet, index out of bounds) and uses no-op value in case of error
    return GetPreparedStatementP<ECSqlSelectPreparedStatement>()->GetValue(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::Impl::GetECSql() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetECSql on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return GetPreparedStatementP()->GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::Impl::GetNativeSql() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetNativeSql on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return GetPreparedStatementP()->GetNativeSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECDb const* ECSqlStatement::Impl::GetECDb() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetECDb on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return &GetPreparedStatementP()->GetECDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Db const* ECSqlStatement::Impl::GetDataSourceDb() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetDataSourceDb on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return &GetPreparedStatementP()->GetDataSourceDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::FailIfNotPrepared(Utf8CP errorMessage) const {
    if (!IsPrepared()) {
        LOG.error(errorMessage);
        return ECSqlStatus::Error;
    }

return ECSqlStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Impl::FailIfWrongType(ECSqlType expectedType, Utf8CP errorMessage) const
    {
    ECSqlStatus stat = FailIfNotPrepared(errorMessage);
    if (!stat.IsSuccess())
        return stat;

    if (GetPreparedStatementP()->GetType() != expectedType)
        {
        GetECDb()->GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, errorMessage);
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlPreparedStatement& ECSqlStatement::Impl::CreatePreparedStatement(ECDbCR ecdb, Exp const& exp)
    {
    switch (exp.GetType())
        {
            case Exp::Type::CommonTable:
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

            case Exp::Type::Pragma:
                m_preparedStatement = std::make_unique<PragmaECSqlPreparedStatement>(ecdb);
                break;

            default:
                BeAssert(false && "ECSqlParseTree is expected to only be of type Select, Insert, Update, Delete");
                break;
        }

    return *m_preparedStatement;
    }

#ifndef NDEBUG
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
NativeLogging::CategoryLogger& ECSqlStatement::Impl::GetPrepareDiagnosticsLogger()
    {
    BeAssert(s_prepareDiagnosticsLogger != nullptr);
    return *s_prepareDiagnosticsLogger;
    }
#endif

#ifndef NDEBUG
//**********************************************************
// ECSqlStatement::Impl::Diagnostics
//**********************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatement::Impl::Diagnostics::Diagnostics(Utf8CP ecsql, NativeLogging::CategoryLogger& logger, bool startTimer)
    : m_logger(logger), m_timer(nullptr), m_ecsql(ecsql)
    {
    if (startTimer && CanLog())
        m_timer = std::unique_ptr<StopWatch>(new StopWatch(true));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlStatement::Impl::Diagnostics::CanLog() const
    {
    return m_logger.isSeverityEnabled(LOG_SEVERITY);
    }
#endif
END_BENTLEY_SQLITE_EC_NAMESPACE
