/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/MultiSpecificationNodesProviderTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentationRules/PresentationRules.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodeProviders.h"
#include "../../../Source/RulesDriven/RulesEngine/NavNodesCache.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct MultiSpecificationNodesProviderTests : ::testing::Test
    {
    static ECDbTestProject* s_project;
    static CustomFunctionsInjector* s_customFunctions;
    
    ECSqlStatementCache m_statementCache;
    RelatedPathsCache m_relatedPathsCache;
    ECExpressionsCache m_expressionsCache;
    PresentationRuleSetPtr m_ruleset;
    NavNodesProviderContextPtr m_context;
    TestNodesProviderFactory m_providerFactory;
    TestUserSettings m_settings;
    JsonNavNodesFactory m_nodesFactory;
    TestNodesCache m_nodesCache;
    
    MultiSpecificationNodesProviderTests() : m_statementCache(5) {}
    
    static void SetUpTestCase()
        {
        Localization::Init();
        s_project = new ECDbTestProject();
        s_project->Create("MultiSpecificationNodesProviderTests", "RulesEngineTest.01.00.ecschema.xml");
        s_customFunctions = new CustomFunctionsInjector(s_project->GetECDb());
        }
    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        DELETE_AND_CLEAR(s_customFunctions);
        Localization::Terminate();
        }

    void SetUp() override
        {
        m_ruleset = PresentationRuleSet::CreateInstance("CustomNodesProviderTests", 1, 0, false, "", "", "", false);
        m_context = NavNodesProviderContext::Create(*m_ruleset, true, TargetTree_Both, 0, 
            m_settings, m_expressionsCache, m_relatedPathsCache, m_nodesFactory, m_nodesCache, m_providerFactory, nullptr);
        m_context->SetQueryContext(s_project->GetECDb(), m_statementCache, *s_customFunctions, nullptr);
        }
    };
ECDbTestProject* MultiSpecificationNodesProviderTests::s_project = nullptr;
CustomFunctionsInjector* MultiSpecificationNodesProviderTests::s_customFunctions = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSpecificationNodesProviderTests, ReturnsNodesForAllSpecifications)
    {
    RootNodeRule rule;
    CustomNodeSpecification spec1(1, false, "type", "label1", "", "imageId");
    CustomNodeSpecification spec2(1, false, "type", "label2", "", "imageId");

    RootNodeRuleSpecificationsList specs;
    specs.push_back(RootNodeRuleSpecification(spec1, rule));
    specs.push_back(RootNodeRuleSpecification(spec2, rule));

    JsonNavNodePtr node;
    NavNodesProviderPtr provider = MultiSpecificationNodesProvider::Create(*m_context, specs);

    EXPECT_TRUE(provider->GetNode(node, 0));
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("label1", node->GetLabel().c_str());

    EXPECT_TRUE(provider->GetNode(node, 1));
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("label2", node->GetLabel().c_str());

    EXPECT_FALSE(provider->GetNode(node, 2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSpecificationNodesProviderTests, SkipsEmptySpecifications)
    {
    BeTest::SetFailOnAssert(false);

    RootNodeRule rule;
    CustomNodeSpecification spec1(1, false, "type", "label1", "", "imageId");
    CustomNodeSpecification spec2(1, false, "", "label2", "", ""); // invalid
    CustomNodeSpecification spec3(1, false, "type", "label3", "", "imageId");

    RootNodeRuleSpecificationsList specs;
    specs.push_back(RootNodeRuleSpecification(spec1, rule));
    specs.push_back(RootNodeRuleSpecification(spec2, rule));
    specs.push_back(RootNodeRuleSpecification(spec3, rule));

    JsonNavNodePtr node;
    NavNodesProviderPtr provider = MultiSpecificationNodesProvider::Create(*m_context, specs);

    EXPECT_TRUE(provider->GetNode(node, 0));
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("label1", node->GetLabel().c_str());

    EXPECT_TRUE(provider->GetNode(node, 1));
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("label3", node->GetLabel().c_str());

    EXPECT_FALSE(provider->GetNode(node, 2));
    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSpecificationNodesProviderTests, HasNodesReturnsFalseWhenNoSpecificationsReturnNodes)
    {
    BeTest::SetFailOnAssert(false);

    RootNodeRule rule;
    CustomNodeSpecification spec1(1, false, "", "label1", "", ""); // invalid
    CustomNodeSpecification spec2(1, false, "", "label2", "", ""); // invalid

    RootNodeRuleSpecificationsList specs;
    specs.push_back(RootNodeRuleSpecification(spec1, rule));
    specs.push_back(RootNodeRuleSpecification(spec2, rule));

    NavNodesProviderPtr provider = MultiSpecificationNodesProvider::Create(*m_context, specs);
    EXPECT_FALSE(provider->HasNodes());

    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSpecificationNodesProviderTests, HasNodesReturnsTrueWhenAtLeastOneSpecificationReturnsNodes)
    {
    BeTest::SetFailOnAssert(false);

    RootNodeRule rule;
    CustomNodeSpecification spec1(1, false, "", "label1", "", ""); // invalid
    CustomNodeSpecification spec2(1, false, "type2", "label2", "description2", "imageId2");

    RootNodeRuleSpecificationsList specs;
    specs.push_back(RootNodeRuleSpecification(spec1, rule));
    specs.push_back(RootNodeRuleSpecification(spec2, rule));

    NavNodesProviderPtr provider = MultiSpecificationNodesProvider::Create(*m_context, specs);
    EXPECT_TRUE(provider->HasNodes());

    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSpecificationNodesProviderTests, CountsNodes)
    {
    BeTest::SetFailOnAssert(false);

    RootNodeRule rule;
    CustomNodeSpecification spec1(1, false, "type", "label1", "", "imageId");
    CustomNodeSpecification spec2(1, false, "", "label2", "", ""); // invalid
    CustomNodeSpecification spec3(1, false, "type", "label3", "", "imageId");

    RootNodeRuleSpecificationsList specs;
    specs.push_back(RootNodeRuleSpecification(spec1, rule));
    specs.push_back(RootNodeRuleSpecification(spec2, rule));
    specs.push_back(RootNodeRuleSpecification(spec3, rule));

    JsonNavNodePtr node;
    NavNodesProviderPtr provider = MultiSpecificationNodesProvider::Create(*m_context, specs);
    EXPECT_EQ(2, provider->GetNodesCount());
    for (size_t i = 0; i < 3; i++)
        provider->GetNode(node, i);
    EXPECT_EQ(2, provider->GetNodesCount());

    BeTest::SetFailOnAssert(true);
    }
