/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "NodesProviderTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct CustomNodesProviderTests : NodesProviderTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, InvalidSpecification_DoesntReturnNodes)
    {
    BeTest::SetFailOnAssert(false);

    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

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

    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, ReturnsOneNode)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

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
    m_context->SetRootNodeContext(&rule);

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
    m_context->SetRootNodeContext(&rule);

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
    TestNavNodePtr parent = TestNodesHelper::CreateCustomNode(*m_connection, "parent_type", "parent_label", "parent_descr");
    Cache(*parent);

    ChildNodeRule rule;
    m_context->SetChildNodeContext(&rule, *parent);

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
    rule->AddSpecification(*spec);    
    m_context->SetRootNodeContext(rule);

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
    rule->AddSpecification(*spec);    
    m_context->SetRootNodeContext(rule);

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
    rule->AddSpecification(*spec);    
    m_context->SetRootNodeContext(rule);

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
    m_context->SetRootNodeContext(&rule);

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
    m_context->SetRootNodeContext(&rule);

    CustomNodeSpecification spec(1, false, "type", "label", "description", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_TRUE(provider->HasNodes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, GetArtifacts)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

    bmap<Utf8String, Utf8String> artifactDefinitions;
    artifactDefinitions.Insert("TestArtifact", "2 + 3");
    m_ruleset->AddPresentationRule(*new NodeArtifactsRule("", artifactDefinitions));

    CustomNodeSpecification spec(1, false, "type", "label", "description", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    bvector<NodeArtifacts> artifacts = provider->GetArtifacts();
    ASSERT_EQ(1, artifacts.size());
    ASSERT_EQ(1, artifacts[0].size());
    auto iter = artifacts[0].find("TestArtifact");
    ASSERT_TRUE(artifacts[0].end() != iter);
    EXPECT_EQ(ECValue(5), iter->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, SetsChildNodeArtifactsExtendedData)
    {
    RootNodeRule rootRule;
    m_context->SetRootNodeContext(&rootRule);
    CustomNodeSpecification rootSpec(1, false, "t_parent", "label", "description", "imageId");

    auto childrenRule = new ChildNodeRule("ParentNode.Type = \"t_parent\"", 1, false, TargetTree_Both);
    childrenRule->AddSpecification(*new CustomNodeSpecification(1, false, "t_child", "label", "description", "imageId"));
    m_ruleset->AddPresentationRule(*childrenRule);

    bmap<Utf8String, Utf8String> artifactDefinitions;
    artifactDefinitions.Insert("TestArtifact", "\"test value\"");
    m_ruleset->AddPresentationRule(*new NodeArtifactsRule("ThisNode.Type = \"t_child\"", artifactDefinitions));

    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, rootSpec);
    JsonNavNodePtr node;
    ASSERT_TRUE(provider->GetNode(node, 0));
    auto artifacts = NavNodeExtendedData(*node).GetChildrenArtifacts();
    ASSERT_EQ(1, artifacts.size());
    ASSERT_EQ(1, artifacts[0].size());
    auto iter = artifacts[0].find("TestArtifact");
    ASSERT_TRUE(artifacts[0].end() != iter);
    EXPECT_EQ(ECValue("test value"), iter->second);
    }