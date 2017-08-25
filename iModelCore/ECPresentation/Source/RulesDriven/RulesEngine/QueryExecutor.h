/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryExecutor.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentationRules/PresentationRules.h>
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
    BeSQLite::EC::ECDb const* m_ecdb;
    JsonNavNodesFactory const& m_factory;
    NavigationQueryContract const& m_contract;

protected:
    NavNodeReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract) : m_factory(factory), m_contract(contract) {}
    BeSQLite::EC::ECDbCR GetECDb() const {return *m_ecdb;}
    JsonNavNodesFactory const& GetFactory() const {return m_factory;}
    NavigationQueryContract const& GetContract() const {return m_contract;}
    virtual JsonNavNodePtr _ReadNode(BeSQLite::EC::ECSqlStatementCR statement) const = 0;

public:
    static NavNodeReaderPtr Create(JsonNavNodesFactory const&, BeSQLite::EC::ECDbCR ecdb, NavigationQueryContract const&, NavigationQueryResultType resultType);
    virtual ~NavNodeReader() {}
    JsonNavNodePtr ReadNode(BeSQLite::EC::ECSqlStatementCR statement) const {return _ReadNode(statement);}
};

/*=================================================================================**//**
* Responsible for stepping through queries.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct QueryExecutor : NonCopyableClass
{
private:
    BeSQLite::EC::ECDbR m_db;
    BeSQLite::EC::ECSqlStatementCache const& m_statementCache;
    PresentationQueryBase const* m_query;
    mutable Utf8String m_queryString;
    mutable bool m_readStarted;
    mutable bool m_readFinished;

private:
    Utf8CP GetQueryString() const;
    BeSQLite::EC::CachedECSqlStatementPtr GetStatement() const;

protected:
    BeSQLite::EC::ECDbR GetDb() const {return m_db;}
    void ReadRecords() const;
    void Reset();

protected:
    virtual void _ReadRecord(BeSQLite::EC::ECSqlStatement&) = 0;

public:
    ECPRESENTATION_EXPORT QueryExecutor(BeSQLite::EC::ECDbR db, BeSQLite::EC::ECSqlStatementCache const&);
    ECPRESENTATION_EXPORT QueryExecutor(BeSQLite::EC::ECDbR db, BeSQLite::EC::ECSqlStatementCache const&, PresentationQueryBase const& query);
    ECPRESENTATION_EXPORT void SetQuery(PresentationQueryBase const& query);
    ECPRESENTATION_EXPORT PresentationQueryBase const* GetQuery() const;
    bool IsReadFinished() const {return m_readFinished;}
    bool IsReadStarted() const {return m_readStarted;}
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
    void _ReadRecord(BeSQLite::EC::ECSqlStatement&) override;

public:
    ECPRESENTATION_EXPORT NavigationQueryExecutor(JsonNavNodesFactory const&, BeSQLite::EC::ECDbR, BeSQLite::EC::ECSqlStatementCache const&, NavigationQuery const&);
    ECPRESENTATION_EXPORT NavigationQueryExecutor(JsonNavNodesFactory const&, BeSQLite::EC::ECDbR, BeSQLite::EC::ECSqlStatementCache const&);
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
    void _ReadRecord(BeSQLite::EC::ECSqlStatement&) override;

public:
    ECPRESENTATION_EXPORT ContentQueryExecutor(BeSQLite::EC::ECDbR, BeSQLite::EC::ECSqlStatementCache const&, ContentQuery const&);
    ECPRESENTATION_EXPORT ContentQueryExecutor(BeSQLite::EC::ECDbR, BeSQLite::EC::ECSqlStatementCache const&);
    void SetPropertyFormatter(IECPropertyFormatter const& formatter) {m_propertyFormatter = &formatter;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}
    ECPRESENTATION_EXPORT void SetQuery(ContentQuery const& query, bool clearCache = true);
    ContentQueryCP GetContentQuery() const {return m_query.get();}
    ECPRESENTATION_EXPORT ContentSetItemPtr GetRecord(size_t index) const;
    ECPRESENTATION_EXPORT size_t GetRecordsCount() const;
    ECPRESENTATION_EXPORT void ClearCache();
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE CountQueryExecutor : QueryExecutor
{
private:
    size_t m_result;

protected:
    void _ReadRecord(BeSQLite::EC::ECSqlStatement&) override;

public:
    CountQueryExecutor(BeSQLite::EC::ECDbR db, BeSQLite::EC::ECSqlStatementCache const& statementCache, PresentationQueryBase const& query)
        : QueryExecutor(db, statementCache, query), m_result(0)
        {}
    CountQueryExecutor(BeSQLite::EC::ECDbR db, BeSQLite::EC::ECSqlStatementCache const& statementCache)
        : QueryExecutor(db, statementCache), m_result(0)
        {}
    ECPRESENTATION_EXPORT size_t GetResult() const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
