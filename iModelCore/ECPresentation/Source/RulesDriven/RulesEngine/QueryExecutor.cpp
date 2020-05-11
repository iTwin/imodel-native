/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include <ECPresentation/LabelDefinition.h>
#include "QueryExecutor.h"
#include "QueryContracts.h"
#include "ExtendedData.h"
#include "JsonNavNode.h"
#include "LoggingHelper.h"

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryExecutor::QueryExecutor(IConnectionCR connection)
    : m_connection(connection), m_query(nullptr), m_readStarted(false), m_readFinished(false), m_step(true)
    {}

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryExecutor::QueryExecutor(IConnectionCR connection, PresentationQueryBase const& query)
    : m_connection(connection), m_query(&query), m_readStarted(false), m_readFinished(false), m_step(true)
    {}

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutor::SetQuery(PresentationQueryBase const& query)
    {
    if (m_query == &query)
        return;

    m_query = &query;
    m_queryString.clear();
    m_statement = nullptr;
    m_readStarted = false;
    m_readFinished = false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryBase const* QueryExecutor::GetQuery() const {return m_query;}

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP QueryExecutor::GetQueryString() const
    {
    if (m_queryString.empty())
        m_queryString = m_query->ToString();
    return m_queryString.c_str();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutor::Reset()
    {
    m_readStarted = false;
    m_readFinished = false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatement* QueryExecutor::GetStatement()
    {
    if (nullptr == m_query)
        return nullptr;

    if (m_statement.IsNull())
        {
        Savepoint txn(m_connection.GetDb(), "QueryExecutor::ReadRecords");
        BeAssert(txn.IsActive());

        Utf8CP query = GetQueryString();
        m_statement = m_connection.GetStatementCache().GetPreparedStatement(GetConnection().GetECDb().Schemas(), GetConnection().GetDb(), query);
        if (m_statement.IsNull())
            {
            LoggingHelper::LogMessage(Log::Default, Utf8PrintfString("[QueryExecutor] Failed to prepare query (%s): '%s'",
                m_connection.GetDb().GetLastError().c_str(), m_queryString.c_str()).c_str(), NativeLogging::LOG_ERROR);
            BeAssert(false);
            return nullptr;
            }

        m_query->BindValues(*m_statement);
        }
    return m_statement.get();
    }

//#define RULES_ENGINE_MEASURE_QUERY_PERFORMANCE 1
/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult QueryExecutor::Step(ECSqlStatement& statement)
    {
#ifdef RULES_ENGINE_MEASURE_QUERY_PERFORMANCE
    bool isFirstStep = !m_readStarted;
    uint64_t startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
#endif
    m_readStarted = true;
    DbResult result = statement.Step();
#ifdef RULES_ENGINE_MEASURE_QUERY_PERFORMANCE
    if (isFirstStep)
        {
        uint64_t elapsedTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() - startTime;
        LoggingHelper::LogMessage(Log::Default, Utf8PrintfString("[QueryExecutor] First step took %" PRIu64 " ms", elapsedTime).c_str(), NativeLogging::LOG_ERROR);
        Utf8String plan = m_connection.GetDb().ExplainQuery(statement->GetNativeSql());
        LoggingHelper::LogMessage(Log::Default, Utf8PrintfString("[QueryExecutor] Query plan: %s", plan.c_str()).c_str(), NativeLogging::LOG_ERROR);
        }
#endif
    if (BE_SQLITE_DONE == result)
        m_readFinished = true;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2020
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
// @bsimethod                                    Grigas.Petraitis                04/2020
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