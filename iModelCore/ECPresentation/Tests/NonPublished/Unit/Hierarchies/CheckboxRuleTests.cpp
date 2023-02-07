/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Rules/PresentationRules.h>
#include "../../../../Source/Hierarchies/NavNodeProviders.h"
#include "../../../../Source/Shared/CustomizationHelper.h"
#include "../../Helpers/TestHelpers.h"
#include "NodesProviderTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*=================================================================================**//**
* @bsiclass
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
* @bsitest
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

    SelectClass<ECClass> selectClass(*m_widgetClass, "this", false);
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract).From(selectClass);
    query->GetNavigationResultParameters().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    auto provider = PostProcess(*QueryBasedNodesProvider::Create(*m_context, *query));
    ASSERT_EQ(2, provider->GetNodesCount());
    for (NavNodePtr node : *provider)
        {
        ASSERT_TRUE(node.IsValid());
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
* @bsitest
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

    SelectClass<ECClass> selectClass(*m_widgetClass, "this", false);
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract).From(selectClass);
    query->GetNavigationResultParameters().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    auto provider = PostProcess(*QueryBasedNodesProvider::Create(*m_context, *query));
    ASSERT_EQ(2, provider->GetNodesCount());
    for (NavNodePtr node : *provider)
        {
        ASSERT_TRUE(node.IsValid());
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
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, AppliesDefaultValueIfPropertyIsNull)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "BoolProperty", true, true, "");
    m_ruleset->AddPresentationRule(*rule);

    SelectClass<ECClass> selectClass(*m_widgetClass, "this", false);
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract).From(selectClass);
    query->GetNavigationResultParameters().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    auto provider = PostProcess(*QueryBasedNodesProvider::Create(*m_context, *query));
    ASSERT_EQ(1, provider->GetNodesCount());
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    ASSERT_TRUE((*iter).IsValid());
    EXPECT_TRUE((*iter)->IsChecked());
    EXPECT_TRUE((*iter)->IsCheckboxVisible());
    EXPECT_TRUE((*iter)->IsCheckboxEnabled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, AppliesDefaultValueIfNotPropertyBound)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "", false, true, "");
    m_ruleset->AddPresentationRule(*rule);

    SelectClass<ECClass> selectClass(*m_widgetClass, "this", false);
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract).From(selectClass);
    query->GetNavigationResultParameters().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    auto provider = PostProcess(*QueryBasedNodesProvider::Create(*m_context, *query));
    ASSERT_EQ(1, provider->GetNodesCount());
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    ASSERT_TRUE((*iter).IsValid());
    EXPECT_TRUE((*iter)->IsChecked());
    EXPECT_TRUE((*iter)->IsCheckboxVisible());
    EXPECT_TRUE((*iter)->IsCheckboxEnabled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, SetCheckBoxEnabled)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "", true, true, "1=1");
    m_ruleset->AddPresentationRule(*rule);

    SelectClass<ECClass> selectClass(*m_widgetClass, "this", false);
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract).From(selectClass);
    query->GetNavigationResultParameters().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    auto provider = PostProcess(*QueryBasedNodesProvider::Create(*m_context, *query));
    ASSERT_EQ(1, provider->GetNodesCount());
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    ASSERT_TRUE((*iter).IsValid());
    EXPECT_TRUE((*iter)->IsCheckboxEnabled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckboxRuleTests, SetCheckBoxDisabled)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    CheckBoxRuleP rule = new CheckBoxRule("", 1, false, "", false, true, "1>2");
    m_ruleset->AddPresentationRule(*rule);

    SelectClass<ECClass> selectClass(*m_widgetClass, "this", false);
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract).From(selectClass);
    query->GetNavigationResultParameters().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    auto provider = PostProcess(*QueryBasedNodesProvider::Create(*m_context, *query));
    ASSERT_EQ(1, provider->GetNodesCount());
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    ASSERT_TRUE((*iter).IsValid());
    EXPECT_FALSE((*iter)->IsCheckboxEnabled());
    }
