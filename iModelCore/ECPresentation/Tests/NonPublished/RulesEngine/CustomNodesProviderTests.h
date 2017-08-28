/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/CustomNodesProviderTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodeProviders.h"
#include "../../../Source/RulesDriven/RulesEngine/NavNodesCache.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct CustomNodesProviderTests : ::testing::Test, IECExpressionsCacheProvider
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

    CustomNodesProviderTests() : m_statementCache(5) {}
    
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp() override;

    ECExpressionsCache& _Get(Utf8CP) override {return m_expressionsCache;}
    };
