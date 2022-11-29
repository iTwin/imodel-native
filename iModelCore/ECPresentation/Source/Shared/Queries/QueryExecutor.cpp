/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include <ECPresentation/LabelDefinition.h>
#include "../ExtendedData.h"
#include "QueryExecutor.h"
#include "QueryContracts.h"

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void LogQueryPerformance(Diagnostics::Scope const& scope)
    {
    uint64_t elapsedTime = scope.GetElapsedTime();
    NativeLogging::SEVERITY severity = LOG_TRACE;
    if (elapsedTime > 60 * 1000)
        severity = LOG_ERROR;
    else if (elapsedTime > 10 * 1000)
        severity = LOG_WARNING;
    else if (elapsedTime > 1000)
        severity = LOG_INFO;
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, severity, Utf8PrintfString("First step took %" PRIu64 " ms.", elapsedTime));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryExecutor::QueryExecutor(IConnectionCR connection)
    : m_connection(connection), m_query(nullptr), m_readStarted(false), m_readFinished(false), m_step(true)
    {}

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryExecutor::QueryExecutor(IConnectionCR connection, PresentationQueryBase const& query)
    : m_connection(connection), m_query(&query), m_readStarted(false), m_readFinished(false), m_step(true)
    {}

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutor::SetQuery(PresentationQueryBase const& query)
    {
    if (m_query == &query)
        return;

    m_query = &query;
    m_statement = nullptr;
    m_readStarted = false;
    m_readFinished = false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryBase const* QueryExecutor::GetQuery() const {return m_query;}

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutor::Reset()
    {
    m_readStarted = false;
    m_readFinished = false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static CachedECSqlStatementPtr GetStatement(IConnectionCR connection, PresentationQueryBase const& query)
    {
    auto scope = Diagnostics::Scope::Create("Prepare statement");

    Savepoint txn(connection.GetDb(), "QueryExecutor::ReadRecords");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start a transaction");

    Utf8CP queryString = query.ToString().c_str();
    ECSqlStatus status;
    CachedECSqlStatementPtr statement = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), queryString, false, &status);
    if (statement.IsNull())
        {
        if (status.IsSQLiteError() && status.GetSQLiteError() == BE_SQLITE_INTERRUPT)
            {
            // this might happen due to cancellation, which is not a failure
            throw DbConnectionInterruptException();
            }

        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare query. Error: '%s'. Query: %s",
            connection.GetDb().GetLastError().c_str(), queryString));
        }

    query.BindValues(*statement);
    return statement;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatement& QueryExecutor::GetStatement()
    {
    auto scope = Diagnostics::Scope::Create("Get statement");

    if (nullptr == m_query)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Invalid query");

    if (m_statement.IsNull())
        m_statement = ::GetStatement(m_connection, *m_query);

    return *m_statement;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult QueryExecutorHelper::Step(ECSqlStatement& statement)
    {
    DbResult result = statement.Step();

    if (BE_SQLITE_BUSY == result)
        throw DbConnectionBusyException();

    if (BE_SQLITE_INTERRUPT == result)
        throw DbConnectionInterruptException();

    if (BE_SQLITE_ROW != result && BE_SQLITE_DONE != result)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Encountered unexpected db result code: %d", (int)result));

    return result;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult QueryExecutor::Step(ECSqlStatement& statement)
    {
    Diagnostics::Scope::Holder scope;
    if (!m_readStarted)
        scope = Diagnostics::Scope::Create(Utf8PrintfString("First step: `%s`", m_query->ToString().c_str()));

    m_readStarted = true;

    DbResult result = QueryExecutorHelper::Step(statement);

    if (scope)
        LogQueryPerformance(**scope);

    if (BE_SQLITE_DONE == result)
        m_readFinished = true;

    return result;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t QueryExecutorHelper::ReadUInt64(IConnectionCR connection, GenericQueryCR query)
    {
    static GenericQueryResultReader<uint64_t> s_uintReader([](ECSqlStatementCR stmt)
        {
        return stmt.GetValueUInt64(0);
        });
    QueryExecutor executor(connection, query);
    return executor.ReadNext<uint64_t>(s_uintReader);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryExecutorHelper::ReadText(IConnectionCR connection, GenericQueryCR query)
    {
    static GenericQueryResultReader<Utf8String> s_textReader([](ECSqlStatementCR stmt)
        {
        return stmt.GetValueText(0);
        });
    QueryExecutor executor(connection, query);
    return executor.ReadNext<Utf8String>(s_textReader);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryExecutorStatus QueryExecutorHelper::ExecuteQuery(IConnectionCR connection, PresentationQueryBase const& query,
    IQueryResultAccumulator& accumulator, ICancelationTokenCR cancellationToken)
    {
    CachedECSqlStatementPtr statement = GetStatement(connection, query);

    auto firstStepScope = Diagnostics::Scope::Create(Utf8PrintfString("First step: `%s`", query.ToString().c_str()));
    QueryResultAccumulatorStatus rowStatus = QueryResultAccumulatorStatus::Continue;
    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*statement))
        {
        if (firstStepScope)
            {
            LogQueryPerformance(**firstStepScope);
            firstStepScope = nullptr;
            }

        ThrowIfCancelled(cancellationToken);

        rowStatus = accumulator.ReadRow(*statement);
        switch (rowStatus)
            {
            case QueryResultAccumulatorStatus::Continue:
                continue;
            case QueryResultAccumulatorStatus::Stop:
                accumulator.Complete(*statement, rowStatus);
                return QueryExecutorStatus::Done;
            default:
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unhandled QueryResultAccumulatorStatus: %d", (int)rowStatus));
            }
        }

    accumulator.Complete(*statement, rowStatus);
    return QueryExecutorStatus::Done;
    }
