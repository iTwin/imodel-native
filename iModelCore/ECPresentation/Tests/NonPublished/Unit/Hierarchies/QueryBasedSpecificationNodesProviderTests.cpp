/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../../Source/Hierarchies/NavigationQueryContracts.h"
#include "NodesProviderTests.h"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBasedSpecificationNodesProviderTests : NodesProviderTests
    {
    ECEntityClassCP m_widgetClass;
    ECEntityClassCP m_gadgetClass;
    ECEntityClassCP m_sprocketClass;
    ECRelationshipClassCP m_widgetHasGadgetsRelationshipClass;
    ECRelationshipClassCP m_gadgetHasSprocketsRelationshipClass;

    void SetUp() override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedSpecificationNodesProviderTests::SetUp()
    {
    m_widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    m_gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    m_sprocketClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();
    m_widgetHasGadgetsRelationshipClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    m_gadgetHasSprocketsRelationshipClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    ECInstanceInserter widgetInserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Test Widget 1"));
        instance.SetValue("BoolProperty", ECValue(true));
        });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Test Widget 2"));
        instance.SetValue("BoolProperty", ECValue(false));
        });
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Test Widget 3"));
        instance.SetValue("BoolProperty", ECValue(true));
        });

    ECInstanceInserter gadgetInserter(s_project->GetECDb(), *m_gadgetClass, nullptr);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);

    ECInstanceInserter sprocketInserter(s_project->GetECDb(), *m_sprocketClass, nullptr);
    IECInstancePtr sprocket1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), sprocketInserter, *m_sprocketClass);
    IECInstancePtr sprocket2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), sprocketInserter, *m_sprocketClass);
    IECInstancePtr sprocket3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), sprocketInserter, *m_sprocketClass);
    IECInstancePtr sprocket4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), sprocketInserter, *m_sprocketClass);

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *m_widgetHasGadgetsRelationshipClass, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *m_widgetHasGadgetsRelationshipClass, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *m_widgetHasGadgetsRelationshipClass, *widget1, *gadget3);

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *m_gadgetHasSprocketsRelationshipClass, *gadget1, *sprocket1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *m_gadgetHasSprocketsRelationshipClass, *gadget1, *sprocket2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *m_gadgetHasSprocketsRelationshipClass, *gadget1, *sprocket3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *m_gadgetHasSprocketsRelationshipClass, *gadget1, *sprocket4);

    NodesProviderTests::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, OverridesLabel)
    {
    LabelOverrideP widgetLabelOverride = new LabelOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Widget\"", 1, "\"WidgetLabel\"", "");
    LabelOverrideP gadgetLabelOverride = new LabelOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Gadget\"", 1, "\"GadgetLabel\"", "");
    m_ruleset->AddPresentationRule(*widgetLabelOverride);
    m_ruleset->AddPresentationRule(*gadgetLabelOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.ECInstanceId = 1 OR this.ECInstanceId = 4", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    auto iter = provider->begin();

    ASSERT_TRUE(iter != provider->end());
    ASSERT_TRUE(nullptr != (*iter)->GetKey()->AsECInstanceNodeKey());
    ASSERT_FALSE((*iter)->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().empty());
    EXPECT_EQ(m_gadgetClass, (*iter)->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetClass());
    EXPECT_STREQ("GadgetLabel", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_TRUE(nullptr != (*iter)->GetKey()->AsECInstanceNodeKey());
    ASSERT_FALSE((*iter)->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().empty());
    EXPECT_EQ(m_widgetClass, (*iter)->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetClass());
    EXPECT_STREQ("WidgetLabel", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, UsesRelatedInstancePropertiesWhenOverridingLabel)
    {
    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsInstanceNode AND widget.BoolProperty", 1, "widget.MyID", "");
    m_ruleset->AddPresentationRule(*labelOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rule->AddSpecification(*spec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    ASSERT_EQ(3, provider->GetNodesCount());

    for (NavNodePtr node : *provider)
        {
        EXPECT_STREQ("Test Widget 1", node->GetLabelDefinition().GetDisplayValue().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, OverridesStyle)
    {
    StyleOverrideP styleOverride = new StyleOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Widget\"", 1,
        "IIf(True, \"ForeColor1\", \"ForeColor2\")",
        "IIf(True, \"BackColor1\", \"BackColor2\")",
        "IIf(True, \"FontStyle1\", \"FontStyle2\")");
    m_ruleset->AddPresentationRule(*styleOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.ECInstanceId = 1 OR this.ECInstanceId = 4", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *spec));
    auto iter = provider->begin();

    ASSERT_TRUE(iter != provider->end());
    ASSERT_TRUE(nullptr != (*iter)->GetKey()->AsECInstanceNodeKey());
    ASSERT_FALSE((*iter)->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().empty());
    EXPECT_EQ(m_gadgetClass, (*iter)->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetClass());
    EXPECT_STREQ("", (*iter)->GetForeColor().c_str());
    EXPECT_STREQ("", (*iter)->GetBackColor().c_str());
    EXPECT_STREQ("", (*iter)->GetFontStyle().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_TRUE(nullptr != (*iter)->GetKey()->AsECInstanceNodeKey());
    ASSERT_FALSE((*iter)->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().empty());
    EXPECT_EQ(m_widgetClass, (*iter)->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetClass());
    EXPECT_STREQ("ForeColor1", (*iter)->GetForeColor().c_str());
    EXPECT_STREQ("BackColor1", (*iter)->GetBackColor().c_str());
    EXPECT_STREQ("FontStyle1", (*iter)->GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, ReturnsChildrenIfHideNodesInHierarchyFlagIsSpecified)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);

    ChildNodeRule* childRule = new ChildNodeRule("", 1, false, TargetTree_Both);
    spec->AddNestedRule(*childRule);

    CustomNodeSpecification* customNodeSpec = new CustomNodeSpecification(1, false, "Custom", "Custom", "", "Custom");
    childRule->AddSpecification(*customNodeSpec);

    size_t index = 0;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    for (NavNodePtr node : *provider)
        {
        ASSERT_STREQ("Custom", node->GetType().c_str());
        index++;
        }
    ASSERT_EQ(1, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, ReturnsChildIfDisplayLabelGroupingNodeHasOnlyOne)
    {
    // make sure there's only one Widget
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    size_t index = 0;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    for (NavNodePtr node : *provider)
        {
        ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        ASSERT_STREQ("MyLabel", node->GetLabelDefinition().GetDisplayValue().c_str());
        index++;
        }
    ASSERT_EQ(1, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, ReturnsChildIfGroupingRuleDoesntHaveCreateGroupForSingleItemFlag)
    {
    // make sure there's only one Widget
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    GroupingRule* groupingRule = new GroupingRule("", 1000, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", false, "", ""));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    size_t index = 0;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    for (NavNodePtr node : *provider)
        {
        ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        ASSERT_STREQ("MyLabel", node->GetLabelDefinition().GetDisplayValue().c_str());
        index++;
        }
    ASSERT_EQ(1, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, HasNodesReturnsFalseWhenQueryReturnsNoResults)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    EXPECT_FALSE(provider->HasNodes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, HasNodesReturnsTrueWhenQueryReturnsResults)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    EXPECT_TRUE(provider->HasNodes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, NodesCount_SimpleCase)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

#ifdef WIP_CLEAR_CACHES_ON_DB_CHANGES
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(0, provider->GetNodesCount());

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(1, provider->GetNodesCount());

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(2, provider->GetNodesCount());
#else
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(2, provider->GetNodesCount());
    ASSERT_EQ(2, provider->GetNodesCount());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NodesCount_WithHideNodesInHierarchyFlag)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    ChildNodeRule* childRule = new ChildNodeRule("", 1, false, TargetTree_Both);
    spec->AddNestedRule(*childRule);

    CustomNodeSpecification* customNodeSpec = new CustomNodeSpecification(1, false, "Custom", "Custom", "", "Custom");
    childRule->AddSpecification(*customNodeSpec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
#ifdef WIP_CLEAR_CACHES_ON_DB_CHANGES
    ASSERT_EQ(0, provider->GetNodesCount());
#endif

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(1, provider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NodesCount_WithDisplayLabelGroupingNodeHavingOnlyOneChild)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

#ifdef WIP_CLEAR_CACHES_ON_DB_CHANGES
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(0, provider->GetNodesCount());

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});
    ASSERT_EQ(1, provider->GetNodesCount());

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});
    ASSERT_EQ(1, provider->GetNodesCount());
#else
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("OtherLabel"));});
    ASSERT_EQ(2, provider->GetNodesCount());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NodesCount_WithDisplayLabelAndSameLabelInstanceGrouping)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(""));
    m_ruleset->AddPresentationRule(*groupingRule);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("OtherLabel"));});
    ASSERT_EQ(2, provider->GetNodesCount());

    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, (*iter)->GetType().c_str()); // the node is ECInstanceNode because SameLabelInstanceGroup groups all instances into one - nothing is left to group for display label node
    ASSERT_STREQ("MyLabel", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, (*iter)->GetType().c_str());
    ASSERT_STREQ("OtherLabel", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NodesCount_WithGroupingRuleNotHavingCreateGroupForSingleItemFlag)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    GroupingRule* groupingRule = new GroupingRule("", 1000, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", false, "", ""));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

#ifdef WIP_CLEAR_CACHES_ON_DB_CHANGES
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(0, provider->GetNodesCount());

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(1, provider->GetNodesCount());

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(1, provider->GetNodesCount());
#else
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ASSERT_EQ(1, provider->GetNodesCount());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, CustomizesNodes)
    {
    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Widget\"", 1, "\"QueryBasedSpecificationNodesProviderTests.CustomizesNodes\"", "");
    m_ruleset->AddPresentationRule(*labelOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);

    size_t index = 0;
    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *spec));
    ASSERT_TRUE(0 < provider->GetNodesCount());
    for (NavNodePtr node : *provider)
        {
        NavNodeExtendedData extendedData(*node);
        EXPECT_TRUE(extendedData.IsCustomized());
        index++;
        }
    ASSERT_EQ(6, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, ReturnsChildNodesWhenHideNodesInHierarchyFlagIsSet)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsInstanceNode", 1000, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    m_ruleset->AddPresentationRule(*childRule);

    size_t index = 0;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    ASSERT_TRUE(provider->HasNodes());
    ASSERT_EQ(2, provider->GetNodesCount());
    for (NavNodePtr node : *provider)
        {
        ECInstancesNodeKey const* key = node->GetKey()->AsECInstanceNodeKey();
        ASSERT_TRUE(nullptr != key);
        ASSERT_FALSE(key->GetInstanceKeys().empty());
        EXPECT_EQ(m_gadgetClass, key->GetInstanceKeys().front().GetClass());
        index++;
        }
    ASSERT_EQ(2, index);
    }

#ifdef wip_postprocess
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, ReturnsChildNodesWhenTheresOnlyOneLabelGroupingNode)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(spec);

    size_t index = 0;
    NavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    ASSERT_EQ(2, provider->GetNodesCount());
    while (provider->GetNode(node, index++))
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        EXPECT_STREQ("Widget", node->GetInstance()->GetClass().GetName().c_str());
        }
    ASSERT_EQ(3, index);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, PropertyGrouping_GroupsSubclassNodesWhenSpecificationReturnsParentClassNodesPolymorphically)
    {
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassE");
    ECClassCP classF = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassF");
    ECClassCP classG = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassG");

    // create our own instances
    ECInstanceInserter inserterE(s_project->GetECDb(), *classE, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserterE, *classE, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserterE, *classE, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    ECInstanceInserter inserterF(s_project->GetECDb(), *classF, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserterF, *classF, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserterF, *classF, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserterF, *classF, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserterF, *classF, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    ECInstanceInserter inserterG(s_project->GetECDb(), *classG, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserterG, *classG, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true);
    rule->AddSpecification(*spec);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassF", "", "", "");
    m_ruleset->AddPresentationRule(*groupingRule);

    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    groupingRule->AddGroup(*propertyGroup);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    // 2 property grouping nodes of ClassF, 2 ECInstance nodes of ClassE and 1 ECInstance node of ClassG
    ASSERT_EQ(5, provider->GetNodesCount());
    auto iter = provider->begin();

    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, (*iter)->GetType().c_str());
    ASSERT_STREQ("1", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, (*iter)->GetType().c_str());
    ASSERT_STREQ("2", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, (*iter)->GetType().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, (*iter)->GetType().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, (*iter)->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, PropertyGrouping_DoesntGroupNullValuesIfSpecified)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    m_ruleset->AddPresentationRule(*groupingRule);

    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    propertyGroup->SetCreateGroupForUnspecifiedValues(false);
    groupingRule->AddGroup(*propertyGroup);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    // expect 1 property grouping and 3 instance nodes (not grouped, because IntProperty values are null)
    ASSERT_EQ(4, provider->GetNodesCount());
    auto iter = provider->begin();

    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, (*iter)->GetType().c_str());
    ASSERT_STREQ("5", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, (*iter)->GetType().c_str());
    ASSERT_STREQ("Test Widget 1", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, (*iter)->GetType().c_str());
    ASSERT_STREQ("Test Widget 2", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());

    ++iter;
    ASSERT_TRUE(iter != provider->end());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, (*iter)->GetType().c_str());
    ASSERT_STREQ("Test Widget 3", (*iter)->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NotifiesAboutUsedRelatedClassesInCustomizationRuleValue)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "this.GetRelatedValue(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\", \"MyID\")", ""));

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    size_t index = 0;
    for (NavNodePtr node : *provider)
        index++;
    EXPECT_EQ(3, index);
    ASSERT_EQ(3, m_usedClassesListener.GetUsedClasses().size());

    auto iter = m_usedClassesListener.GetUsedClasses().find(m_widgetClass);
    ASSERT_TRUE(m_usedClassesListener.GetUsedClasses().end() != iter);
    EXPECT_FALSE(iter->second); // not polymorphic

    iter = m_usedClassesListener.GetUsedClasses().find(m_widgetHasGadgetsRelationshipClass);
    ASSERT_TRUE(m_usedClassesListener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic

    iter = m_usedClassesListener.GetUsedClasses().find(m_gadgetClass);
    ASSERT_TRUE(m_usedClassesListener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic
    }

#define NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY   (uint32_t)2

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithForward0To1Relationship_WithForwardSpec_DeterminesWithoutQuery)
    {
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassE");
    ECRelationshipClassCP classDHasE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();

    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDHasE, *d, *e);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classD->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection::RequiredRelationDirection_Forward, "", classDHasE->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_EQ(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithForward0To1Relationship_WithBackwardSpec_DeterminesWithQuery)
    {
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassE");
    ECRelationshipClassCP classDHasE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();

    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDHasE, *d, *e);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classE->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection::RequiredRelationDirection_Backward, "", classDHasE->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_LT(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithForward0To1Relationship_WithBothDirectionsSpec_DeterminesWithoutQuery)
    {
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassE");
    ECRelationshipClassCP classDHasE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();

    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDHasE, *d, *e);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classD->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection::RequiredRelationDirection_Both, "", classDHasE->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_EQ(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithSkipRelatedLevels_WithSomeRelationshipsEnding0_DeterminesWithQuery)
    {
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassE");
    ECRelationshipClassCP classDHasE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();

    IECInstancePtr d1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDHasE, *d1, *e);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classD->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 1, "",
        RequiredRelationDirection::RequiredRelationDirection_Both, "", classDHasE->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_LT(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithForward1To1Relationship_WithForwardSpec_DeterminesWithoutQuery)
    {
    ECClassCP classA2Base = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassA2Base");
    ECClassCP classB2 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassB2");
    ECRelationshipClassCP classA2BaseHasB2 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassA2BaseHasB2")->GetRelationshipClassCP();

    IECInstancePtr a2Base = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA2Base);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classA2BaseHasB2, *a2Base, *b2);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA2Base->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection::RequiredRelationDirection_Forward, "", classA2BaseHasB2->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_EQ(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithForward1To1Relationship_WithBackwardSpec_DeterminesWithoutQuery)
    {
    ECClassCP classA2Base = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassA2Base");
    ECClassCP classB2 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassB2");
    ECRelationshipClassCP classA2BaseHasB2 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassA2BaseHasB2")->GetRelationshipClassCP();

    IECInstancePtr a2Base = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA2Base);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classA2BaseHasB2, *a2Base, *b2);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classB2->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);

    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection::RequiredRelationDirection_Backward, "", classA2BaseHasB2->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_EQ(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithForward1To1Relationship_WithBothDirectionsSpec_DeterminesWithoutQuery)
    {
    ECClassCP classA2Base = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassA2Base");
    ECClassCP classB2 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassB2");
    ECRelationshipClassCP classA2BaseHasB2 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassA2BaseHasB2")->GetRelationshipClassCP();

    IECInstancePtr a2Base = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA2Base);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classA2BaseHasB2, *a2Base, *b2);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA2Base->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection::RequiredRelationDirection_Both, "", classA2BaseHasB2->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_EQ(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithBackward0To0Relationship_WithForwardSpec_DeterminesWithQuery)
    {
    ECClassCP classG = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassG");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECRelationshipClassCP classGHasD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassGUsesClassD")->GetRelationshipClassCP();

    IECInstancePtr g = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classG);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classGHasD, *d, *g);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classG->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection::RequiredRelationDirection_Forward, "", classGHasD->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_LT(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithBackward0To0Relationship_WithBackwardSpec_DeterminesWithQuery)
    {
    ECClassCP classG = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassG");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECRelationshipClassCP classGHasD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassGUsesClassD")->GetRelationshipClassCP();

    IECInstancePtr g = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classG);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classGHasD, *d, *g);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classD->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection::RequiredRelationDirection_Backward, "", classGHasD->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_LT(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_WithBackward0To0Relationship_WithBothDirectionsSpec_DeterminesWithQuery)
    {
    ECClassCP classG = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassG");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECRelationshipClassCP classGHasD = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassGUsesClassD")->GetRelationshipClassCP();

    IECInstancePtr g = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classG);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classGHasD, *d, *g);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classG->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection::RequiredRelationDirection_Both, "", classGHasD->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_LT(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, DeterminesIfNodeHasChildrenByRelationship_AlwaysDeterminesWithQueryIfInstanceFilterIsSet)
    {
    ECClassCP classA2Base = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassA2Base");
    ECClassCP classB2 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassB2");
    ECRelationshipClassCP classA2BaseHasB2 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassA2BaseHasB2")->GetRelationshipClassCP();

    IECInstancePtr a2Base = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA2Base);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classA2BaseHasB2, *a2Base, *b2);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA2Base->GetFullName(), false));
    m_ruleset->AddPresentationRule(*rule);
    ChildNodeRuleP childRule = new ChildNodeRule("", 0, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(0, ChildrenHint::Unknown, false, false, false, false, 0, "TRUE",
        RequiredRelationDirection::RequiredRelationDirection_Both, "", classA2BaseHasB2->GetFullName(), ""));
    m_ruleset->AddPresentationRule(*childRule);

    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*m_context);
    m_context->SetRootNodeContext(rule);

    auto provider = PostProcess(*QueryBasedSpecificationNodesProvider::Create(*m_context, *rule->GetSpecifications().front()));
    auto iter = provider->begin();
    ASSERT_TRUE(iter != provider->end());
    EXPECT_TRUE((*iter)->HasChildren());
    EXPECT_LT(NUM_CACHED_QUERIES_WHEN_CHILDREN_DETERMINED_WITHOUT_RUNNING_QUERY, m_connection->GetStatementCache().Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, GetArtifacts_DoesntRequestArtifactsFromChildProvidersWhenNoArtifactsRuleAppliesForSpecification)
    {
    auto captureArtifacts = ArtifactsCapturer::Create();
    m_context->AddArtifactsCapturer(captureArtifacts.get());

    RootNodeRule* rule1 = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule1);
    m_context->SetRootNodeContext(rule1);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "ECInstanceId = 1", "RulesEngineTest:Widget", false);
    rule1->AddSpecification(*spec);

    RootNodeRule* rule2 = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule2);
    rule2->AddCustomizationRule(*new NodeArtifactsRule());

    RefCountedPtr<QueryBasedSpecificationNodesProvider> provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    provider->GetNodesCount();
    EXPECT_TRUE(captureArtifacts->GetArtifacts().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, GetArtifacts_RequestsArtifactsFromChildProvidersWhenArtifactsDefinedAtRulesetRootLevel)
    {
    auto captureArtifacts = ArtifactsCapturer::Create();
    m_context->AddArtifactsCapturer(captureArtifacts.get());

    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "ECInstanceId = 1", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    bmap<Utf8String, Utf8String> artifactDefinitions;
    artifactDefinitions.Insert("test", "123");
    m_ruleset->AddPresentationRule(*new NodeArtifactsRule("", artifactDefinitions));

    RefCountedPtr<QueryBasedSpecificationNodesProvider> provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    provider->GetNodesCount();
    EXPECT_EQ(1, captureArtifacts->GetArtifacts().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, GetArtifacts_RequestsArtifactsFromChildProvidersWhenArtifactsDefinedAtNestedRuleLevel)
    {
    auto captureArtifacts = ArtifactsCapturer::Create();
    m_context->AddArtifactsCapturer(captureArtifacts.get());

    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "ECInstanceId = 1", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    bmap<Utf8String, Utf8String> artifactDefinitions;
    artifactDefinitions.Insert("test", "123");
    rule->AddCustomizationRule(*new NodeArtifactsRule("", artifactDefinitions));

    RefCountedPtr<QueryBasedSpecificationNodesProvider> provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    provider->GetNodesCount();
    EXPECT_EQ(1, captureArtifacts->GetArtifacts().size());
    }
