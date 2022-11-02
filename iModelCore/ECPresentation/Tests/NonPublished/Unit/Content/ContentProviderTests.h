/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../../../../Source/Content/ContentProviders.h"
#include "../../Helpers/TestHelpers.h"
#include "../../Helpers/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentProviderTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    CustomFunctionsInjector* m_customFunctions;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    PresentationRuleSetPtr m_ruleset;
    NavNodesFactory m_nodesFactory;
    ContentProviderContextPtr m_context;
    std::shared_ptr<RulesetVariables> m_rulesetVariables;
    RuleSetLocaterManager m_locaterManager;
    std::shared_ptr<TestNodeLocater> m_nodesLocater;
    DefaultCategorySupplier m_categorySupplier;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;

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
