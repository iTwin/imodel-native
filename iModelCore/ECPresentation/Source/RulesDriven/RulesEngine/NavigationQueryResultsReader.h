/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "QueryExecutor.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* Responsible for creating a nodes from a ECSql statement.
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct NavNodesReader : IQueryResultReader<JsonNavNodePtr>
{
private:
    IConnectionCP m_connection;
    Utf8String m_locale;
    JsonNavNodesFactory const& m_factory;
    NavigationQueryContract const& m_contract;
    NavNodeExtendedData const* m_navNodeExtendedData;

protected:
    NavNodesReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract)
        : m_factory(factory), m_contract(contract), m_connection(nullptr) {}
    IConnectionCR GetConnection() const {return *m_connection;}
    Utf8StringCR GetLocale() const {return m_locale;}
    JsonNavNodesFactory const& GetFactory() const {return m_factory;}
    NavigationQueryContract const& GetContract() const {return m_contract;}
    virtual JsonNavNodePtr _ReadNode(ECSqlStatementCR) const = 0;
    QueryResultReaderStatus _ReadRecord(JsonNavNodePtr&, ECSqlStatementCR) override;

public:
    ECPRESENTATION_EXPORT static std::unique_ptr<NavNodesReader> Create(JsonNavNodesFactory const&, IConnectionCR, Utf8String, NavigationQueryContract const&, NavigationQueryResultType, NavNodeExtendedData const*);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
