/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/CustomizationHelperTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "../../../Source/RulesDriven/RulesEngine/CustomizationHelper.h"
#include "../../../Source/RulesDriven/RulesEngine/NavNodesCache.h"
#include <UnitTests/BackDoor/ECPresentation/TestRulesetLocater.h>
#include "TestHelpers.h"
#include "TestLocalizationProvider.h"
#include "ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Pranciskus.Ambrazas               03/2016
+===============+===============+===============+===============+===============+======*/
struct CustomizationHelperTests : ::testing::Test, IECExpressionsCacheProvider
{
    static ECDbTestProject* s_project;
    static CustomFunctionsInjector* s_customFunctions;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;
    ECSqlStatementCache m_statementCache;
    PresentationRuleSetPtr m_ruleset;
    NavNodesProviderContextPtr m_context;
    TestNodesProviderFactory m_providerFactory;
    TestUserSettings m_settings;
    JsonNavNodesFactory m_nodesFactory;
    TestNodesCache m_nodesCache;


    CustomizationHelperTests() : m_statementCache(5) {}
    
    static void SetUpTestCase();
    static void TearDownTestCase();

    virtual ECExpressionsCache& _Get(Utf8CP) override {return m_expressionsCache;}

    void SetUp() override
        {
        if (!s_project->GetECDb().GetDefaultTransaction()->IsActive())
            s_project->GetECDb().GetDefaultTransaction()->Begin();
        
        m_ruleset = PresentationRuleSet::CreateInstance("CustomizationHelperTests", 1, 0, false, "", "", "", false);
        m_context = NavNodesProviderContext::Create(*m_ruleset, true, TargetTree_Both, 0, 
            m_settings, m_expressionsCache, m_relatedPathsCache, m_nodesFactory, m_nodesCache, m_providerFactory, nullptr);
        m_context->SetQueryContext(s_project->GetECDb(), m_statementCache, *s_customFunctions, nullptr);
        }

    void TearDown() override
        {
        s_project->GetECDb().GetDefaultTransaction()->Cancel();
        }

    Utf8String GetDisplayLabel(IECInstanceCR instance)
        {
        Utf8String label;
        if (ECObjectsStatus::Success == instance.GetDisplayLabel(label))
            return Utf8String(label.c_str());

        return Utf8String(instance.GetClass().GetDisplayLabel().c_str());
        }
};
ECDbTestProject* CustomizationHelperTests::s_project = nullptr;
CustomFunctionsInjector* CustomizationHelperTests::s_customFunctions = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelperTests::SetUpTestCase()
    {
    CustomizationHelperTests::s_project = new ECDbTestProject();
    CustomizationHelperTests::s_project->Create("CustomizationHelperTests", "RulesEngineTest.01.00.ecschema.xml");
    CustomizationHelperTests::s_customFunctions = new CustomFunctionsInjector(CustomizationHelperTests::s_project->GetECDb());

    BeFileName sqlangFile;
    BeTest::GetHost().GetDocumentsRoot(sqlangFile);
    sqlangFile.AppendToPath(L"ECPresentationTestData");
    sqlangFile.AppendToPath(L"RulesEngineLocalizedStrings.sqlang.db3");
    BeSQLite::L10N::Shutdown();
    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(sqlangFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelperTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(CustomizationHelperTests::s_project);
    DELETE_AND_CLEAR(CustomizationHelperTests::s_customFunctions);

    BeSQLite::L10N::Shutdown();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_NodeWasCustomized)
    {
    JsonNavNodePtr node = m_nodesFactory.CreateCustomNode(BeGuid(true), "label", "description", "imageId", "type");
    CustomizationHelper::Customize(*m_context, *node, false);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyOnlyDescriptionOverride)
    {
    LabelOverrideP labelOverride = new LabelOverride("ThisNode.Type=\"type\"", 1, "\"overridedLabel\"", "\"overridedDescription\"");
    m_ruleset->AddPresentationRule(*labelOverride);
    JsonNavNodePtr node = m_nodesFactory.CreateCustomNode(BeGuid(true), "label", "description", "imageId", "type");
    CustomizationHelper::Customize(*m_context, *node, false);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_STREQ("label", node->GetLabel().c_str());
    ASSERT_STREQ("overridedDescription", node->GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyLabelAndDescriptionOverride)
    {
    LabelOverrideP labelOverride = new LabelOverride("ThisNode.Type=\"type\"", 1, "\"overridedLabel\"", "\"overridedDescription\"");
    m_ruleset->AddPresentationRule(*labelOverride);
    JsonNavNodePtr node = m_nodesFactory.CreateCustomNode(BeGuid(true), "label", "description", "imageId", "type");
    CustomizationHelper::Customize(*m_context, *node, true);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_STREQ("overridedLabel", node->GetLabel().c_str());
    ASSERT_STREQ("overridedDescription", node->GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyStyleOverride)
    {
    StyleOverrideP styleOverride = new StyleOverride("ThisNode.Type=\"customType\"", 1, "\"overridedForeColor\"", "\"overridedBackColor\"", "\"overridedFontStyle\"");
    m_ruleset->AddPresentationRule(*styleOverride);
    JsonNavNodePtr node = m_nodesFactory.CreateCustomNode(BeGuid(true), "label", "description", "imageId", "customType");
    CustomizationHelper::Customize(*m_context, *node, false);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_STREQ("overridedForeColor", node->GetForeColor().c_str());
    ASSERT_STREQ("overridedBackColor", node->GetBackColor().c_str());
    ASSERT_STREQ("overridedFontStyle", node->GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyImageIdOverride)
    {
    ImageIdOverrideP imageIdOverride = new ImageIdOverride("ThisNode.Type=\"customType\"", 1, "\"overridedImageId\"");
    m_ruleset->AddPresentationRule(*imageIdOverride);
    JsonNavNodePtr node = m_nodesFactory.CreateCustomNode(BeGuid(true), "label", "description", "imageId", "customType");
    CustomizationHelper::Customize(*m_context, *node, false);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_STREQ("overridedImageId", node->GetCollapsedImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyCheckboxRules)
    {
    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Type=\"customType\"", 1, false, "", false, false, "");
    m_ruleset->AddPresentationRule(*checkBoxRule);
    JsonNavNodePtr node = m_nodesFactory.CreateCustomNode(BeGuid(true), "label", "description", "imageId", "customType");
    CustomizationHelper::Customize(*m_context, *node, false);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_TRUE(node->IsCheckboxVisible());
    ASSERT_TRUE(node->IsCheckboxEnabled());
    ASSERT_FALSE(node->IsChecked());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizeNode_ApplyLocalization)
    {
    TestLocalizationProvider provider;
    provider.SetHandler([](Utf8StringCR key, Utf8StringR localizedValue) { localizedValue = "localized"; return true; });
    m_context->SetLocalizationContext(provider);
    JsonNavNodePtr node = m_nodesFactory.CreateCustomNode(BeGuid(true), "@customLabel@", "description", "imageId", "type");
    CustomizationHelper::Customize(*m_context, *node, false);
    ASSERT_TRUE(NavNodeExtendedData(*node).IsCustomized());
    ASSERT_STREQ("localized", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CustomizationHelperTests, CustomizationExpressionContextHasParentNodeSymbols)
    {
    JsonNavNodePtr parentNode = m_nodesFactory.CreateCustomNode(BeGuid(true), "Parent", "description", "imageId", "ParentType");
    m_nodesCache.Cache(*parentNode, false);
    uint64_t parentNodeId = parentNode->GetNodeId();

    ChildNodeRule rule("", 1, false, RuleTargetTree::TargetTree_Both);
    NavNodesProviderContextPtr childContext = NavNodesProviderContext::Create(*m_ruleset, true, TargetTree_Both, &parentNodeId, 
        m_settings, m_expressionsCache, m_relatedPathsCache, m_nodesFactory, m_nodesCache, m_providerFactory, nullptr);
    childContext->SetQueryContext(*m_context);
    childContext->SetChildNodeContext(rule, *parentNode);

    JsonNavNodePtr thisNode = m_nodesFactory.CreateCustomNode(BeGuid(true), "This", "description", "imageId", "ThisType");
    NavNodeExtendedData(*thisNode).SetVirtualParentId(parentNodeId);

    StyleOverrideP styleOverride = new StyleOverride("ParentNode.Type=\"ParentType\"", 1, "\"overridenForeColor\"", "\"overridenBackColor\"", "\"overridenFontStyle\"");
    m_ruleset->AddPresentationRule(*styleOverride);
    CustomizationHelper::Customize(*childContext, *thisNode, false);
    ASSERT_TRUE(NavNodeExtendedData(*thisNode).IsCustomized());
    ASSERT_STREQ("overridenForeColor", thisNode->GetForeColor().c_str());
    ASSERT_STREQ("overridenBackColor", thisNode->GetBackColor().c_str());
    ASSERT_STREQ("overridenFontStyle", thisNode->GetFontStyle().c_str());
    }
