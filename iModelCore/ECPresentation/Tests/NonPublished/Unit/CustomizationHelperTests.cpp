/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../Source/Shared/CustomizationHelper.h"
#include "../../../Source/Hierarchies/NavNodesCache.h"
#include <UnitTests/ECPresentation/TestRuleSetLocater.h>
#include "../Helpers/TestHelpers.h"
#include "../Helpers/ECDbTestProject.h"
#include "../Helpers/TestNodesCache.h"
#include "../Helpers/TestNodesProvider.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CustomizationHelperTests : ECPresentationTest
{
    static ECDbTestProject* s_project;
    CustomFunctionsInjector* m_customFunctions;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;
    PresentationRuleSetPtr m_ruleset;
    NavNodesProviderContextPtr m_context;
    TestNodesProviderFactory m_providerFactory;
    std::unique_ptr<TestNodesFactory> m_nodesFactory;
    std::shared_ptr<TestNodesCache> m_nodesCache;

    CustomizationHelperTests() {}

    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        if (!s_project->GetECDb().GetDefaultTransaction()->IsActive())
            s_project->GetECDb().GetDefaultTransaction()->Begin();

        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        m_nodesCache = std::make_shared<TestNodesCache>();
        m_customFunctions = new CustomFunctionsInjector(m_connections, *m_connection);
        m_ruleset = PresentationRuleSet::CreateInstance("CustomizationHelperTests");
        m_nodesFactory = std::make_unique<TestNodesFactory>(*m_connection, "specificationIdentifier", m_ruleset->GetRuleSetId());
        m_context = NavNodesProviderContext::Create(*m_ruleset, TargetTree_Both, nullptr,
            std::make_unique<RulesetVariables>(), m_expressionsCache, m_relatedPathsCache,
            *m_nodesFactory, m_nodesCache, m_providerFactory, nullptr);
        m_context->SetQueryContext(m_connections, *m_connection, nullptr);
        }

    void TearDown() override
        {
        s_project->GetECDb().GetDefaultTransaction()->Cancel();
        DELETE_AND_CLEAR(m_customFunctions);
        }

    Utf8String GetDisplayLabel(IECInstanceCR instance)
        {
        Utf8String label;
        if (ECObjectsStatus::Success == instance.GetDisplayLabel(label))
            return Utf8String(label.c_str());

        return Utf8String(instance.GetClass().GetDisplayLabel().c_str());
        }

    NavNodePtr CreateNode(Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type)
        {
        return m_nodesFactory->CreateCustomNode(nullptr, label, description, imageId, type);
        }
};
ECDbTestProject* CustomizationHelperTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelperTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("CustomizationHelperTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelperTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_NodeWasCustomized)
    {
    NavNodePtr node = CreateNode("label", "description", "imageId", "type");
    CustomizationHelper::Customize(*m_context, nullptr, *node);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyLabelAndDescriptionOverride)
    {
    LabelOverrideP labelOverride = new LabelOverride("ThisNode.Type=\"type\"", 1, "\"overridedLabel\"", "\"overridedDescription\"");
    m_ruleset->AddPresentationRule(*labelOverride);
    NavNodePtr node = CreateNode("label", "description", "imageId", "type");
    CustomizationHelper::Customize(*m_context, nullptr, *node);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_STREQ("overridedLabel", node->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("overridedDescription", node->GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyStyleOverride)
    {
    StyleOverrideP styleOverride = new StyleOverride("ThisNode.Type=\"customType\"", 1, "\"overridedForeColor\"", "\"overridedBackColor\"", "\"overridedFontStyle\"");
    m_ruleset->AddPresentationRule(*styleOverride);
    NavNodePtr node = CreateNode("label", "description", "imageId", "customType");
    CustomizationHelper::Customize(*m_context, nullptr, *node);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_STREQ("overridedForeColor", node->GetForeColor().c_str());
    ASSERT_STREQ("overridedBackColor", node->GetBackColor().c_str());
    ASSERT_STREQ("overridedFontStyle", node->GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyImageIdOverride)
    {
    ImageIdOverrideP imageIdOverride = new ImageIdOverride("ThisNode.Type=\"customType\"", 1, "\"overridedImageId\"");
    m_ruleset->AddPresentationRule(*imageIdOverride);
    NavNodePtr node = CreateNode("label", "description", "imageId", "customType");
    CustomizationHelper::Customize(*m_context, nullptr, *node);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_STREQ("overridedImageId", node->GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyCheckboxRules)
    {
    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Type=\"customType\"", 1, false, "", false, false, "");
    m_ruleset->AddPresentationRule(*checkBoxRule);
    NavNodePtr node = CreateNode("label", "description", "imageId", "customType");
    CustomizationHelper::Customize(*m_context, nullptr, *node);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_TRUE(node->IsCheckboxVisible());
    ASSERT_TRUE(node->IsCheckboxEnabled());
    ASSERT_FALSE(node->IsChecked());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomizationHelperTests, CustomizeNode_ApplyExtendedDataRules)
    {
    ExtendedDataRuleP rule = new ExtendedDataRule("ThisNode.Type=\"customType\"");
    rule->AddItem("key1", "\"test string\"");
    rule->AddItem("key2", "ThisNode.Label & \" \" & ThisNode.Type");
    m_ruleset->AddPresentationRule(*rule);
    NavNodePtr node = CreateNode("label", "description", "imageId", "customType");
    CustomizationHelper::Customize(*m_context, nullptr, *node);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    EXPECT_EQ(2, node->GetUsersExtendedData().GetJson().MemberCount());
    EXPECT_STREQ("test string", node->GetUsersExtendedData().GetJson()["key1"].GetString());
    EXPECT_STREQ("label customType", node->GetUsersExtendedData().GetJson()["key2"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomizationHelperTests, CustomizeNode_ApplyExtendedDataRulesByPriority)
    {
    ExtendedDataRuleP rule = new ExtendedDataRule("ThisNode.Type=\"customType\"");
    rule->AddItem("key1", "\"test string\"");
    rule->SetPriority(2000);
    ExtendedDataRuleP rule2 = new ExtendedDataRule("ThisNode.Type=\"customType\"");
    rule2->AddItem("key1", "\"wrong string\"");
    rule2->AddItem("key2", "\"test string2\"");
    m_ruleset->AddPresentationRule(*rule);
    m_ruleset->AddPresentationRule(*rule2);
    NavNodePtr node = CreateNode("label", "description", "imageId", "customType");
    CustomizationHelper::Customize(*m_context, nullptr, *node);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    EXPECT_EQ(2, node->GetUsersExtendedData().GetJson().MemberCount());
    EXPECT_STREQ("test string", node->GetUsersExtendedData().GetJson()["key1"].GetString());
    EXPECT_STREQ("test string2", node->GetUsersExtendedData().GetJson()["key2"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomizationHelperTests, CustomizeNode_ApplyExtendedDataRulesByPriorityWithInvalidExpression)
    {
    ExtendedDataRuleP rule = new ExtendedDataRule("ThisNode.Type=\"customType\"");
    rule->AddItem("key1", "this.wrongProperty" );
    rule->SetPriority(2000);
    ExtendedDataRuleP rule2 = new ExtendedDataRule("ThisNode.Type=\"customType\"");
    rule2->AddItem("key1", "\"wrong string\"");
    rule2->AddItem("key2", "\"test string2\"");
    m_ruleset->AddPresentationRule(*rule);
    m_ruleset->AddPresentationRule(*rule2);
    NavNodePtr node = CreateNode("label", "description", "imageId", "customType");
    CustomizationHelper::Customize(*m_context, nullptr, *node);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    EXPECT_EQ(1, node->GetUsersExtendedData().GetJson().MemberCount());
    EXPECT_STREQ("test string2", node->GetUsersExtendedData().GetJson()["key2"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomizationHelperTests, CustomizeNode_EvaluateArtifacts)
    {
    NodeArtifactsRuleP rule = new NodeArtifactsRule("ThisNode.Type=\"customType\"");
    rule->AddItem("string_value", "\"test string\"");
    rule->AddItem("boolean_value", "(1 + 2) = 3");
    rule->AddItem("int_value", "1 + 2");
    m_ruleset->AddPresentationRule(*rule);
    NavNodePtr node = CreateNode("label", "description", "imageId", "customType");
    auto result = CustomizationHelper::EvaluateArtifacts(*m_context, *node);
    EXPECT_EQ(3, result.size());
    EXPECT_EQ(ECValue("test string"), result["string_value"]);
    EXPECT_EQ(ECValue(true), result["boolean_value"]);
    EXPECT_EQ(ECValue(3), result["int_value"]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizationExpressionContextHasParentNodeSymbols)
    {
    NavNodePtr parentNode = CreateNode("Parent", "description", "imageId", "ParentType");
    RulesEngineTestHelpers::CacheNode(*m_nodesCache, *parentNode);

    NavNodePtr thisNode = CreateNode("This", "description", "imageId", "ThisType");
    RulesEngineTestHelpers::CacheNode(*m_nodesCache, *thisNode, parentNode->GetNodeId());

    ChildNodeRule rule("", 1, false, RuleTargetTree::TargetTree_Both);
    NavNodesProviderContextPtr childContext = NavNodesProviderContext::Create(*m_ruleset, TargetTree_Both, parentNode.get(),
        std::make_unique<RulesetVariables>(), m_expressionsCache, m_relatedPathsCache,
        *m_nodesFactory, m_nodesCache, m_providerFactory, nullptr);
    childContext->SetQueryContext(*m_context);
    childContext->SetChildNodeContext(&rule, *parentNode);

    StyleOverrideP styleOverride = new StyleOverride("ParentNode.Type=\"ParentType\"", 1, "\"overridenForeColor\"", "\"overridenBackColor\"", "\"overridenFontStyle\"");
    m_ruleset->AddPresentationRule(*styleOverride);
    NodesFinalizer(*childContext).Customize(*thisNode);
    ASSERT_TRUE(NavNodeExtendedData(*thisNode).IsCustomized());
    ASSERT_STREQ("overridenForeColor", thisNode->GetForeColor().c_str());
    ASSERT_STREQ("overridenBackColor", thisNode->GetBackColor().c_str());
    ASSERT_STREQ("overridenFontStyle", thisNode->GetFontStyle().c_str());
    }
