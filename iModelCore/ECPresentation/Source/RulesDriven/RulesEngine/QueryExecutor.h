/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryExecutor.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <Logging/bentleylogging.h>
#include "NavigationQuery.h"
#include "JsonNavNode.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

typedef RefCountedPtr<struct NavNodeReader> NavNodeReaderPtr;
/*=================================================================================**//**
* Responsible for creating a node from a ECSql statement.
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct NavNodeReader : RefCountedBase
{
private:
    IConnectionCP m_connection;
    JsonNavNodesFactory const& m_factory;
    NavigationQueryContract const& m_contract;

protected:
    NavNodeReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract) : m_factory(factory), m_contract(contract) {}
    IConnectionCR GetConnection() const {return *m_connection;}
    JsonNavNodesFactory const& GetFactory() const {return m_factory;}
    NavigationQueryContract const& GetContract() const {return m_contract;}
    virtual JsonNavNodePtr _ReadNode(ECSqlStatementCR statement) const = 0;

public:
    static NavNodeReaderPtr Create(JsonNavNodesFactory const&, IConnectionCR, NavigationQueryContract const&, NavigationQueryResultType resultType);
    virtual ~NavNodeReader() {}
    JsonNavNodePtr ReadNode(ECSqlStatementCR statement) const {return _ReadNode(statement);}
};

/*=================================================================================**//**
* Responsible for stepping through queries.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct QueryExecutor : NonCopyableClass
{
private:
    IConnectionCR m_connection;
    ECSqlStatementCache const& m_statementCache;
    PresentationQueryBase const* m_query;
    mutable Utf8String m_queryString;
    bool m_readStarted;
    bool m_readFinished;

private:
    Utf8CP GetQueryString() const;
    CachedECSqlStatementPtr GetStatement() const;

protected:
    IConnectionCR GetConnection() const {return m_connection;}
    ECDbR GetDb() const {return m_connection.GetDb();}

protected:
    virtual void _Reset();
    virtual void _ReadRecord(ECSqlStatement&) = 0;

public:
    ECPRESENTATION_EXPORT QueryExecutor(IConnectionCR, ECSqlStatementCache const&);
    ECPRESENTATION_EXPORT QueryExecutor(IConnectionCR, ECSqlStatementCache const&, PresentationQueryBase const& query);
    ECPRESENTATION_EXPORT void SetQuery(PresentationQueryBase const& query);
    ECPRESENTATION_EXPORT PresentationQueryBase const* GetQuery() const;
    ECPRESENTATION_EXPORT void ReadRecords(ICancelationTokenCP = nullptr);
    bool IsReadFinished() const {return m_readFinished;}
    bool IsReadStarted() const {return m_readStarted;}
    void Reset() {_Reset();}
};

/*=================================================================================**//**
* Responsible for stepping through queries and creating nodes for their results.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE NavigationQueryExecutor : QueryExecutor
{    
private:
    NavigationQueryCP m_query;
    JsonNavNodesFactory const& m_nodesFactory;
    NavNodeReaderPtr m_reader;
    mutable bvector<JsonNavNodePtr> m_nodes;
    
protected:
    void _Reset() override;
    void _ReadRecord(ECSqlStatement&) override;

public:
    ECPRESENTATION_EXPORT NavigationQueryExecutor(JsonNavNodesFactory const&, IConnectionCR, ECSqlStatementCache const&, NavigationQuery const&);
    ECPRESENTATION_EXPORT NavigationQueryExecutor(JsonNavNodesFactory const&, IConnectionCR, ECSqlStatementCache const&);
    ECPRESENTATION_EXPORT void SetQuery(NavigationQuery const& query, bool clearCache = true);
    ECPRESENTATION_EXPORT JsonNavNodePtr GetNode(size_t index) const;
    ECPRESENTATION_EXPORT size_t GetNodesCount() const;
};

/*=================================================================================**//**
* Responsible for stepping through queries and content set items for their results.
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ContentQueryExecutor : QueryExecutor
{    
private:
    ContentQueryCPtr m_query;
    mutable bvector<ContentSetItemPtr> m_records;
    IECPropertyFormatter const* m_propertyFormatter;
    
protected:
    void _Reset() override;
    void _ReadRecord(ECSqlStatement&) override;

public:
    ECPRESENTATION_EXPORT ContentQueryExecutor(IConnectionCR, ECSqlStatementCache const&, ContentQuery const&);
    ECPRESENTATION_EXPORT ContentQueryExecutor(IConnectionCR, ECSqlStatementCache const&);
    void SetPropertyFormatter(IECPropertyFormatter const& formatter) {m_propertyFormatter = &formatter;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}
    ECPRESENTATION_EXPORT void SetQuery(ContentQuery const& query, bool clearCache = true);
    ContentQueryCP GetContentQuery() const {return m_query.get();}
    ECPRESENTATION_EXPORT ContentSetItemPtr GetRecord(size_t index) const;
    ECPRESENTATION_EXPORT size_t GetRecordsCount() const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE CountQueryExecutor : QueryExecutor
{
private:
    size_t m_result;

protected:
    void _ReadRecord(ECSqlStatement&) override;

public:
    CountQueryExecutor(IConnectionCR connection, ECSqlStatementCache const& statementCache, PresentationQueryBase const& query)
        : QueryExecutor(connection, statementCache, query), m_result(0)
        {}
    CountQueryExecutor(IConnectionCR connection, ECSqlStatementCache const& statementCache)
        : QueryExecutor(connection, statementCache), m_result(0)
        {}
    ECPRESENTATION_EXPORT size_t GetResult() const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
