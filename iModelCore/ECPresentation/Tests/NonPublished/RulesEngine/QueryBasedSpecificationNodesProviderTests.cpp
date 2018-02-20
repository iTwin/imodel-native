/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/QueryBasedSpecificationNodesProviderTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "NodesProviderTests.h"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
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
* @bsimethod                                    Grigas.Petraitis                07/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, OverridesLabel)
    {
    LabelOverrideP widgetLabelOverride = new LabelOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Widget\"", 1, "\"WidgetLabel\"", "");
    LabelOverrideP gadgetLabelOverride = new LabelOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Gadget\"", 1, "\"GadgetLabel\"", "");
    m_ruleset->AddPresentationRule(*widgetLabelOverride);
    m_ruleset->AddPresentationRule(*gadgetLabelOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "this.ECInstanceId = 1 OR this.ECInstanceId = 4", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);    

    size_t index = 0;
    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    ASSERT_TRUE(provider->GetNode(node, index++));
    ASSERT_TRUE(nullptr != node->GetKey()->AsECInstanceNodeKey());
    EXPECT_EQ(m_gadgetClass->GetId(), node->GetKey()->AsECInstanceNodeKey()->GetECClassId());
    EXPECT_STREQ("GadgetLabel", node->GetLabel().c_str());

    ASSERT_TRUE(provider->GetNode(node, index++));
    ASSERT_TRUE(nullptr != node->GetKey()->AsECInstanceNodeKey());
    EXPECT_EQ(m_widgetClass->GetId(), node->GetKey()->AsECInstanceNodeKey()->GetECClassId());
    EXPECT_STREQ("WidgetLabel", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, UsesRelatedInstancePropertiesWhenOverridingLabel)
    {
    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsInstanceNode AND widget.BoolProperty", 1, "widget.MyID", "");
    m_ruleset->AddPresentationRule(*labelOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rule->AddSpecification(*spec);    

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    ASSERT_EQ(3, provider->GetNodesCount());

    size_t index = 0;
    JsonNavNodePtr node;
    while (provider->GetNode(node, index++))
        {
        EXPECT_STREQ("Test Widget 1", node->GetLabel().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
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
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "this.ECInstanceId = 1 OR this.ECInstanceId = 4", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec); 

    size_t index = 0;
    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    
    ASSERT_TRUE(provider->GetNode(node, index++));
    ASSERT_TRUE(nullptr != node->GetKey()->AsECInstanceNodeKey());
    EXPECT_EQ(m_gadgetClass->GetId(), node->GetKey()->AsECInstanceNodeKey()->GetECClassId());
    EXPECT_STREQ("", node->GetForeColor().c_str());
    EXPECT_STREQ("", node->GetBackColor().c_str());
    EXPECT_STREQ("Regular", node->GetFontStyle().c_str());
    
    ASSERT_TRUE(provider->GetNode(node, index++));
    ASSERT_TRUE(nullptr != node->GetKey()->AsECInstanceNodeKey());
    EXPECT_EQ(m_widgetClass->GetId(), node->GetKey()->AsECInstanceNodeKey()->GetECClassId());
    EXPECT_STREQ("ForeColor1", node->GetForeColor().c_str());
    EXPECT_STREQ("BackColor1", node->GetBackColor().c_str());
    EXPECT_STREQ("FontStyle1", node->GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, ReturnsChildrenIfHideNodesInHierarchyFlagIsSpecified)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);

    ChildNodeRule* childRule = new ChildNodeRule("", 1, false, TargetTree_Both);
    spec->AddNestedRule(*childRule);

    CustomNodeSpecification* customNodeSpec = new CustomNodeSpecification(1, false, "Custom", "Custom", "", "Custom");
    childRule->AddSpecification(*customNodeSpec);

    size_t index = 0;
    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    while (provider->GetNode(node, index++))
        ASSERT_STREQ("Custom", node->GetType().c_str());
    ASSERT_TRUE(index > 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, ReturnsChildIfDisplayLabelGroupingNodeHasOnlyOne)
    {
    // make sure there's only one Widget
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    size_t index = 0;
    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    while (provider->GetNode(node, index++))
        {
        ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        ASSERT_STREQ("MyLabel", node->GetLabel().c_str());
        }
    ASSERT_EQ(2, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, ReturnsChildIfGroupingRuleDoesntHaveCreateGroupForSingleItemFlag)
    {
    // make sure there's only one Widget
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("MyLabel"));});

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    GroupingRule* groupingRule = new GroupingRule("", 1000, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", false, "", ""));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    size_t index = 0;
    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    while (provider->GetNode(node, index++))
        {
        ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        ASSERT_STREQ("MyLabel", node->GetLabel().c_str());
        }
    ASSERT_EQ(2, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, HasNodesReturnsFalseWhenQueryReturnsNoResults)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    EXPECT_FALSE(provider->HasNodes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, HasNodesReturnsTrueWhenQueryReturnsResults)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    EXPECT_TRUE(provider->HasNodes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedSpecificationNodesProviderTests, NodesCount_SimpleCase)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

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
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NodesCount_WithHideNodesInHierarchyFlag)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

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
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NodesCount_WithDisplayLabelGroupingNodeHavingOnlyOneChild)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

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
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NodesCount_WithDisplayLabelAndSameLabelInstanceGrouping)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

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

    JsonNavNodePtr myLabelNode;
    ASSERT_TRUE(provider->GetNode(myLabelNode, 0));
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, myLabelNode->GetType().c_str()); // the node is ECInstanceNode because SameLabelInstanceGroup groups all instances into one - nothing is left to group for display label node
    ASSERT_STREQ("MyLabel", myLabelNode->GetLabel().c_str());
    
    JsonNavNodePtr otherLabelNode;
    ASSERT_TRUE(provider->GetNode(otherLabelNode, 1));
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, otherLabelNode->GetType().c_str());
    ASSERT_STREQ("OtherLabel", otherLabelNode->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NodesCount_WithGroupingRuleNotHavingCreateGroupForSingleItemFlag)
    {
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

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
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, CustomizesNodes)
    {
    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Widget\"", 1, "\"QueryBasedSpecificationNodesProviderTests.CustomizesNodes\"", "");
    m_ruleset->AddPresentationRule(*labelOverride);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget,Gadget", false);
    rule->AddSpecification(*spec);    

    size_t index = 0;
    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    ASSERT_TRUE(0 < provider->GetNodesCount());
    while (provider->GetNode(node, index++))
        {
        NavNodeExtendedData extendedData(*node);
        EXPECT_TRUE(extendedData.IsCustomized());
        }
    ASSERT_TRUE(index > 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
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
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsInstanceNode", 1000, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    m_ruleset->AddPresentationRule(*childRule);

    size_t index = 0;
    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    ASSERT_TRUE(provider->HasNodes());
    ASSERT_EQ(2, provider->GetNodesCount());
    while (provider->GetNode(node, index++))
        {
        ECInstanceNodeKey const* key = node->GetKey()->AsECInstanceNodeKey();
        ASSERT_TRUE(nullptr != key);
        EXPECT_EQ(m_gadgetClass->GetId(), key->GetECClassId());
        }
    ASSERT_EQ(3, index);
    }

#ifdef wip_postprocess
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, ReturnsChildNodesWhenTheresOnlyOneLabelGroupingNode)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(spec);

    size_t index = 0;
    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    ASSERT_EQ(2, provider->GetNodesCount());
    while (provider->GetNode(node, index++))
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        EXPECT_STREQ("Widget", node->GetInstance()->GetClass().GetName().c_str());
        }
    ASSERT_EQ(3, index);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
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
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true);
    rule->AddSpecification(*spec);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassF", "", "", "");
    m_ruleset->AddPresentationRule(*groupingRule);

    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    groupingRule->AddGroup(*propertyGroup);

    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    // 2 property grouping nodes of ClassF, 2 ECInstance nodes of ClassE and 1 ECInstance node of ClassG
    ASSERT_EQ(5, provider->GetNodesCount());

    ASSERT_TRUE(provider->GetNode(node, 0));
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node->GetType().c_str());
    ASSERT_STREQ("1", node->GetLabel().c_str());

    ASSERT_TRUE(provider->GetNode(node, 1));
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node->GetType().c_str());
    ASSERT_STREQ("2", node->GetLabel().c_str());
    
    ASSERT_TRUE(provider->GetNode(node, 2));
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
    
    ASSERT_TRUE(provider->GetNode(node, 3));
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
    
    ASSERT_TRUE(provider->GetNode(node, 4));
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, PropertyGrouping_DoesntGroupNullValuesIfSpecified)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
        
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    m_ruleset->AddPresentationRule(*groupingRule);

    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    propertyGroup->SetCreateGroupForUnspecifiedValues(false);
    groupingRule->AddGroup(*propertyGroup);

    JsonNavNodePtr node;
    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);

    // expect 1 property grouping and 3 instance nodes (not grouped, because IntProperty values are null)
    ASSERT_EQ(4, provider->GetNodesCount());

    ASSERT_TRUE(provider->GetNode(node, 0));
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node->GetType().c_str());
    ASSERT_STREQ("5", node->GetLabel().c_str());

    ASSERT_TRUE(provider->GetNode(node, 1));
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
    ASSERT_STREQ("Test Widget 1", node->GetLabel().c_str());

    ASSERT_TRUE(provider->GetNode(node, 2));
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
    ASSERT_STREQ("Test Widget 2", node->GetLabel().c_str());

    ASSERT_TRUE(provider->GetNode(node, 3));
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
    ASSERT_STREQ("Test Widget 3", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedSpecificationNodesProviderTests, NotifiesAboutUsedRelatedClassesInCustomizationRuleValue)
    {    
    RootNodeRule* rule = new RootNodeRule("", 1000, false, TargetTree_Both, false);
    m_ruleset->AddPresentationRule(*rule);
    m_context->SetRootNodeContext(*rule);

    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*spec);

    m_ruleset->AddPresentationRule(*new LabelOverride("", 1, "this.GetRelatedValue(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\", \"MyID\")", ""));

    NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, *spec);
    ASSERT_NE(0, provider->GetNodesCount());
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
