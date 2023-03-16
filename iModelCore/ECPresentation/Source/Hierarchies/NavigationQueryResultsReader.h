/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Shared/Queries/QueryExecutor.h"
#include "NavigationQuery.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* Responsible for creating a nodes from a ECSql statement.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodesReader : IQueryResultReader<NavNodePtr>
{
private:
    IConnectionCR m_connection;
    NavNodesFactory const& m_factory;
    IContractProvider<NavigationQueryContract> const& m_contractProvider;
    NavNodeExtendedData const& m_navNodeExtendedData;
    NavNodeKeyCP m_parentKey;

protected:
    NavNodesReader(NavNodesFactory const& factory, IContractProvider<NavigationQueryContract> const& contractProvider, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : m_factory(factory), m_contractProvider(contractProvider), m_connection(connection), m_navNodeExtendedData(extendedData), m_parentKey(parentKey)
        {}
    IConnectionCR GetConnection() const {return m_connection;}
    NavNodesFactory const& GetFactory() const {return m_factory;}
    NavigationQueryContract const* GetContract(uint64_t id) const {return m_contractProvider.GetContract(id);}
    NavNodeExtendedData const& GetNavNodeExtendedData() const {return m_navNodeExtendedData;}
    NavNodeKeyCP GetParentKey() const {return m_parentKey;}
    void InitNode(NavNodeR) const;
    virtual NavNodePtr _ReadNode(ECSqlStatementCR) const = 0;
    virtual QueryResultReaderStatus _ReadRecord(NavNodePtr&, ECSqlStatementCR) override;

public:
    ECPRESENTATION_EXPORT static std::unique_ptr<IQueryResultReader<NavNodePtr>> Create(NavNodesFactory const&, IConnectionCR,
        IContractProvider<NavigationQueryContract> const&, NavigationQueryResultType, NavNodeExtendedData const&, NavNodeKeyCP);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
