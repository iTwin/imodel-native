/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/CustomNodesProviderTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "CustomNodesProviderTests.h"
#include "TestNavNode.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

ECDbTestProject* CustomNodesProviderTests::s_project = nullptr;
CustomFunctionsInjector* CustomNodesProviderTests::s_customFunctions = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodesProviderTests::SetUpTestCase()
    {
    Localization::Init();
    CustomNodesProviderTests::s_project = new ECDbTestProject();
    CustomNodesProviderTests::s_project->Create("CustomNodesProviderTests", "RulesEngineTest.01.00.ecschema.xml");
    CustomNodesProviderTests::s_customFunctions = new CustomFunctionsInjector(CustomNodesProviderTests::s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodesProviderTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(CustomNodesProviderTests::s_project);
    DELETE_AND_CLEAR(CustomNodesProviderTests::s_customFunctions);
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodesProviderTests::SetUp()
    {
    m_ruleset = PresentationRuleSet::CreateInstance("CustomNodesProviderTests", 1, 0, false, "", "", "", false);
    m_context = NavNodesProviderContext::Create(*m_ruleset, true, TargetTree_Both, 0, 
        m_settings, m_expressionsCache, m_relatedPathsCache, m_nodesFactory, m_nodesCache, m_providerFactory, nullptr);
    m_context->SetQueryContext(s_project->GetECDb(), m_statementCache, *s_customFunctions, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, InvalidSpecification_DoesntReturnNodes)
    {
    BeTest::SetFailOnAssert(false);

    RootNodeRule rule;
    m_context->SetRootNodeContext(rule);

    JsonNavNodePtr node;

    CustomNodeSpecification specNoType(1, false, "", "label", "", "imageId");
    NavNodesProviderPtr provider1 = CustomNodesProvider::Create(*m_context, specNoType);
    EXPECT_EQ(0, provider1->GetNodesCount());
    EXPECT_FALSE(provider1->GetNode(node, 0));
    EXPECT_TRUE(node.IsNull());

    CustomNodeSpecification specNoLabel(1, false, "type", "", "", "imageId");
    NavNodesProviderPtr provider2 = CustomNodesProvider::Create(*m_context, specNoLabel);
    EXPECT_EQ(0, provider2->GetNodesCount());
    EXPECT_FALSE(provider2->GetNode(node, 0));
    EXPECT_TRUE(node.IsNull());
    

    CustomNodeSpecification specNoImageId(1, false, "type", "label", "", "");
    NavNodesProviderPtr provider3 = CustomNodesProvider::Create(*m_context, specNoImageId);
    EXPECT_EQ(0, provider3->GetNodesCount());
    EXPECT_FALSE(provider3->GetNode(node, 0));
    EXPECT_TRUE(node.IsNull());

    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, ReturnsOneNode)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(rule);

    JsonNavNodePtr node;
    CustomNodeSpecification spec(1, false, "type", "label", "", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    EXPECT_TRUE(node.IsValid());
    EXPECT_FALSE(provider->GetNode(node, 1));
    EXPECT_EQ(1, provider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, ReturnsNodesAfterCount)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(rule);

    JsonNavNodePtr node;
    CustomNodeSpecification spec(1, false, "type", "label", "", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_EQ(1, provider->GetNodesCount());
    EXPECT_TRUE(provider->GetNode(node, 0));
    EXPECT_TRUE(node.IsValid());
    EXPECT_FALSE(provider->GetNode(node, 1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, ReturnsValidNode)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(rule);

    JsonNavNodePtr node;
    CustomNodeSpecification spec(1, false, "type", "label", "description", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    EXPECT_EQ(1, provider->GetNodesCount());
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("type", node->GetType().c_str());
    EXPECT_STREQ("label", node->GetLabel().c_str());
    EXPECT_STREQ("description", node->GetDescription().c_str());
    EXPECT_STREQ("imageId", node->GetExpandedImageId().c_str());
    EXPECT_STREQ("imageId", node->GetCollapsedImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, ReturnsValidChildNode)
    {
    TestNavNodePtr parent = TestNavNode::Create();
    m_nodesCache.Cache(*parent, true);

    ChildNodeRule rule;
    m_context->SetChildNodeContext(rule, *parent);

    JsonNavNodePtr node;
    CustomNodeSpecification spec(1, false, "type", "label", "description", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    EXPECT_EQ(1, provider->GetNodesCount());
    ASSERT_TRUE(node.IsValid());
    EXPECT_EQ(0, node->GetParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, OverridesLabelAndDescription)
    {
    LabelOverrideP labelOverride = new LabelOverride("ThisNode.Type=\"MyCustomNode\"", 1, "\"overriden_label\"", "\"overriden_description\"");
    m_ruleset->AddPresentationRule(*labelOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);

    CustomNodeSpecification* spec = new CustomNodeSpecification(1, false, "MyCustomNode", "label", "descr", "imageId");
    rule->GetSpecificationsR().push_back(spec);    
    m_context->SetRootNodeContext(*rule);

    JsonNavNodePtr node;
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, *spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("overriden_label", node->GetLabel().c_str());
    EXPECT_STREQ("overriden_description", node->GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, OverridesStyle)
    {
    StyleOverrideP styleOverride = new StyleOverride("ThisNode.Type=\"MyCustomNode\"", 1, 
        "IIf(True, \"ForeColor1\", \"ForeColor2\")", 
        "IIf(True, \"BackColor1\", \"BackColor2\")", 
        "IIf(True, \"FontStyle1\", \"FontStyle2\")");
    m_ruleset->AddPresentationRule(*styleOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);

    CustomNodeSpecification* spec = new CustomNodeSpecification(1, false, "MyCustomNode", "label", "", "imageId");
    rule->GetSpecificationsR().push_back(spec);    
    m_context->SetRootNodeContext(*rule);

    JsonNavNodePtr node;
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, *spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("ForeColor1", node->GetForeColor().c_str());
    EXPECT_STREQ("BackColor1", node->GetBackColor().c_str());
    EXPECT_STREQ("FontStyle1", node->GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, OverridesImageId)
    {
    ImageIdOverrideP imageOverride = new ImageIdOverride("ThisNode.Type=\"MyCustomNode\"", 1, "\"MyImageId\"");
    m_ruleset->AddPresentationRule(*imageOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);

    CustomNodeSpecification* spec = new CustomNodeSpecification(1, false, "MyCustomNode", "label", "", "imageId");
    rule->GetSpecificationsR().push_back(spec);    
    m_context->SetRootNodeContext(*rule);

    JsonNavNodePtr node;
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, *spec);
    EXPECT_TRUE(provider->GetNode(node, 0));
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("MyImageId", node->GetExpandedImageId().c_str());
    EXPECT_STREQ("MyImageId", node->GetCollapsedImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, HasNodesReturnsFalseWhenSpecificationIsInvalid)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(rule);

    BeTest::SetFailOnAssert(false);
    CustomNodeSpecification specNoType(1, false, "", "label", "", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, specNoType);
    EXPECT_FALSE(provider->HasNodes());
    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, HasNodesReturnsTrueWhenSpecificationIsValid)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(rule);

    CustomNodeSpecification spec(1, false, "type", "label", "description", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_TRUE(provider->HasNodes());
    }
