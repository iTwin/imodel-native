/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NodesProviderTests.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodeProviders.h"
#include "../../../Source/RulesDriven/RulesEngine/NavNodesCache.h"
#include "ECDbTestProject.h"
#include "TestNavNode.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct NodesProviderTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    IConnectionPtr m_connection;
    ECSqlStatementCache m_statementCache;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;
    PresentationRuleSetPtr m_ruleset;
    TestConnectionManager m_connections;
    NavNodesProviderContextPtr m_context;
    TestNodesProviderContextFactory m_providerContextFactory;
    TestNodesProviderFactory m_providerFactory;
    TestUserSettings m_settings;
    TestNodesCache m_nodesCache;
    TestECDbUsedClassesListener m_usedClassesListener;
    
    NodesProviderTests() 
        : m_statementCache(10), m_providerContextFactory(m_connections), m_nodesCache(m_connections, &m_providerContextFactory)
        {}

    static void SetUpTestCase();
    static void TearDownTestCase();
    
    virtual void SetUp() override;
    virtual void TearDown() override;
    };
