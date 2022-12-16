/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/Rules/PresentationRules.h>
#include <Bentley/Logging.h>
#include "PresentationQuery.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class QueryResultReaderStatus
    {
    Row,            //!< Row successfully read
    RowAndRepeat,   //!< Row successfully read
    Skip,           //!< Row read, but reader wants to skip it
    Stop,           //!< Row read, but reader wants to stop reading
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TResult>
struct IQueryResultReader
{
protected:
    virtual QueryResultReaderStatus _ReadRecord(TResult&, ECSqlStatementCR) = 0;
    virtual bool _Complete(TResult&, ECSqlStatementCR) {return false;}
public:
    virtual ~IQueryResultReader() {}
    QueryResultReaderStatus ReadRecord(TResult& result, ECSqlStatementCR stmt) {return _ReadRecord(result, stmt);}
    bool Complete(TResult& result, ECSqlStatementCR stmt) {return _Complete(result, stmt);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TResult>
struct GenericQueryResultReader : IQueryResultReader<TResult>
{
private:
    std::function<QueryResultReaderStatus(TResult&, ECSqlStatementCR)> m_func;
protected:
    QueryResultReaderStatus _ReadRecord(TResult& result, ECSqlStatementCR stmt) override {return m_func(result, stmt);}
public:
    GenericQueryResultReader(std::function<QueryResultReaderStatus(TResult&, ECSqlStatementCR)> func) : m_func(func) {}
    GenericQueryResultReader(std::function<TResult(ECSqlStatementCR)> func)
        {
        m_func = [func](TResult& result, ECSqlStatementCR stmt)
            {
            result = func(stmt);
            return QueryResultReaderStatus::Row;
            };
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class QueryExecutorStatus
    {
    Row,       //!< Row successfully read
    Done,      //!< No more results
    };

/*=================================================================================**//**
* Responsible for stepping through queries.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryExecutor : NonCopyableClass
{
private:
    IConnectionCR m_connection;
    PresentationQuery const* m_query;
    CachedECSqlStatementPtr m_statement;
    bool m_step;
    bool m_readStarted;
    bool m_readFinished;

private:
    ECPRESENTATION_EXPORT ECSqlStatement& GetStatement();
    ECPRESENTATION_EXPORT DbResult Step(ECSqlStatement&);

protected:
    IConnectionCR GetConnection() const {return m_connection;}

public:
    ECPRESENTATION_EXPORT QueryExecutor(IConnectionCR);
    ECPRESENTATION_EXPORT QueryExecutor(IConnectionCR, PresentationQuery const& query);
    ECPRESENTATION_EXPORT void SetQuery(PresentationQuery const& query);
    ECPRESENTATION_EXPORT PresentationQuery const* GetQuery() const;
    bool IsReadFinished() const { return m_readFinished; }
    bool IsReadStarted() const { return m_readStarted; }
    ECPRESENTATION_EXPORT void Reset();
    template<typename TResult> QueryExecutorStatus ReadNext(TResult& result, IQueryResultReader<TResult>& reader)
        {
        auto scope = Diagnostics::Scope::Create("QueryExecutor: Read next");
        result = TResult();

        if (m_readFinished)
            return QueryExecutorStatus::Done;

        ECSqlStatement& stmt = GetStatement();
        while (!m_step || BE_SQLITE_ROW == Step(stmt))
            {
            m_step = true;
            switch (reader.ReadRecord(result, stmt))
                {
                case QueryResultReaderStatus::Stop:
                    m_readFinished = true;
                    return QueryExecutorStatus::Done;
                case QueryResultReaderStatus::RowAndRepeat:
                    m_step = false;
                    return QueryExecutorStatus::Row;
                case QueryResultReaderStatus::Row:
                    return QueryExecutorStatus::Row;
                }
            }
        m_readFinished = true;
        if (reader.Complete(result, stmt))
            return QueryExecutorStatus::Row;
        return QueryExecutorStatus::Done;
        }
    template<typename TResult> TResult ReadNext(IQueryResultReader<TResult>& reader)
        {
        TResult result = TResult();
        if (QueryExecutorStatus::Row == ReadNext(result, reader))
            return result;
        return TResult();
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class QueryResultAccumulatorStatus
    {
    Continue, //!< Continue serving rows
    Stop,     //!< Skip remaining rows
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IQueryResultAccumulator
{
protected:
    virtual QueryResultAccumulatorStatus _ReadRow(ECSqlStatementCR) = 0;
    virtual void _Complete(ECSqlStatementCR, QueryResultAccumulatorStatus) {}

public:
    virtual ~IQueryResultAccumulator() = default;
    QueryResultAccumulatorStatus ReadRow(ECSqlStatementCR statement) {return _ReadRow(statement);}
    void Complete(ECSqlStatementCR statement, QueryResultAccumulatorStatus rowStatus) {_Complete(statement, rowStatus);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryExecutorHelper
{
private:
    QueryExecutorHelper() {}
public:
    ECPRESENTATION_EXPORT static uint64_t ReadUInt64(IConnectionCR, PresentationQueryCR);
    ECPRESENTATION_EXPORT static Utf8String ReadText(IConnectionCR, PresentationQueryCR);

    ECPRESENTATION_EXPORT static DbResult Step(ECSqlStatement&);

    //! Executes query and forwards each result row to accumulator.
    //! @returns QueryExecutorStatus::Done if operation is successful.
    ECPRESENTATION_EXPORT static QueryExecutorStatus ExecuteQuery(IConnectionCR, PresentationQuery const&,
        IQueryResultAccumulator&, ICancelationTokenCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
