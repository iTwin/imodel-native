/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentProviderTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include "../../../Source/RulesDriven/RulesEngine/ContentProviders.h"
#include "ECDbTestProject.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct ContentProviderTests : public ::testing::Test
    {
    static ECDbTestProject* s_project;
    static CustomFunctionsInjector* s_customFunctions;
    ECSqlStatementCache m_statementCache;
    PresentationRuleSetPtr m_ruleset;
    JsonNavNodesFactory m_nodesFactory;
    ContentProviderContextPtr m_context;
    TestUserSettings m_settings;
    RuleSetLocaterManager m_locaterManager;
    TestNodeLocater m_nodesLocater;
    DefaultCategorySupplier m_categorySupplier;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;

    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;
    
    ContentProviderTests() : m_statementCache(5) {}

    public: static void SetUpTestCase();
    public: static void TearDownTestCase();
    
    void SetUp() override;
    void TearDown() override;
    };
