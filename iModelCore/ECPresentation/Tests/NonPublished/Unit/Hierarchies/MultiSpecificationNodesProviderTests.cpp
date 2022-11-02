/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Rules/PresentationRules.h>
#include "../../../../Source/Hierarchies/NavNodeProviders.h"
#include "../../../../Source/Hierarchies/NavNodesCache.h"
#include "NodesProviderTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiSpecificationNodesProviderTests : NodesProviderTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSpecificationNodesProviderTests, ReturnsNodesForAllSpecifications)
    {
    RootNodeRule rule;
    CustomNodeSpecification spec1(1, false, "type", "label1", "", "imageId");
    CustomNodeSpecification spec2(1, false, "type", "label2", "", "imageId");

    RootNodeRuleSpecificationsList specs;
    specs.push_back(RootNodeRuleSpecification(spec1, rule));
    specs.push_back(RootNodeRuleSpecification(spec2, rule));

    NavNodesProviderPtr provider = MultiSpecificationNodesProvider::Create(*m_context, specs);

    auto iter = provider->begin();
    EXPECT_TRUE(iter != provider->end());
    EXPECT_STREQ("label1", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    EXPECT_TRUE(iter != provider->end());
    EXPECT_STREQ("label2", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    EXPECT_TRUE(iter == provider->end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    NavNodesProviderPtr provider = MultiSpecificationNodesProvider::Create(*m_context, specs);

    auto iter = provider->begin();
    EXPECT_TRUE(iter != provider->end());
    EXPECT_STREQ("label1", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    EXPECT_TRUE(iter != provider->end());
    EXPECT_STREQ("label3", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    EXPECT_TRUE(iter == provider->end());
    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
* @bsitest
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
* @bsitest
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

    NavNodesProviderPtr provider = MultiSpecificationNodesProvider::Create(*m_context, specs);
    EXPECT_EQ(2, provider->GetNodesCount());
    for (NavNodePtr node : *provider)
        ;
    EXPECT_EQ(2, provider->GetNodesCount());

    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSpecificationNodesProviderTests, EvaluatesChildrenArtifactsWhenCheckingChildrenAndFirstSpecDoesNotHaveArtifacts)
    {
    auto captureArtifacts = ArtifactsCapturer::Create();
    m_context->AddArtifactsCapturer(captureArtifacts.get());

    RootNodeRuleP rule1 = new RootNodeRule();
    CustomNodeSpecificationP spec1 = new CustomNodeSpecification(1, false, "type1", "label1", "", "imageId");
    rule1->AddSpecification(*spec1);
    m_ruleset->AddPresentationRule(*rule1);

    RootNodeRuleP rule2 = new RootNodeRule();
    CustomNodeSpecificationP spec2 = new CustomNodeSpecification(1, false, "type2", "label2", "", "imageId");
    rule2->AddSpecification(*spec2);
    m_ruleset->AddPresentationRule(*rule2);

    bmap<Utf8String, Utf8String> artifactDefinitions;
    artifactDefinitions.Insert("test", "123");
    rule2->AddCustomizationRule(*new NodeArtifactsRule("", artifactDefinitions));

    RootNodeRuleSpecificationsList specs;
    specs.push_back(RootNodeRuleSpecification(*spec1, *rule1));
    specs.push_back(RootNodeRuleSpecification(*spec2, *rule2));

    NavNodesProviderPtr provider = MultiSpecificationNodesProvider::Create(*m_context, specs);
    EXPECT_EQ(true, provider->HasNodes());

    EXPECT_EQ(1, captureArtifacts->GetArtifacts().size());
    }
