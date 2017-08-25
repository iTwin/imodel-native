/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/QueryBasedNodesProviderTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct QueryBasedNodesProviderTests : ::testing::Test
    {
    static ECDbTestProject* s_project;
    static CustomFunctionsInjector* s_customFunctions;
    ECSqlStatementCache m_statementCache;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;
    PresentationRuleSetPtr m_ruleset;
    NavNodesProviderContextPtr m_context;
    TestNodesProviderFactory m_providerFactory;
    TestUserSettings m_settings;
    JsonNavNodesFactory m_nodesFactory;
    TestNodesCache m_nodesCache;
    
    
    ECEntityClassCP m_widgetClass;
    ECEntityClassCP m_gadgetClass;
    ECEntityClassCP m_sprocketClass;

    QueryBasedNodesProviderTests() : m_statementCache(5) {}

    static void SetUpTestCase();
    static void TearDownTestCase();
    
    void SetUp() override;
    };
