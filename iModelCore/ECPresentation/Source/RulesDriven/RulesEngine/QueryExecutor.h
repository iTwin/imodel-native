/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <Logging/bentleylogging.h>
#include "NavigationQuery.h"
#include "JsonNavNode.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
enum class QueryResultReaderStatus
    {
    Error,          //!< Error occured, stop reading
    Row,            //!< Row sucessfully read
    RowAndRepeat,   //!< Row sucessfully read
    Skip,           //!< Row read, but reader wants to skip it
    Stop,           //!< Row read, but reader wants to stop reading
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
template<typename TResult>
struct IQueryResultReader
{
protected:
    virtual QueryResultReaderStatus _ReadRecord(TResult&, ECSqlStatementCR) = 0;
public:
    virtual ~IQueryResultReader() {}
    QueryResultReaderStatus ReadRecord(TResult& result, ECSqlStatementCR stmt) {return _ReadRecord(result, stmt);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
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
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
enum class QueryExecutorStatus
    {
    Error,  //!< Error executing query or reading results
    Row,    //!< Row successfully read
    Done,   //!< No more results
    };

/*=================================================================================**//**
* Responsible for stepping through queries.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct QueryExecutor : NonCopyableClass
{
private:
    IConnectionCR m_connection;
    PresentationQueryBase const* m_query;
    mutable Utf8String m_queryString;
    CachedECSqlStatementPtr m_statement;
    bool m_step;
    bool m_readStarted;
    bool m_readFinished;

private:
    Utf8CP GetQueryString() const;
    ECPRESENTATION_EXPORT ECSqlStatement* GetStatement();
    ECPRESENTATION_EXPORT DbResult Step(ECSqlStatement&);

protected:
    IConnectionCR GetConnection() const {return m_connection;}

public:
    ECPRESENTATION_EXPORT QueryExecutor(IConnectionCR);
    ECPRESENTATION_EXPORT QueryExecutor(IConnectionCR, PresentationQueryBase const& query);
    ECPRESENTATION_EXPORT void SetQuery(PresentationQueryBase const& query);
    ECPRESENTATION_EXPORT PresentationQueryBase const* GetQuery() const;
    bool IsReadFinished() const { return m_readFinished; }
    bool IsReadStarted() const { return m_readStarted; }
    ECPRESENTATION_EXPORT void Reset();
    template<typename TResult> TResult ReadNext(IQueryResultReader<TResult>& reader)
        {
        ECSqlStatement* stmt = GetStatement();
        if (!stmt)
            return TResult();

        while (!m_step || BE_SQLITE_ROW == Step(*stmt))
            {
            m_step = true;
            TResult result;
            switch (reader.ReadRecord(result, *stmt))
                {
                case QueryResultReaderStatus::Error:
                case QueryResultReaderStatus::Stop:
                    m_readFinished = true;
                    return TResult();
                case QueryResultReaderStatus::RowAndRepeat:
                    m_step = false;
                    return result;
                case QueryResultReaderStatus::Row:
                    return result;
                }
            }
        return TResult();
        }
    template<typename TResult> QueryExecutorStatus ReadNext(TResult& result, IQueryResultReader<TResult>& reader)
        {
        result = TResult();
        ECSqlStatement* stmt = GetStatement();
        if (!stmt)
            return QueryExecutorStatus::Error;

        while (!m_step || BE_SQLITE_ROW == Step(*stmt))
            {
            m_step = true;
            switch (reader.ReadRecord(result, *stmt))
                {
                case QueryResultReaderStatus::Error:
                    m_readFinished = true;
                    return QueryExecutorStatus::Error;
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
        return QueryExecutorStatus::Done;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
struct QueryExecutorHelper
{
private:
    QueryExecutorHelper() {}
public:
    ECPRESENTATION_EXPORT static uint64_t ReadUInt64(IConnectionCR, GenericQueryCR);
    ECPRESENTATION_EXPORT static Utf8String ReadText(IConnectionCR, GenericQueryCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
