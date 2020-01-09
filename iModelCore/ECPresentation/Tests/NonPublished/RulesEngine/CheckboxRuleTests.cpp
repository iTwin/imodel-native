/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodeProviders.h"
#include "../../../Source/RulesDriven/RulesEngine/CustomizationHelper.h"
#include "NodesProviderTests.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct CheckboxRuleTests : NodesProviderTests
    {
    ECClassCP m_widgetClass;
    void SetUp() override
        {
        NodesProviderTests::SetUp();
        m_widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, SetsPropertyBoundCheckboxProperties)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("A"));
        instance.SetValue("BoolProperty", ECValue(true));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("B"));
        instance.SetValue("BoolProperty", ECValue(false));
        });
    
    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "BoolProperty", false, false, "");
    m_ruleset->AddPresentationRule(*rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash("");
    
    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    ASSERT_EQ(2, provider->GetNodesCount());
    for (size_t i = 0; i < 2; i++)
        {
        JsonNavNodePtr node;
        ASSERT_TRUE(provider->GetNode(node, i));
        EXPECT_TRUE(node->IsCheckboxVisible());
        EXPECT_TRUE(node->IsCheckboxEnabled());

        NavNodeExtendedData extendedData(*node);
        EXPECT_STREQ("BoolProperty", extendedData.GetCheckboxBoundPropertyName());
        EXPECT_FALSE(extendedData.IsCheckboxBoundPropertyInversed());

        if (node->GetLabelDefinition().GetDisplayValue().Equals("A"))
            EXPECT_TRUE(node->IsChecked());
        else
            EXPECT_FALSE(node->IsChecked());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, SetsInversedPropertyBoundCheckboxProperties)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("A"));
        instance.SetValue("BoolProperty", ECValue(true));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("B"));
        instance.SetValue("BoolProperty", ECValue(false));
        });
    
    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "BoolProperty", true, false, "");
    m_ruleset->AddPresentationRule(*rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash("");
    
    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    ASSERT_EQ(2, provider->GetNodesCount());
    for (size_t i = 0; i < 2; i++)
        {
        JsonNavNodePtr node;
        ASSERT_TRUE(provider->GetNode(node, i));
        EXPECT_TRUE(node->IsCheckboxVisible());
        EXPECT_TRUE(node->IsCheckboxEnabled());

        NavNodeExtendedData extendedData(*node);
        EXPECT_STREQ("BoolProperty", extendedData.GetCheckboxBoundPropertyName());
        EXPECT_TRUE(extendedData.IsCheckboxBoundPropertyInversed());

        if (node->GetLabelDefinition().GetDisplayValue().Equals("A"))
            EXPECT_FALSE(node->IsChecked());
        else
            EXPECT_TRUE(node->IsChecked());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, AppliesDefaultValueIfPropertyIsNull)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    
    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "BoolProperty", true, true, "");
    m_ruleset->AddPresentationRule(*rule);

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash("");
    
    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    ASSERT_EQ(1, provider->GetNodesCount());
    JsonNavNodePtr node;
    ASSERT_TRUE(provider->GetNode(node, 0));
    ASSERT_TRUE(node.IsValid());
    EXPECT_TRUE(node->IsChecked());
    EXPECT_TRUE(node->IsCheckboxVisible());
    EXPECT_TRUE(node->IsCheckboxEnabled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, AppliesDefaultValueIfNotPropertyBound)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    
    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "", false, true, "");
    m_ruleset->AddPresentationRule(*rule);

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash("");
    
    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    ASSERT_EQ(1, provider->GetNodesCount());
    JsonNavNodePtr node;
    ASSERT_TRUE(provider->GetNode(node, 0));
    ASSERT_TRUE(node.IsValid());
    EXPECT_TRUE(node->IsChecked());
    EXPECT_TRUE(node->IsCheckboxVisible());
    EXPECT_TRUE(node->IsCheckboxEnabled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, SetCheckBoxEnabled)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    
    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "", true, true, "1=1");
    m_ruleset->AddPresentationRule(*rule);

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash("");
    
    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    ASSERT_EQ(1, provider->GetNodesCount());
    JsonNavNodePtr node;
    ASSERT_TRUE(provider->GetNode(node, 0));
    ASSERT_TRUE(node.IsValid());
    EXPECT_TRUE(node->IsCheckboxEnabled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, SetCheckBoxDisabled)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    
    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "", false, true, "1>2");
    m_ruleset->AddPresentationRule(*rule);

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash("");
    
    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    ASSERT_EQ(1, provider->GetNodesCount());
    JsonNavNodePtr node;
    ASSERT_TRUE(provider->GetNode(node, 0));
    ASSERT_TRUE(node.IsValid());
    EXPECT_FALSE(node->IsCheckboxEnabled());
    }
