/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Rules/PresentationRules.h>
#include "NodesProviderTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CustomNodesProviderTests : NodesProviderTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, InvalidSpecification_DoesntReturnNodes)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

    CustomNodeSpecification specNoType(1, false, "", "label", "", "imageId");
    NavNodesProviderPtr provider1 = CustomNodesProvider::Create(*m_context, specNoType);
    EXPECT_EQ(0, provider1->GetNodesCount());
    EXPECT_TRUE(provider1->begin() == provider1->end());

    CustomNodeSpecification specNoLabel(1, false, "type", "", "", "imageId");
    NavNodesProviderPtr provider2 = CustomNodesProvider::Create(*m_context, specNoLabel);
    EXPECT_EQ(0, provider2->GetNodesCount());
    EXPECT_TRUE(provider2->begin() == provider2->end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, ReturnsOneNode)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

    CustomNodeSpecification spec(1, false, "type", "label", "", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    auto iter = provider->begin();
    EXPECT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter).IsValid());
    ++iter;
    EXPECT_TRUE(iter == provider->end());
    EXPECT_EQ(1, provider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, ReturnsNodesAfterCount)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

    CustomNodeSpecification spec(1, false, "type", "label", "", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    EXPECT_EQ(1, provider->GetNodesCount());
    auto iter = provider->begin();
    EXPECT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter).IsValid());
    ++iter;
    EXPECT_TRUE(iter == provider->end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, ReturnsValidNode)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

    NavNodePtr node;
    CustomNodeSpecification spec(1, false, "type", "label", "description", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    auto iter = provider->begin();
    EXPECT_TRUE(iter != provider->end());
    EXPECT_EQ(1, provider->GetNodesCount());
    ASSERT_TRUE((*iter).IsValid());
    EXPECT_STREQ("type", (*iter)->GetType().c_str());
    EXPECT_STREQ("label", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("description", (*iter)->GetDescription().c_str());
    EXPECT_STREQ("imageId", (*iter)->GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, ReturnsValidChildNode)
    {
    auto parent = TestNodesHelper::CreateCustomNode(*m_connection, "parent_type", "parent_label", "parent_descr");
    Cache(*parent);

    ChildNodeRule rule;
    m_context->SetChildNodeContext(&rule, *parent);

    CustomNodeSpecification spec(1, false, "type", "label", "description", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, spec);
    auto iter = provider->begin();
    EXPECT_TRUE(iter != provider->end());
    EXPECT_EQ(1, provider->GetNodesCount());
    ASSERT_TRUE((*iter).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    auto provider = PostProcess(*CustomNodesProvider::Create(*m_context, *spec));
    auto iter = provider->begin();
    EXPECT_TRUE(iter != provider->end());
    ASSERT_TRUE((*iter).IsValid());
    EXPECT_STREQ("overriden_label", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("overriden_description", (*iter)->GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    auto provider = PostProcess(*CustomNodesProvider::Create(*m_context, *spec));
    auto iter = provider->begin();
    EXPECT_TRUE(iter != provider->end());
    ASSERT_TRUE((*iter).IsValid());
    EXPECT_STREQ("ForeColor1", (*iter)->GetForeColor().c_str());
    EXPECT_STREQ("BackColor1", (*iter)->GetBackColor().c_str());
    EXPECT_STREQ("FontStyle1", (*iter)->GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    auto provider = PostProcess(*CustomNodesProvider::Create(*m_context, *spec));
    auto iter = provider->begin();
    EXPECT_TRUE(iter != provider->end());
    ASSERT_TRUE((*iter).IsValid());
    EXPECT_STREQ("MyImageId", (*iter)->GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, HasNodesReturnsFalseWhenSpecificationIsInvalid)
    {
    RootNodeRule rule;
    m_context->SetRootNodeContext(&rule);

    CustomNodeSpecification specNoType(1, false, "", "label", "", "imageId");
    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, specNoType);
    EXPECT_FALSE(provider->HasNodes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodesProviderTests, SetsChildNodeArtifactsExtendedData)
    {
    RootNodeRule rootRule;
    m_context->SetRootNodeContext(&rootRule);
    CustomNodeSpecification rootSpec(1, false, "t_parent", "label", "description", "imageId");
    // note: children artifacts are evaluated only if used in hide expression (for performance reasons)
    rootSpec.SetHideExpression("ThisNode.ChildrenArtifacts");

    auto childrenRule = new ChildNodeRule("ParentNode.Type = \"t_parent\"", 1, false, TargetTree_Both);
    childrenRule->AddSpecification(*new CustomNodeSpecification(1, false, "t_child", "label", "description", "imageId"));
    m_ruleset->AddPresentationRule(*childrenRule);

    bmap<Utf8String, Utf8String> artifactDefinitions;
    artifactDefinitions.Insert("TestArtifact", "\"test value\"");
    m_ruleset->AddPresentationRule(*new NodeArtifactsRule("ThisNode.Type = \"t_child\"", artifactDefinitions));

    NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, rootSpec);
    auto nodeIter = provider->begin();
    EXPECT_TRUE(nodeIter != provider->end());
    auto artifacts = NavNodeExtendedData(**nodeIter).GetChildrenArtifacts();
    ASSERT_EQ(1, artifacts.size());
    ASSERT_EQ(1, artifacts[0].size());
    auto iter = artifacts[0].find("TestArtifact");
    ASSERT_TRUE(artifacts[0].end() != iter);
    EXPECT_EQ(ECValue("test value"), iter->second);
    }
