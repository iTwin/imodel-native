/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

    uint64_t m_providerIndex;
    
    NodesProviderTests() 
        : m_providerContextFactory(m_connections), m_nodesCache(m_connections, &m_providerContextFactory),
        m_providerIndex(0)
        {}

    static void SetUpTestCase();
    static void TearDownTestCase();
    
    virtual void SetUp() override;
    virtual void TearDown() override;
    
    void Cache(JsonNavNodeR node);
    };
