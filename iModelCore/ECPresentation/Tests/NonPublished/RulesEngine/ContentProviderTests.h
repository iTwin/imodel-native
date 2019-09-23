/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct ContentProviderTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    CustomFunctionsInjector* m_customFunctions;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    PresentationRuleSetPtr m_ruleset;
    JsonNavNodesFactory m_nodesFactory;
    ContentProviderContextPtr m_context;
    TestUserSettings m_settings;
    RuleSetLocaterManager m_locaterManager;
    TestNodeLocater m_nodesLocater;
    DefaultCategorySupplier m_categorySupplier;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;
    PolymorphicallyRelatedClassesCache m_polymorphicallyRelatedClassesCache;

    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;
    
    ContentProviderTests() 
        : m_locaterManager(m_connections)
        {}

    static void SetUpTestCase();
    static void TearDownTestCase();
    
    void SetUp() override;
    void TearDown() override;
    };
