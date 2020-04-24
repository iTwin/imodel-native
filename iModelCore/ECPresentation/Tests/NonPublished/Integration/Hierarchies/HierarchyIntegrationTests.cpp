/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../PresentationManagerIntegrationTests.h"
#include "../../../../Source/RulesDriven/RulesEngine/LocalizationHelper.h"
#include "../../RulesEngine/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerNavigationTests : PresentationManagerIntegrationTests
{
    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;
    ECSchemaCP m_schema;

    void _ConfigureManagerParams(RulesDrivenECPresentationManager::Params& params) override
        {
        PresentationManagerIntegrationTests::_ConfigureManagerParams(params);
        params.SetMode(RulesDrivenECPresentationManager::Mode::ReadOnly);
        }

    void SetUp() override
        {
        PresentationManagerIntegrationTests::SetUp();
        m_schema = s_project->GetECDb().Schemas().GetSchema("RulesEngineTest");
        ASSERT_TRUE(nullptr != m_schema);

        m_widgetClass = m_schema->GetClassCP("Widget");
        m_gadgetClass = m_schema->GetClassCP("Gadget");
        m_sprocketClass = m_schema->GetClassCP("Sprocket");
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static CustomNodeSpecificationP CreateCustomNodeSpecification(Utf8String typeAndLabel, std::function<void(CustomNodeSpecificationR)> configure = nullptr)
    {
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, false, typeAndLabel, typeAndLabel, typeAndLabel, typeAndLabel);
    spec->SetHasChildren(ChildrenHint::Unknown);
    if (nullptr != configure)
        configure(*spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void VerifyNodeInstances(NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& instances)
    {
    bvector<ECInstanceKey> nodeInstanceKeys = NavNodeExtendedData(node).GetInstanceKeys();
    ASSERT_EQ(instances.size(), nodeInstanceKeys.size());
    for (size_t i = 0; i < instances.size(); ++i)
        {
        EXPECT_EQ(instances[i]->GetClass().GetId(), nodeInstanceKeys[i].GetClassId());
        EXPECT_STREQ(instances[i]->GetInstanceId().c_str(), nodeInstanceKeys[i].GetInstanceId().ToString().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void VerifyNodeInstance(NavNodeCR node, IECInstanceCR instance) {VerifyNodeInstances(node, {&instance});}

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, NavigationOptions_GetRuleSetId)
    {
    RulesDrivenECPresentationManager::NavigationOptions options("test id");
    ASSERT_STREQ("test id", options.GetRulesetId());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_NotGrouped)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });

    // expect two nodes
    ASSERT_EQ(2, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    VerifyNodeInstance(*nodes[0], *gadgetInstance);

    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[1]->GetType().c_str());
    VerifyNodeInstance(*nodes[1], *widgetInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByClass)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_EQ(m_gadgetClass->GetId(), nodes[0]->GetKey()->AsECClassGroupingNodeKey()->GetECClassId());
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, gadgetNodes[0]->GetType().c_str());
    ASSERT_EQ(nodes[0]->GetNodeId(), gadgetNodes[0]->GetParentNodeId());
    VerifyNodeInstance(*gadgetNodes[0], *gadgetInstance);

    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[1]->GetType().c_str());
    EXPECT_EQ(m_widgetClass->GetId(), nodes[1]->GetKey()->AsECClassGroupingNodeKey()->GetECClassId());
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[1], PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetNodes[0]->GetType().c_str());
    ASSERT_EQ(nodes[1]->GetNodeId(), widgetNodes[0]->GetParentNodeId());
    VerifyNodeInstance(*widgetNodes[0], *widgetInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_AlwaysReturnsChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr node = nodes[0];
    EXPECT_EQ(ChildrenHint::Always, NavNodeExtendedData(*node).GetChildrenHint());
    EXPECT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_DoNotSort_ReturnsUnsortedNodes)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget2", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget1", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, true, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, true, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    allInstanceNodesSpecification->AddNestedRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideNodesInHierarchy)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, true, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    allInstanceNodesSpecification->AddNestedRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    ASSERT_STREQ("test", instanceNode->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideExpression_ReturnsEmptyListIfExpressionEvalutesToTrue)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP spec = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    spec->SetHideExpression("ThisNode.IsOfClass(\"Widget\", \"RulesEngineTest\")");
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideExpression_ReturnsNodesIfExpressionEvaluatesToFalse)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP spec = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    spec->SetHideExpression("ThisNode.IsOfClass(\"Gadget\", \"RulesEngineTest\")");
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByLabel_DoesntGroup1Instance)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByLabelGroups3InstancesWith1GroupingNode)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("GadgetID", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    NavNodeCPtr labelGroupingNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure we have 2 widget instances
    DataContainer<NavNodeCPtr> instanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get(); });
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByLabelGroups4InstancesWith2GroupingNodes)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> labelGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, labelGroupingNodes.GetSize());

    NavNodeCPtr labelGroupingNode = labelGroupingNodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("GadgetID", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 gadget instances
    DataContainer<NavNodeCPtr> instanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get(); });
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());

    labelGroupingNode = labelGroupingNodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 widget instances
    instanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get(); });
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, RemovesLabelGroupingNodeIfOnlyOneChild)
    {
    // make sure there are instances with unique labels and instances with same labels
    IECInstancePtr instanceWithUniqueLabel = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Unique Label")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Same Label")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Same Label")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Same Label")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });

    // make sure we have one instance node and one display label grouping node
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr labelGroupingNode = nodes[0];
    NavNodeCPtr instanceNode = nodes[1];

    ASSERT_TRUE(nullptr != instanceNode->GetKey()->AsECInstanceNodeKey());
    VerifyNodeInstance(*instanceNode, *instanceWithUniqueLabel);

    EXPECT_TRUE(labelGroupingNode->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    EXPECT_TRUE(labelGroupingNode->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, AlwaysReturnsResultsFlag_SetToNodeFromSpecification)
    {
    // insert a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });

    // make sure the parent node has the "always returns results" flag
    ASSERT_EQ(2, nodes.GetSize());
    EXPECT_EQ(ChildrenHint::Always, NavNodeExtendedData(*nodes[0]).GetChildrenHint());
    EXPECT_EQ(ChildrenHint::Unknown, NavNodeExtendedData(*nodes[1]).GetChildrenHint());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    childRule->SetCondition("ParentNode.IsInstanceNode And ParentNode.ClassName = \"Widget\"");
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });

    // make sure the node was hidden
    ASSERT_TRUE(0 == nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);


    // insert a gadget
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    childRule->SetCondition("ParentNode.IsInstanceNode And ParentNode.ClassName = \"Widget\"");
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });

    // make sure the node was not hidden
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideIfNoChildren_IgnoredIfHasAlwaysReturnsNodesFlag)
    {
    // insert a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, true, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });

    // make sure the node was not hidden
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideNodesInHierarchy_ReturnsChildNodes)
    {
    // add a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // add a gadget
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    childRule->SetCondition("ParentNode.IsInstanceNode");
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });

    // make sure we get gadget, not widget
    ASSERT_EQ(1, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *gadgetInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideNodesInHierarchy_ReturnsNoChildrenWhenThereAreNoChildSpecifications)
    {
    // add a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // add a gadget
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    childRule->SetCondition("ParentNode.IsInstanceNode AND ParentNode.ClassName=\"Widget\"");
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });

    // expect 1 widget node
    ASSERT_EQ(1, rootNodes.GetSize());

    // request for child nodes
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get(); });

    // expect empty container
    ASSERT_EQ(0, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsSpecifiedNumberOfNodes_LabelOverride)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(2), options.GetJson()).get(); });

    // expect 3 nodes: C, D, E
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("C", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("D", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("E", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsSpecifiedNumberOfNodes)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Paging_SkipsSpecifiedNumberOfNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(2), options.GetJson()).get(); });

    // expect 3 nodes: C, D, E
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("C", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("D", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("E", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkippingMoreThanExists)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(5), options.GetJson()).get(); });

    // expect 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    ASSERT_TRUE(nodes[0].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_ReturnsSpecifiedNumberOfNodes)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(0, 2), options.GetJson()).get(); });

    // expect 2 nodes: A, B
    ASSERT_EQ(2, nodes.GetSize());
    EXPECT_STREQ("A", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("B", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_PageSizeHigherThanTheNumberOfNodes)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(0, 5), options.GetJson()).get(); });

    // expect 2 nodes: A, B
    ASSERT_EQ(2, nodes.GetSize());
    EXPECT_STREQ("A", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("B", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_IndexHigherThanPageSize)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(0, 2), options.GetJson()).get(); });

    // expect nullptr
    EXPECT_TRUE(nodes[2].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsAndReturnsSpecifiedNumberOfNodes)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str());
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(1, 3), options.GetJson()).get(); });

    // expect 3 nodes: B, C, D
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("B", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("C", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("D", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_Type_Label_Description_ImageId)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "label", "description", "imageid");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    ASSERT_STREQ("type", instanceNode->GetType().c_str());
    ASSERT_STREQ("label", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("description", instanceNode->GetDescription().c_str());
    ASSERT_STREQ("imageid", instanceNode->GetCollapsedImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("a"));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*CreateCustomNodeSpecification("b"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("type", [](CustomNodeSpecificationR spec) { spec.SetHideIfNoChildren(true); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideNodesInHierarchy_ReturnsEmptyList)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("type", [](CustomNodeSpecificationR spec) {spec.SetHideNodesInHierarchy(true); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideNodesInHierarchy_ReturnsEmptyListForRootNode)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("type", [](CustomNodeSpecificationR spec) {spec.SetHideNodesInHierarchy(true); }));
    rules->AddPresentationRule(*rootRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideExpression_ReturnsEmptyListIfExpressionEvalutesToTrue)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("T_MyType", [](CustomNodeSpecificationR spec) {spec.SetHideExpression("ThisNode.Type = \"T_MyType\""); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideExpression_ReturnsNodesIfExpressionEvaluatesToFalse)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("T_MyType", [](CustomNodeSpecificationR spec) {spec.SetHideExpression("ThisNode.Type = \"T_OtherType\""); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_AlwaysReturnsChildren_NodeHasChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("type", [](CustomNodeSpecificationR spec) {spec.SetHasChildren(ChildrenHint::Always); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    ASSERT_EQ(1, m_manager->GetRootNodesCount(s_project->GetECDb(), options).get());

    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr node = nodes[0];
    EXPECT_TRUE(node->HasChildren());

    rapidjson::Document nodeJson = node->AsJson();
    ASSERT_TRUE(nodeJson.HasMember(NAVNODE_HasChildren));
    ASSERT_TRUE(nodeJson[NAVNODE_HasChildren].GetBool());
    }

/*---------------------------------------------------------------------------------**//**
* Note: In this test we initialize data providers for the first 2 specs when determining
* root node's children. The first spec returns 0 nodes and the second one returns 1 node.
* Overall the root node should have 2 children - 2'nd and 3'rd.
* @betest                                       Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_ReturnsCorrectChildNodesWhenOneOfThemIsHiddenAndCombinedHierarchyLevelIsNotInitialized)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"root\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*CreateCustomNodeSpecification("child1", [](CustomNodeSpecificationR spec) {spec.SetHideNodesInHierarchy(true); }));
    childRule->AddSpecification(*CreateCustomNodeSpecification("child2"));
    childRule->AddSpecification(*CreateCustomNodeSpecification("child3"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("child2", childNodes[0]->GetType().c_str());
    EXPECT_STREQ("child3", childNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Recursion prevention (VSTS#102711)
* @betest                                       Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_DoesNotReturnNodesOfTheSameSpecificationAlreadyExistingInHierarchy)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("T_ROOT"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*CreateCustomNodeSpecification("T_CHILD"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 T_ROOT node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(BeTest::GetNameOfCurrentTest()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("T_ROOT", rootNodes[0]->GetType().c_str());

    // T_ROOT node should have 1 T_CHILD node and it should have no children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("T_CHILD", childNodes[0]->GetType().c_str());
    EXPECT_FALSE(childNodes[0]->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_AlwaysReturnsChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr node = nodes[0];
    EXPECT_EQ(ChildrenHint::Always, NavNodeExtendedData(*node).GetChildrenHint());
    EXPECT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_HideNodesInHierarchy)
    {
    // insert some widget instance
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    ASSERT_STREQ("test", instanceNode->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert widget instance
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert widget instance
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByClass)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByLabel_DoesntGroup1Instance)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByLabel_Groups3InstancesWith1GroupingNode)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget instance node
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("GadgetID", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 widget label grouping node
    NavNodeCPtr labelGroupingNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure we have 2 widget instances
    DataContainer<NavNodeCPtr> instanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get(); });
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByLabel_Groups4InstancesWith2GroupingNodes)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> labelGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, labelGroupingNodes.GetSize());

    // make sure we have 2 gadget instances in gadget grouping node
    NavNodeCPtr labelGroupingNode = labelGroupingNodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("GadgetID", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());
    DataContainer<NavNodeCPtr> instanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get(); });
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());

    // make sure we have 2 widget instances in widget grouping node
    labelGroupingNode = labelGroupingNodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());
    instanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get(); });
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByClassAndByLabel)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget1")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget1")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget2")); });

    // insert gadget instance
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, true, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 1 gadget instance node
    DataContainer<NavNodeCPtr> gadgetInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, gadgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, gadgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*gadget).c_str(), gadgetInstanceNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 widget nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(2, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, widgetNodes[0]->GetType().c_str());

    // make sure we have 2 Widget1 instance nodes
    DataContainer<NavNodeCPtr> widget1Nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widgetNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, widget1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widget1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget1", widget1Nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 Widget2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetNodes[1]->GetType().c_str());
    EXPECT_STREQ("Widget2", widgetNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByClassAndByLabel_InstanceLabelOverride)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget1")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget1")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget2")); });

    // insert gadget instance
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceNodesOfSpecificClasses_GroupedByClassAndByLabel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, true, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 1 gadget instance node
    DataContainer<NavNodeCPtr> gadgetInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, gadgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, gadgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), gadgetInstanceNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 widget nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(2, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, widgetNodes[0]->GetType().c_str());

    // make sure we have 2 Widget1 instance nodes
    DataContainer<NavNodeCPtr> widget1Nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widgetNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, widget1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widget1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget1", widget1Nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 Widget2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetNodes[1]->GetType().c_str());
    EXPECT_STREQ("Widget2", widgetNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_DoNotSort_ReturnsUnsortedNodes)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    instanceNodesOfSpecificClassesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget2", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget1", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_ArePolymorphic)
    {
    // insert some ClassE & ClassF instances
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    IECInstancePtr classEInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr classFInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_AreNotPolymorphic)
    {
    // insert some ClassE & ClassF instances
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    IECInstancePtr classEInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr classFInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_InstanceFilter)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(10)); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(5)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "this.IntProperty<=5", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* Note: In this test we initialize data providers for the first 2 specs when determining
* root node's children. The first spec returns 0 nodes and the second one returns 1 node.
* Overall the root node should have 2 children - 2'nd and 3'rd.
* @betest                                       Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_ReturnsCorrectChildNodesWhenOneOfThemIsHiddenAndCombinedHierarchyLevelIsNotInitialized)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"root\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false,
        true, false, false, false, "", m_widgetClass->GetFullName(), false));
    childRule->AddSpecification(*CreateCustomNodeSpecification("child2"));
    childRule->AddSpecification(*CreateCustomNodeSpecification("child3"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("child2", childNodes[0]->GetType().c_str());
    EXPECT_STREQ("child3", childNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Note: In this test we initialize data providers for the first spec when determining
* root node's children. The first spec returns its child nodes and the second one returns
* 1 node. Overall the root node should have 2 children - 2'nd and 3'rd.
* @betest                                       Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_ReturnsCorrectChildNodesWhenOneOfThemReturnsItsChildrenAndCombinedHierarchyLevelIsNotInitialized)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"root\"", 1, false, TargetTree_Both);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true,
        false, false, false, false, "", m_widgetClass->GetFullName(), false));
    childRule1->AddSpecification(*CreateCustomNodeSpecification("child3"));
    rules->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.IsInstanceNode", 1, false, TargetTree_Both);
    childRule2->AddSpecification(*CreateCustomNodeSpecification("child2"));
    rules->AddPresentationRule(*childRule2);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("child2", childNodes[0]->GetType().c_str());
    EXPECT_STREQ("child3", childNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_AlwaysReturnsChildren)
    {
    // insert widget & gadget instances with relationship
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, true, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    childRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());

    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get(); });

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    NavNodeCPtr node = gadgetNodes[0];
    EXPECT_EQ(ChildrenHint::Always, NavNodeExtendedData(*node).GetChildrenHint());
    EXPECT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_HideNodesInHierarchy)
    {
    // insert widget & gadget instances with relationship
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, true, false, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    ChildNodeRule* customNodeRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    customNodeRule->AddSpecification(*customNodeSpecification);
    relatedInstanceNodesSpecification->AddNestedRule(*customNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

     // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());

    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> customNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get(); });

    // make sure we have 1 custom node
    ASSERT_EQ(1, customNodes.GetSize());

    NavNodeCPtr instanceNode = customNodes[0];
    ASSERT_STREQ("test", instanceNode->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert widget & gadget instances with relationship
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, true, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());

    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get(); });

    // make sure we have 0 gadget nodes
    ASSERT_EQ(0, gadgetNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert widget & gadget instances with relationship
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, true, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    ChildNodeRule* customNodeRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    customNodeRule->AddSpecification(*customNodeSpecification);
    relatedInstanceNodesSpecification->AddNestedRule(*customNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get(); });

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByClass)
    {
    // insert widget & gadget instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get(); });

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_GroupedByClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_GroupedByClass)
    {
    // set up the dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP rel = GetClass("A_Has_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *d);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false, 
        "", { new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)) }));
    rules->AddPresentationRule(*childRule);

    // verify
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    EXPECT_STREQ("C", classGroupingNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[1]->GetType().c_str());
    EXPECT_STREQ("D", classGroupingNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *d);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_ReturnsRelatedChildrenBasedOnSameSpecificationAndGroupedByClass, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_A" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_ReturnsRelatedChildrenBasedOnSameSpecificationAndGroupedByClass)
    {
    // set up the dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP rel = GetClass("A_Has_A")->GetRelationshipClassCP();

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *b, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *c, *d);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childrenRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", rel->GetFullName(), classA->GetFullName()));
    rules->AddPresentationRule(*childrenRule);

    // request & validate
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b);
    VerifyNodeInstance(*rootNodes[1], *c);
    VerifyNodeInstance(*rootNodes[2], *d);

    DataContainer<NavNodeCPtr> classGroupedChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, classGroupedChildNodes.GetSize());
    EXPECT_STREQ("C", classGroupedChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupedChildNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childInstanceNodes.GetSize());
    VerifyNodeInstance(*childInstanceNodes[0], *c);

    DataContainer<NavNodeCPtr> classGroupedGrandChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *childInstanceNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, classGroupedGrandChildNodes.GetSize());
    EXPECT_STREQ("D", classGroupedGrandChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> grandChildInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupedGrandChildNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, grandChildInstanceNodes.GetSize());
    VerifyNodeInstance(*grandChildInstanceNodes[0], *d);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsRelatedChildrenBasedOnSameSpecificationAndGroupedByClass, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_A" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsRelatedChildrenBasedOnSameSpecificationAndGroupedByClass)
    {
    // set up the dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP rel = GetClass("A_Has_A")->GetRelationshipClassCP();

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *b, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *c, *d);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childrenRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "", {
        new RepeatableRelationshipPathSpecification({new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, classA->GetFullName())})
        }));
    rules->AddPresentationRule(*childrenRule);

    // request & validate
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b);
    VerifyNodeInstance(*rootNodes[1], *c);
    VerifyNodeInstance(*rootNodes[2], *d);

    DataContainer<NavNodeCPtr> classGroupedChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, classGroupedChildNodes.GetSize());
    EXPECT_STREQ("C", classGroupedChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupedChildNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childInstanceNodes.GetSize());
    VerifyNodeInstance(*childInstanceNodes[0], *c);

    DataContainer<NavNodeCPtr> classGroupedGrandChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *childInstanceNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, classGroupedGrandChildNodes.GetSize());
    EXPECT_STREQ("D", classGroupedGrandChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> grandChildInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupedGrandChildNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, grandChildInstanceNodes.GetSize());
    VerifyNodeInstance(*grandChildInstanceNodes[0], *d);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByLabel_DoesntGroup1Instance)
    {
    // insert widget & gadget instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, true, false, 0, "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ("GadgetID", gadgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get(); });

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetNodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", widgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode)
    {
    // insert widget, gadget & sprocket instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("SprocketID"));});
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("SprocketID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Sprocket", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, true, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "RulesEngineTest:Widget,Sprocket");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ(GetDefaultDisplayLabel(*gadgetInstance).c_str(), gadgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 2 sprocket nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[0]->GetType().c_str());
    DataContainer<NavNodeCPtr> sprocketNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, sprocketNodes.GetSize());

    // make sure we have 1 widget node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_DoNotSort_ReturnsUnsortedNodes)
    {
    // insert some gadget & sprocket instances with relationships
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Sprocket", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "RulesEngineTest:Widget,Sprocket");
    relatedInstanceNodesSpecification->SetDoNotSort(true);
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ(GetDefaultDisplayLabel(*gadgetInstance).c_str(), gadgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> sprocketNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get(); });

    // make sure we have 2 sprocket nodes
    ASSERT_EQ(2, sprocketNodes.GetSize());

    ASSERT_STREQ("Sprocket2", sprocketNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Sprocket1", sprocketNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsSingleStepRelatedInstanceNodes, R"*(
    <ECEntityClass typeName="Model">
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsSingleStepRelatedInstanceNodes)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model, *element);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", modelClass->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule("ParentNode.ClassName=\"Model\"", 1, false);
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(modelContainsElementsRelationship->GetFullName(), RequiredRelationDirection_Forward),
        })}));
    rules->AddPresentationRule(*relatedNodeRule);

    // validate
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *element);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsMultiStepRelatedInstanceNodes, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsMultiStepRelatedInstanceNodes)
    {
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP elementOwnsChildElementsRelationship = GetClass("ElementOwnsChildElements")->GetRelationshipClassCP();

    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("root"));});
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *e1, *e2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *e2, *e3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *e3, *e4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.ElementName=\"root\"", elementClass->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule("ParentNode.ECInstance.ElementName=\"root\"", 1, false);
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(elementOwnsChildElementsRelationship->GetFullName(), RequiredRelationDirection_Forward, "", 3),
        })}));
    rules->AddPresentationRule(*relatedNodeRule);

    // validate
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *e4);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsMultiRelationshipSingleStepRelatedInstanceNodes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsMultiRelationshipSingleStepRelatedInstanceNodes)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1, false);
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        })}));
    rules->AddPresentationRule(*relatedNodeRule);

    // validate
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsMultiRelationshipMultiStepRelatedInstanceNodes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsMultiRelationshipMultiStepRelatedInstanceNodes)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBB = GetClass("B_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b1, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b2, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1, false);
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBB->GetFullName(), RequiredRelationDirection_Forward, "", 2),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        })}));
    rules->AddPresentationRule(*relatedNodeRule);

    // validate
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodesWithRecursiveRelationshipInFront, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodesWithRecursiveRelationshipInFront)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAA = GetClass("A_To_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Name", ECValue("a1"));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a2, *a3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a3, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Name = \"a1\"", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule("ParentNode.ECInstance.Name=\"a1\"", 1, false);
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        })}));
    rules->AddPresentationRule(*relatedNodeRule);

    // validate
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(4, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c1);
    VerifyNodeInstance(*childNodes[1], *c2);
    VerifyNodeInstance(*childNodes[2], *c3);
    VerifyNodeInstance(*childNodes[3], *c4);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodesWithRecursiveRelationshipInTheMiddle, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodesWithRecursiveRelationshipInTheMiddle)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBB = GetClass("B_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b1, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b2, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1, false);
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBB->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        })}));
    rules->AddPresentationRule(*relatedNodeRule);

    // validate
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(4, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c1);
    VerifyNodeInstance(*childNodes[1], *c2);
    VerifyNodeInstance(*childNodes[2], *c3);
    VerifyNodeInstance(*childNodes[3], *c4);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodesWithRecursiveRelationshipAtTheEnd, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodesWithRecursiveRelationshipAtTheEnd)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipCC = GetClass("C_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1, false);
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipCC->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        })}));
    rules->AddPresentationRule(*relatedNodeRule);

    // validate
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(4, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c1);
    VerifyNodeInstance(*childNodes[1], *c2);
    VerifyNodeInstance(*childNodes[2], *c3);
    VerifyNodeInstance(*childNodes[3], *c4);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_SkipRelatedLevel)
    {
    // insert widget, gadget & sprocket instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass);
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

     // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> sprocketNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get(); });

    // make sure we have 1 sprocket node
    ASSERT_EQ(1, sprocketNodes.GetSize());
    NavNodeCPtr sprocketNode = sprocketNodes[0];
    ASSERT_STREQ(GetDefaultDisplayLabel(*sprocketInstance).c_str(), sprocketNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#711486
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_SkipRelatedLevel_DoesntDuplicateNodesWhenSkippingMultipleDifferentInstancesWithTheSameEndpointInstance)
    {
    // insert widget, gadget & sprocket instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCR relationshipWidgetsHaveGadgets = *m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetsHaveGadgets, *widget2, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetsHaveGadgets, *widget2, *gadget2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.IntProperty = 1", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childrenRule = new ChildNodeRule("ParentNode.IsOfClass(\"Widget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1,
        "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

     // make sure we have 1 widget root node
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeCPtr widget1Node = rootNodes[0];
    ECInstanceId widgetId;
    ECInstanceId::FromString(widgetId, widget1->GetInstanceId().c_str());
    VerifyNodeInstance(*widget1Node, *widget1);

    // make sure we have 1 widget child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widget1Node, PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr widget2Node = childNodes[0];
    ECInstanceId widget2Id;
    ECInstanceId::FromString(widget2Id, widget2->GetInstanceId().c_str());
    VerifyNodeInstance(*widget2Node, *widget2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_InstanceFilter)
    {
    // insert some gadget & sprocket instances with relationships
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket123"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Sprocket", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "this.MyID~\"Sprocket1\"", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket");
    relatedInstanceNodesSpecification->SetDoNotSort(true);
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ(GetDefaultDisplayLabel(*gadgetInstance).c_str(), gadgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> sprocketNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get(); });

    // make sure we have 1 sprocket node
    ASSERT_EQ(1, sprocketNodes.GetSize());
    ASSERT_STREQ("Sprocket1", sprocketNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_InstanceFilter_AppliesOnlyToMatchingInstances, R"*(
    <ECEntityClass typeName="ClassA">
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="Name" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AHasB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="True">
            <Class class="ClassA"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="True">
            <Class class="ClassB"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_InstanceFilter_AppliesOnlyToMatchingInstances)
    {
    ECEntityClassCP classA = GetClass("ClassA")->GetEntityClassCP();
    ECEntityClassCP classB = GetClass("ClassB")->GetEntityClassCP();
    ECRelationshipClassCP relationship = GetClass("AHasB")->GetRelationshipClassCP();

    // insert some gadget & sprocket instances with relationships
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Name", ECValue("One")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationship, *instanceA1, *instanceB1);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Name", ECValue("Two")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationship, *instanceA2, *instanceB2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule("ParentNode.ClassName = \"ClassA\"", 1000, false, TargetTree_Both);
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "this.Name = \"One\" OR this.Name = \"Two\"", RequiredRelationDirection_Both, "", relationship->GetFullName(), classB->GetFullName()));
    rules->AddPresentationRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByLabel_GroupsByClassAndByLabel)
    {
    // insert widget, gadget & sprocket instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Sprocket\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, true, false, true, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "RulesEngineTest:Widget,Sprocket");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get(); });

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 2 sprocket nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> sprocketNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, sprocketNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, sprocketNodes[0]->GetType().c_str());

    // make sure we have 2 Sprocket1 instance nodes
    DataContainer<NavNodeCPtr> sprocket1Nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *sprocketNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, sprocket1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, sprocket1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("Sprocket1", sprocket1Nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 Sprocket2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, sprocketNodes[1]->GetType().c_str());
    EXPECT_STREQ("Sprocket2", sprocketNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 widget instance node
    DataContainer<NavNodeCPtr> widgetInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(1, widgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widgetInstance).c_str(), widgetInstanceNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByLabel_GroupsByClassAndByLabel_InstanceLabelOverride)
    {
    // insert widget, gadget & sprocket instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstanceNodes_GroupedByLabel_GroupsByClassAndByLabel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Sprocket", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, true, false, true, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "RulesEngineTest:Widget,Sprocket");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get(); });

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 2 sprocket nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> sprocketNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, sprocketNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, sprocketNodes[0]->GetType().c_str());

    // make sure we have 2 Sprocket1 instance nodes
    DataContainer<NavNodeCPtr> sprocket1Nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *sprocketNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, sprocket1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, sprocket1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("Sprocket1", sprocket1Nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 Sprocket2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, sprocketNodes[1]->GetType().c_str());
    EXPECT_STREQ("Sprocket2", sprocketNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 widget instance node
    DataContainer<NavNodeCPtr> widgetInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(1, widgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widgetInstance).c_str(), widgetInstanceNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_FindsAllMixinSubclassesAndCustomizesNodesCorrectly, R"*(
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
        <ECCustomAttributes>
            <LinkTableRelationshipMap xmlns="ECDbMap.2.0">
                <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
            </LinkTableRelationshipMap>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Classification" modifier="Sealed">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="IClassified" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.1.0">
                <AppliesToEntityClass>Element</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="IClassifiedIsClassifiedAs" modifier="None" strength="referencing">
        <BaseClass>ElementRefersToElements</BaseClass>
        <Source multiplicity="(0..*)" roleLabel="is classified as" polymorphic="true">
            <Class class="IClassified"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="classifies" polymorphic="false">
            <Class class="Classification"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="SpecificClassifiedClass">
        <BaseClass>Element</BaseClass>
        <BaseClass>IClassified</BaseClass>
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_FindsAllMixinSubclassesAndCustomizesNodesCorrectly)
    {
    ECEntityClassCP classClassification = GetClass("Classification")->GetEntityClassCP();
    ECEntityClassCP classSpecificClassifiedClass = GetClass("SpecificClassifiedClass")->GetEntityClassCP();
    ECRelationshipClassCP relationshipIClassifiedIsClassifiedAs = GetClass("IClassifiedIsClassifiedAs")->GetRelationshipClassCP();

    IECInstancePtr classification = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classClassification);
    IECInstancePtr classified = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classSpecificClassifiedClass, [](IECInstanceR instance) { instance.SetValue("Label", ECValue("test label")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipIClassifiedIsClassifiedAs, *classified, *classification);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classSpecificClassifiedClass->GetFullName(), "Label"));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classClassification->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "",
        RequiredRelationDirection_Backward, "", relationshipIClassifiedIsClassifiedAs->GetFullName(), GetClass("IClassified")->GetFullName()));
    rules->AddPresentationRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(BeTest::GetNameOfCurrentTest()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_STREQ("test label", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }


/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_ReturnsWithoutExcludedRelatedClassInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>
    <ECEntityClass typeName="CustomElement" displayLabel="Custom Element">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="GeometricElement" displayLabel="Geometric Element">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement" displayLabel="PhysicalElement Element">
        <BaseClass>GeometricElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_ReturnsWithoutExcludedRelatedClassInstances)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP customElementClass = GetClass("CustomElement");
    ECClassCP geometricElementClass = GetClass("GeometricElement");
    ECClassCP physicalElementClass = GetClass("PhysicalElement");
    ECRelationshipClassCP modelContainsElementsRel = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr baseElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass,
        [&](IECInstanceR inst){inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), modelContainsElementsRel->GetId()));});
    IECInstancePtr customElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass,
        [&](IECInstanceR inst){inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), modelContainsElementsRel->GetId()));});
    IECInstancePtr geometricElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *geometricElementClass,
        [&](IECInstanceR inst){inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), modelContainsElementsRel->GetId()));});
    IECInstancePtr physicalElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *physicalElementClass,
        [&](IECInstanceR inst){inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), modelContainsElementsRel->GetId()));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", modelClass->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Model\"", 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "",
        modelContainsElementsRel->GetFullName(), Utf8PrintfString("%s;E:%s", elementClass->GetFullName(), geometricElementClass->GetFullName())));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *model);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(3, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *baseElement);
    VerifyNodeInstance(*childNodes[1], *customElement);
    VerifyNodeInstance(*childNodes[2], *physicalElement);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_ReturnsWithoutPolymorphicallyExcludedRelatedClassInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>
    <ECEntityClass typeName="CustomElement" displayLabel="Custom Element">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="GeometricElement" displayLabel="Geometric Element">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement" displayLabel="PhysicalElement Element">
        <BaseClass>GeometricElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_ReturnsWithoutPolymorphicallyExcludedRelatedClassInstances)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP customElementClass = GetClass("CustomElement");
    ECClassCP geometricElementClass = GetClass("GeometricElement");
    ECClassCP physicalElementClass = GetClass("PhysicalElement");
    ECRelationshipClassCP modelContainsElementsRel = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr baseElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass,
        [&](IECInstanceR inst){inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), modelContainsElementsRel->GetId()));});
    IECInstancePtr customElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass,
        [&](IECInstanceR inst){inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), modelContainsElementsRel->GetId()));});
    IECInstancePtr geometricElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *geometricElementClass,
        [&](IECInstanceR inst){inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), modelContainsElementsRel->GetId()));});
    IECInstancePtr physicalElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *physicalElementClass,
        [&](IECInstanceR inst){inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), modelContainsElementsRel->GetId()));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", modelClass->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Model\"", 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "",
        modelContainsElementsRel->GetFullName(), Utf8PrintfString("%s;PE:%s", elementClass->GetFullName(), geometricElementClass->GetFullName())));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *model);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *baseElement);
    VerifyNodeInstance(*childNodes[1], *customElement);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert widget instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, true, false, false);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    searchResultInstanceSpecification->AddNestedRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widget).c_str(), widgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 custom node
    DataContainer<NavNodeCPtr> customNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *widgetNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, customNodes.GetSize());
    EXPECT_STREQ("test", customNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, true, false, false);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByClass)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, true, false);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByLabel_DoesntGroup1Instance)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, true);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 instance node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByLabel_Groups3InstancesWith1GroupingNode)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, true);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM RulesEngineTest.Gadget", "RulesEngineTest", "Gadget"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("GadgetID", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 widget instance nodes with label grouping node
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[1]->GetType().c_str());
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(2, widgetNodes.GetSize());
    ASSERT_STREQ("WidgetID", widgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("WidgetID", widgetNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByLabel_Groups4InstancesWith2GroupingNodes)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, true);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM RulesEngineTest.Gadget", "RulesEngineTest", "Gadget"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 2 gadget instance nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[0]->GetType().c_str());
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_STREQ("GadgetID", gadgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("GadgetID", gadgetNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 widget instance nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[1]->GetType().c_str());
    DataContainer<NavNodeCPtr> widgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(2, widgetNodes.GetSize());
    ASSERT_STREQ("WidgetID", widgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("WidgetID", widgetNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_DoNotSort_ReturnsUnsortedNodes)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    searchResultInstanceSpecification->SetDoNotSort(true);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget2", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget1", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ImageIdOverride)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    ImageIdOverrideP imageIdOverride = new ImageIdOverride("ThisNode.Type=\"customType\"", 1, "\"overridedImageId\"");
    rules->AddPresentationRule(*imageIdOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node with overrided ImageId
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedImageId", nodes[0]->GetCollapsedImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Mantas.Kontrimas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, UsesRelatedInstanceInImageIdOverrideCondition)
    {
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass,
        [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Gadget ID"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    ImageIdOverrideP imageIdOverride = new ImageIdOverride("ThisNode.ClassName = \"Widget\" ANDALSO gadgetAlias.MyID = \"Gadget ID\"", 1, "\"overridedImageId\"");
    rules->AddPresentationRule(*imageIdOverride);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    instanceNodesOfSpecificClassesSpecification->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget", "gadgetAlias"));
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node with overrided ImageId
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedImageId", nodes[0]->GetCollapsedImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LabelOverride)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.Type=\"customType\"", 1, "\"overridedLabel\"", "\"overridedDescription\"");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node with overrided Label & Description
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedLabel", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("overridedDescription", nodes[0]->GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LabelOverrideWithGroupedInstancesCountOnClassGroupingNode)
    {
    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget,Gadget", true);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsClassGroupingNode", 1, "\"Count: \" & ThisNode.GroupedInstancesCount", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // verify there're two grouping nodes with correct labels
    ASSERT_EQ(2, nodes.GetSize());
    ASSERT_STREQ("Count: 2", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 3", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNode)
    {
    // insert some instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    rules->AddPresentationRule(*groupingRule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsPropertyGroupingNode", 1, "\"Count: \" & ThisNode.GroupedInstancesCount", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // verify there're three grouping nodes with correct labels
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Count: 1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 2", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 3", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNodeSortedByPropertyValue)
    {
    // insert some instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingSpec->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    rules->AddPresentationRule(*groupingRule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsPropertyGroupingNode", 1, "\"Count: \" & ThisNode.GroupedInstancesCount", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // verify there're three grouping nodes with correct labels
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Count: 2", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 3", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 1", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, StyleOverride)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");

    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    StyleOverrideP styleOverride = new StyleOverride("ThisNode.Type=\"customType\"", 1, "\"overridedForeColor\"", "\"overridedBackColor\"", "\"overridedFontStyle\"");
    rules->AddPresentationRule(*styleOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node with overrided BackColor, ForeColor & FontStyle
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedBackColor", nodes[0]->GetBackColor().c_str());
    ASSERT_STREQ("overridedForeColor", nodes[0]->GetForeColor().c_str());
    ASSERT_STREQ("overridedFontStyle", nodes[0]->GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LocalizationResourceKeyDefinition_WithExistingKey)
    {
    InitTestL10N();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "@custom:Label@", "description", "imageId");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    LocalizationResourceKeyDefinitionP localizationRecourceKeyDefinition = new LocalizationResourceKeyDefinition(1, "custom:Label", "RulesEngine:Test", "notfound");
    rules->AddPresentationRule(*localizationRecourceKeyDefinition);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("T35T", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LocalizationResourceKeyDefinition_KeyNotFound)
    {
    InitTestL10N();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "@custom:Label@", "description", "imageId");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    LocalizationResourceKeyDefinitionP localizationRecourceKeyDefinition = new LocalizationResourceKeyDefinition(1, "custom:Label", "key", "notfound");
    rules->AddPresentationRule(*localizationRecourceKeyDefinition);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("notfound", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CheckBoxRule_UsesDefaultValueIfPropertyIsNull)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"WidgetID\"", 1, false, "BoolProperty", false, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget node
    ASSERT_STREQ("GadgetID", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 widget node with CheckBoxRule
    ASSERT_STREQ("WidgetID", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(nodes[1]->IsCheckboxVisible());
    ASSERT_TRUE(nodes[1]->IsCheckboxEnabled());
    ASSERT_FALSE(nodes[1]->IsChecked());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CheckBoxRule_WithoutProperty)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "customLabel", "description", "imageId");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"customLabel\"", 1, false, "", false, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("customLabel", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(nodes[0]->IsCheckboxVisible());
    ASSERT_TRUE(nodes[0]->IsCheckboxEnabled());
    ASSERT_FALSE(nodes[0]->IsChecked());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CheckBoxRule_UsesInversedPropertyName)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){
        instance.SetValue("BoolProperty", ECValue(false));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){
        instance.SetValue("BoolProperty", ECValue(false));
        instance.SetValue("MyID", ECValue("GadgetID"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"WidgetID\"", 1, false, "BoolProperty", true, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget node
    ASSERT_STREQ("GadgetID", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 widget node with CheckBoxRule
    ASSERT_STREQ("WidgetID", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(nodes[1]->IsCheckboxVisible());
    ASSERT_TRUE(nodes[1]->IsCheckboxEnabled());
    ASSERT_TRUE(nodes[1]->IsChecked());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CheckBoxRule_DoesNotUseInversedPropertyName)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){
        instance.SetValue("BoolProperty", ECValue(false));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){
        instance.SetValue("BoolProperty", ECValue(false));
        instance.SetValue("MyID", ECValue("GadgetID"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"WidgetID\"", 1, false, "BoolProperty", false, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget node
    ASSERT_STREQ("GadgetID", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 widget node with CheckBoxRule
    ASSERT_STREQ("WidgetID", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(nodes[1]->IsCheckboxVisible());
    ASSERT_TRUE(nodes[1]->IsCheckboxEnabled());
    ASSERT_FALSE(nodes[1]->IsChecked());
    }
/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingAscending_LabelOverride)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget3"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SortingRule_SortingAscending", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "MyID", true, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 3 nodes sorted ascending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Widget2", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Widget3", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingAscending)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("widget2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget3"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "MyID", true, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 3 nodes sorted ascending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("widget2", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Widget3", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingDescending)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("widget2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget3"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "MyID", false, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 node sorted descending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget3", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("widget2", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Widget1", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_DoNotSort)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget3"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "DoNotSortByWidgetLabel", false, true, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 3 unsorted nodes
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget2", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Widget3", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Widget1", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingAscendingPolymorphically)
    {
    // insert some ClassE & ClassF instances
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(2)); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(4)); });
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(1)); });
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(3)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "ClassE", "IntProperty", true, false, true);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 4 sorted ascending ClassE & ClassF nodes
    ASSERT_EQ(4, nodes.GetSize());

    VerifyNodeInstance(*nodes[0], *instance3);
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance3).c_str(), nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyNodeInstance(*nodes[1], *instance1);
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyNodeInstance(*nodes[2], *instance4);
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance4).c_str(), nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyNodeInstance(*nodes[3], *instance2);
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), nodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingByTwoProperties)
    {
    // insert some widget instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("DoubleProperty", ECValue(1.0));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("DoubleProperty", ECValue(2.0));
        });
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("DoubleProperty", ECValue(4.0));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "IntProperty", true, false, false);
    rules->AddPresentationRule(*sortingRule);
    sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "DoubleProperty", false, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 3 nodes sorted ascending by IntProperty & descending by DoubleProperty
    ASSERT_EQ(3, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *instance3);
    VerifyNodeInstance(*nodes[1], *instance2);
    VerifyNodeInstance(*nodes[2], *instance1);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingByEnumProperty)
    {
    // insert some instances
    ECEntityClassCP classQ = GetClass("RulesEngineTest", "ClassQ")->GetEntityClassCP();
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance)
        {
        instance.SetValue("IntEnum", ECValue(1));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance)
        {
        instance.SetValue("IntEnum", ECValue(3));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "ClassQ", "IntEnum", true, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes sorted correctly by enum display value
    ASSERT_EQ(2, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *instance2);
    VerifyNodeInstance(*nodes[1], *instance1);
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#176463
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortsLargeNumbersOfNodesAtTheSameHierarchyLevelCorrectly, R"*(
    <ECEntityClass typeName="SpatialIndex" modifier="Sealed" displayLabel="Spatial Index">
        <ECProperty propertyName="MinX" typeName="long" readOnly="True" displayLabel="Min X" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortsLargeNumbersOfNodesAtTheSameHierarchyLevelCorrectly)
    {
    // set up dataset
    ECClassCP elementClass = GetClass("SpatialIndex");
    size_t instancesCount = 257; // need at least 257 instances to reproduce the issue
    for (size_t i = 0; i < instancesCount; ++i)
        {
        RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [i](IECInstanceR instance)
            {
            instance.SetValue("MinX", ECValue((int64_t)i));
            });
        }

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", elementClass->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, elementClass->GetFullName(), "MinX"));

    //make sure we have all nodes in correct order
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(instancesCount, rootNodes.GetSize());
    for (size_t i = 0; i < instancesCount; ++i)
        EXPECT_STREQ(std::to_string(i).c_str(), rootNodes[i]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByBaseClass)
    {
    // insert some widget instances
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    ECClassCP classG = m_schema->GetClassCP("ClassG");
    IECInstancePtr instanceG = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classG);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", false, "RulesEngineTest", "ClassE");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 classE child nodes
    DataContainer<NavNodeCPtr> classEChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classEChildNodes[0]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceF).c_str(), classEChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classEChildNodes[1]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceG).c_str(), classEChildNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_DoesNotCreateGroupForSingleItem)
    {
    // insert ClassF instance
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", false, "RulesEngineTest", "ClassE");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 ClassF node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceF).c_str(), nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_CreatesGroupForSingleItem)
    {
    // insert ClassF instance
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", true, "RulesEngineTest", "ClassE");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 ClassF node
    DataContainer<NavNodeCPtr> classFNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, classFNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classFNodes[0]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceF).c_str(), classFNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", false, "MyID", "");
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 Widget node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem_InstanceLabelOverride)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", false, "MyID", "");
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 Widget node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_CreatesGroupForSingleItem_InstanceLabelOverride)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "MyID", "");
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 widget property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_SetImageIdForGroupingNode)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "changedImageId", true, "MyID", "");
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 widget property grouping node with changed ImageId
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("changedImageId", nodes[0]->GetCollapsedImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByNullProperty)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert 2 instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr node = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node->GetType().c_str());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), node->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    EXPECT_EQ(2, children.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsPolymorphically)
    {
    ECClassCP classD = m_schema->GetClassCP("ClassD");
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    ECRelationshipClassCP classDHasClassE = m_schema->GetClassCP("ClassDHasClassE")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, [](IECInstanceR instance){instance.SetValue("StringProperty", ECValue("ClassD_Label"));});
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDHasClassE, *instanceD, *instanceE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDHasClassE, *instanceD, *instanceF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:ClassD", "StringProperty"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "ClassD", ""));
    rules->AddPresentationRule(*groupingRule);

    GroupingRuleP groupingRule2 = new GroupingRule("", 1, false, "RulesEngineTest", "ClassF", "", "", "");
    groupingRule2->AddGroup(*new PropertyGroup("", "", true, "IntProperty", ""));
    rules->AddPresentationRule(*groupingRule2);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassD_Label", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, children[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, children[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByStringProperty, R"*(
    <ECEntityClass typeName="MyClass">
        <ECProperty propertyName="MyProp" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByStringProperty)
    {
    ECClassCP ecClass = GetClass("MyClass");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProp", ECValue("My Value"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProp", ECValue("My Value"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", ecClass->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, ecClass->GetSchema().GetName(), ecClass->GetName(), "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "MyProp", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("My Value", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, children.GetSize());
    for (size_t i = 0; i < 2; ++i)
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, children[i]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByIntegerEnumAsGroupingValue)
    {
    ECClassCP classQ = m_schema->GetClassCP("ClassQ");

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance){instance.SetValue("IntEnum", ECValue(3));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance){instance.SetValue("IntEnum", ECValue(1));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassQ", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassQ", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntEnum", "");
    propertyGroup->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    propertyGroup->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 property grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("A", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[1]->GetType().c_str());
    ASSERT_STREQ("Z", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByStringEnumAsDisplayLabel)
    {
    ECClassCP classQ = m_schema->GetClassCP("ClassQ");

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance){instance.SetValue("StrEnum", ECValue("Three"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classQ, [](IECInstanceR instance){instance.SetValue("StrEnum", ECValue("One"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassQ", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassQ", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "StrEnum", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 property grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[1]->GetType().c_str());
    ASSERT_STREQ("3", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByNullForeignKey)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");

    // insert an instance with null foreign key to ClassD
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "ClassD", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByRelationshipProperty)
    {
    ECRelationshipClassCP rel = m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *widget, *gadget, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childNodeRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "WidgetHasGadget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Priority", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // expect 1 widget node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    VerifyNodeInstance(*rootNodes[0], *widget);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });

    // expect 1 child property grouping node
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    ASSERT_STREQ("5", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByRelationshipNavigationProperty)
    {
    ECEntityClassCP classS = m_schema->GetClassCP("ClassS")->GetEntityClassCP();
    ECEntityClassCP classT = m_schema->GetClassCP("ClassT")->GetEntityClassCP();
    ECEntityClassCP classU = m_schema->GetClassCP("ClassU")->GetEntityClassCP();
    ECRelationshipClassCP rel_st = m_schema->GetClassCP("ClassSHasClassT")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_stu = m_schema->GetClassCP("STRelationshipHasClassU")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr instanceS = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classS, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instanceT = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classT, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instanceU = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classU, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_st, *instanceS, *instanceT, [&](IECInstanceR instance)
        {
        instance.SetValue("InstanceU", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceU).GetId(), rel_stu));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassS", true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:ClassSHasClassT", "RulesEngineTest:ClassT"));
    rules->AddPresentationRule(*childNodeRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassSHasClassT", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "InstanceU", ""));
    rules->AddPresentationRule(*groupingRule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.ClassName=\"ClassU\"", 1, "\"Label \" & this.IntProperty", "");
    rules->AddPresentationRule(*labelOverride);

    // request for root nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // expect 1 ClassS node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceS);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });

    // expect 1 child property grouping node with label of instanceU
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Label 3", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // request for grandchildren
    DataContainer<NavNodeCPtr> grandchildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get(); });

    // expect 1 grandchild instance node
    ASSERT_EQ(1, grandchildNodes.GetSize());
    VerifyNodeInstance(*grandchildNodes[0], *instanceT);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByRelationshipNavigationProperty_InstanceLabelOverride)
    {
    ECEntityClassCP classS = m_schema->GetClassCP("ClassS")->GetEntityClassCP();
    ECEntityClassCP classT = m_schema->GetClassCP("ClassT")->GetEntityClassCP();
    ECEntityClassCP classU = m_schema->GetClassCP("ClassU")->GetEntityClassCP();
    ECRelationshipClassCP rel_st = m_schema->GetClassCP("ClassSHasClassT")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_stu = m_schema->GetClassCP("STRelationshipHasClassU")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr instanceS = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classS, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instanceT = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classT, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instanceU = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classU, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_st, *instanceS, *instanceT, [&](IECInstanceR instance)
        {
        instance.SetValue("InstanceU", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceU).GetId(), rel_stu));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsByRelationshipNavigationProperty_InstanceLabelOverride", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassS", true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:ClassSHasClassT", "RulesEngineTest:ClassT"));
    rules->AddPresentationRule(*childNodeRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassSHasClassT", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "InstanceU", ""));
    rules->AddPresentationRule(*groupingRule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:ClassU", "IntProperty"));

    // request for root nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // expect 1 ClassS node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceS);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });

    // expect 1 child property grouping node with label of instanceU
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("3", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // request for grandchildren
    DataContainer<NavNodeCPtr> grandchildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get(); });

    // expect 1 grandchild instance node
    ASSERT_EQ(1, grandchildNodes.GetSize());
    VerifyNodeInstance(*grandchildNodes[0], *instanceT);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsBySkippedRelationshipProperty)
    {
    ECRelationshipClassCP rel1 = m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel1, *widget, *gadget, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel2, *gadget, *sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket"));
    rules->AddPresentationRule(*childNodeRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "WidgetHasGadget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Priority", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // expect 1 widget node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *widget);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });

    // expect 1 child property grouping node
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    ASSERT_STREQ("5", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsMultipleClassesByTheSameRelationshipProperty)
    {
    ECEntityClassCP classD = m_schema->GetClassCP("ClassD")->GetEntityClassCP();
    ECEntityClassCP classF = m_schema->GetClassCP("ClassF")->GetEntityClassCP();
    ECEntityClassCP classG = m_schema->GetClassCP("ClassG")->GetEntityClassCP();
    ECRelationshipClassCP rel = m_schema->GetClassCP("ClassDReferencesClassE")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    IECInstancePtr instanceG = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classG);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instanceD, *instanceF, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instanceD, *instanceG, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:ClassD", false));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:ClassDReferencesClassE", "RulesEngineTest:ClassF,ClassG"));
    rules->AddPresentationRule(*childNodeRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassDReferencesClassE", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Priority", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // expect 1 ClassD node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceD);

    // expect 1 child property grouping node
    DataContainer<NavNodeCPtr> childNodes1 = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes1.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes1[0]->GetType().c_str());
    ASSERT_STREQ("5", childNodes1[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 2 child instance nodes under the grouping node
    DataContainer<NavNodeCPtr> childNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *childNodes1[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childNodes2.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes2[0]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceF).c_str(), childNodes2[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes2[1]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceG).c_str(), childNodes2[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("a0"));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("a1"));});
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("a1"));});
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(""));
    rules->AddPresentationRule(*groupingRule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "UserLabel"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "UserLabel"));

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 3 instance nodes, one of them is merged
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("a0", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstance(*nodes[0], *a1);
    EXPECT_STREQ("a1", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*nodes[1], {a2, a3});
    EXPECT_STREQ("b", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstance(*nodes[2], *b);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_InstanceLabelOverride_SameLabelInstanceGroup)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_SameLabelInstanceGroup", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    SameLabelInstanceGroupP sameLabelInstanceGroup = new SameLabelInstanceGroup("");
    groupingRule->AddGroup(*sameLabelInstanceGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 widget instance node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_ReturnsChildrenForAllGroupedInstances_UsingRelatedInstanceNodesSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_Bs" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup_ReturnsChildrenForAllGroupedInstances_UsingRelatedInstanceNodesSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_Has_Bs")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a")); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a")); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b1);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *b2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "UserLabel"));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(""));
    rules->AddPresentationRule(*groupingRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", rel->GetFullName(), classB->GetFullName()));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], {a1, a2});

    // request children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *b1);
    VerifyNodeInstance(*childNodes[1], *b2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_ReturnsChildrenForAllGroupedInstances_UsingInstanceNodesOfSpecificClassesSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_Bs" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_Bs" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup_ReturnsChildrenForAllGroupedInstances_UsingInstanceNodesOfSpecificClassesSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_Has_Bs")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a")); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a")); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b1);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *b2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "UserLabel"));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(""));
    rules->AddPresentationRule(*groupingRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.A.Id = parent.ECInstanceId", classB->GetFullName(), false));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], {a1, a2});

    // request children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *b1);
    VerifyNodeInstance(*childNodes[1], *b2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_DuringPostProcessing_FromDifferentHierarchyLevels, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_Bs" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup_DuringPostProcessing_FromDifferentHierarchyLevels)
    {
    ECClassCP baseClass = GetClass("Element");
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP rel = GetClass("A_Has_Bs")->GetRelationshipClassCP();

    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("c")); });

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a1")); });
    IECInstancePtr b11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b11")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b11);
    IECInstancePtr b12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("m")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b12);
    IECInstancePtr b13 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b13")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b13);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a2")); });
    IECInstancePtr b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("m")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *b21);
    IECInstancePtr b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b22")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *b22);
    IECInstancePtr b23 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b23")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *b23);

    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("m")); });

    /*
    produce the following hierarchy:
        c
        a1 (virtual)
            b11
            b12
            b13
        a2 (virtual)
            b21
            b22
            b23
        d
    virtual nodes are hidden so without postprocessing we would see this:
        c
        b11
        b12
        b13
        b21
        b22
        b23
        d
    and since postprocessing merges b12, b21 and d, we should see this:
        c
        b11
        merge of [b12, b21, d]
        b13
        b22
        b23
    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, baseClass->GetFullName(), "UserLabel"));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classC->GetFullName(), false));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false,
        "", classA->GetFullName(), false));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classD->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    auto childSpec = new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", rel->GetFullName(), classB->GetFullName());
    childSpec->SetDoNotSort(true);
    childRule->AddSpecification(*childSpec);
    rules->AddPresentationRule(*childRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, baseClass->GetSchema().GetName(), baseClass->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(6, rootNodes.GetSize());
    EXPECT_STREQ("c", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[0], {c});
    EXPECT_STREQ("b11", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[1], {b11});
    EXPECT_STREQ("m", rootNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[2], {b12, b21, d});
    EXPECT_STREQ("b13", rootNodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[3], {b13});
    EXPECT_STREQ("b22", rootNodes[4]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[4], {b22});
    EXPECT_STREQ("b23", rootNodes[5]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[5], {b23});
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_DuringPostProcessing_FromSameHierarchyLevel, R"*(
    <ECEntityClass typeName="MyClass">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup_DuringPostProcessing_FromSameHierarchyLevel)
    {
    ECClassCP ecClass = GetClass("MyClass");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("a"));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("a"));});
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("a"));});
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("b"));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("b"));});
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("b"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, ecClass->GetFullName(), "UserLabel"));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", ecClass->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, ecClass->GetSchema().GetName(), ecClass->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("a", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[0], {a1, a2, a3});
    EXPECT_STREQ("b", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[1], {b1, b2, b3});
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_DuringPostProcessing_GroupsChildNodes, R"*(
    <ECEntityClass typeName="MyClass">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup_DuringPostProcessing_GroupsChildNodes)
    {
    ECClassCP ecClass = GetClass("MyClass");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a")); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b")); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, ecClass->GetFullName(), "UserLabel"));

    RootNodeRule* rootRule = new RootNodeRule();
    auto rootSpec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.UserLabel = \"a\"", ecClass->GetFullName(), false);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.UserLabel = \"b\"", ecClass->GetFullName(), false));
    rootSpec->AddNestedRule(*childRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, ecClass->GetSchema().GetName(), ecClass->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    childRule->AddCustomizationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], {a});

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("b", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*childNodes[0], {b1, b2});
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_DuringPostProcessing_GroupsNodesFromDifferentSpecifications, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup_DuringPostProcessing_GroupsNodesFromDifferentSpecifications)
    {
    ECClassCP baseClass = GetClass("Element");
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a1")); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a2")); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b1")); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("m")); });
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("m")); });
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("c2")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, baseClass->GetFullName(), "UserLabel"));

    RootNodeRule* rootRule = new RootNodeRule();
    auto rootSpec = new CustomNodeSpecification(1, false, "T_ROOT", "Root", "", "");
    rootSpec->SetHasChildren(ChildrenHint::Unknown);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false));
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classB->GetFullName(), false));
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classC->GetFullName(), false));
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classD->GetFullName(), false));
    rootSpec->AddNestedRule(*childRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, baseClass->GetSchema().GetName(), baseClass->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId()).GetJson();
    EXPECT_EQ(1, m_manager->GetRootNodesCount(s_project->GetECDb(), options).get());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("T_ROOT", rootNodes[0]->GetType().c_str());

    EXPECT_EQ(5, m_manager->GetChildrenCount(s_project->GetECDb(), *rootNodes[0], options).get());
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(5, childNodes.GetSize());

    EXPECT_STREQ("a1", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*childNodes[0], {a1});

    EXPECT_STREQ("a2", childNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*childNodes[1], {a2});

    EXPECT_STREQ("b1", childNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*childNodes[2], {b1});

    EXPECT_STREQ("m", childNodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*childNodes[3], {b2, c1});

    EXPECT_STREQ("c2", childNodes[4]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*childNodes[4], {c2});
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges)
    {
    // insert some widget instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(7));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Other()).c_str(), nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, children[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByIntegersRange)
    {
    // insert an instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(4));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, children[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByDoublesRange)
    {
    // insert an instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("DoubleProperty", ECValue(4.5));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "DoubleProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, children[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByLongsRange)
    {
    // insert an instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("LongProperty", ECValue((int64_t)4));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "LongProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, children[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByDateTimeRange)
    {
    // insert an instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("DateProperty", ECValue(DateTime(2017, 5, 30)));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "DateProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "2017-05-01", "2017-06-01");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, children[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_WithHideIfNoChildrenParent)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(4));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetClass, *widget, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false,
        "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // expect 1 gadget node
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*gadget).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // request children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("1 - 5", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByBaseClassAndByInstancesClasses)
    {
    // insert some widget instances
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, true, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", true, "RulesEngineTest", "ClassE");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 classE child nodes
    DataContainer<NavNodeCPtr> classEChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classEChildNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassF", classEChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Recursion prevention
* @betest                                       Pranciskus.Ambrazas               07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DoesNotReturnECInstanceNodesOfTheSameSpecificationAlreadyExistingInHierarchy)
    {
    // insert classD & classE instances with relationship
    ECRelationshipClassCR relationship = *m_schema->GetClassCP("ClassDHasClassE")->GetRelationshipClassCP();
    ECClassCP classD = m_schema->GetClassCP("ClassD");
    IECInstancePtr classDInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    IECInstancePtr classEInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationship, *classDInstance, *classEInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassD", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassD,ClassE");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*relatedNodeRule);

    // make sure we have 1 ClassD root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*classDInstance).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // ClassD instance node should have 1 ClassE instance child node
    DataContainer<NavNodeCPtr> classDChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, classDChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classDChildNodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*classEInstance).c_str(), classDChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // ClassE instance node has 1 ClassD instance node. There's such a node already in the hierarchy, but it's based on
    // different (root node rule) specification, so we allow it
    DataContainer<NavNodeCPtr> classEChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classDChildNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classEChildNodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*classDInstance).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // This time ClassD instance node has no children because there's such a node up in the
    // hierarchy and it's also based on the same specification.
    DataContainer<NavNodeCPtr> classDChildNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *classEChildNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(0, classDChildNodes2.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#624346
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupingWorksCorrectlyWithRelatedInstancesSpecificationWhenParentRelatedInstanceNodeLevelIsHidden)
    {
    ECRelationshipClassCR widgetHasGadgetsRelationship = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    /* Create the following hierarchy:
            + widget
            +--+ Property grouping node
            |  +--- gadget
       Then, hide the first level (widget). The expected result:
            + Property grouping node
            +--- gadget
    */
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), widgetHasGadgetsRelationship, *widget, *gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*relatedNodeRule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Gadget", "", "", "");
    PropertyGroup* propertyGroupSpec = new PropertyGroup("", "", true, "MyID");
    groupingRule->AddGroup(*propertyGroupSpec);
    rules->AddPresentationRule(*groupingRule);

    // make sure we have 1 property grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());

    // the node should have 1 Gadget child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *gadget);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomizesNodesWhenCustomizationRulesDefinedInSupplementalRuleset)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule sets
    PresentationRuleSetPtr primaryRules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*primaryRules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    primaryRules->AddPresentationRule(*rule);

    PresentationRuleSetPtr supplementalRules = PresentationRuleSet::CreateInstance(primaryRules->GetRuleSetId(), 1, 0, true, "Customization", "", "", false);
    m_locater->AddRuleSet(*supplementalRules);

    LabelOverride* customizationRule = new LabelOverride("", 1, "\"Test\"", "");
    supplementalRules->AddPresentationRule(*customizationRule);

    // make sure we have 1 node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(primaryRules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *widget);
    EXPECT_STREQ("Test", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupingChildrenByRelatedInstanceProperty)
    {
    ECRelationshipClassCR widgetHasGadgetsRelationship = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCR gadgetHasSprocketsRelationship = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    /* Create the following hierarchy:
            + widget
            +--+ Sprocket property grouping node
            |  +--- gadget + related sprocket
    */
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){
        instance.SetValue("Description", ECValue("test"));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("test"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), widgetHasGadgetsRelationship, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), gadgetHasSprocketsRelationship, *gadget, *sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP relatedInstanceNodesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "sprocket.MyID = parent.Description", "RulesEngineTest:Gadget", false);
    relatedInstanceNodesSpecification->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket", "sprocket"));
    childNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*childNodeRule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Sprocket", "", "", "");
    PropertyGroup* propertyGroupSpec = new PropertyGroup("", "", true, "Description");
    groupingRule->AddGroup(*propertyGroupSpec);
    rules->AddPresentationRule(*groupingRule);

    // make sure we have 1 widget
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // the node should have 1 property grouping node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());

    // the child node should have 1 Gadget child node
    DataContainer<NavNodeCPtr> grandChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, grandChildNodes.GetSize());
    VerifyNodeInstance(*grandChildNodes[0], *gadget);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupingNodesByMultiStepRelatedInstanceProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Name" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupingNodesByMultiStepRelatedInstanceProperty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_Has_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Name", ECValue("test")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, classB->GetFullName()),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()),
        }), "c", true));
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, classC->GetSchema().GetName(), classC->GetName(), "", "", "");
    PropertyGroup* propertyGroupSpec = new PropertyGroup("", "", true, "Name");
    groupingRule->AddGroup(*propertyGroupSpec);
    rules->AddPresentationRule(*groupingRule);

    // make sure we have 1 widget
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("test", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // the child node should have 1 Gadget child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *a);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupingChildrenByRelatedInstanceProperty_InstanceLabelOverride)
    {
    ECRelationshipClassCR widgetHasGadgetsRelationship = *m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCR gadgetHasSprocketsRelationship = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    /* Create the following hierarchy:
            + widget
            +--+ Sprocket property grouping node
            |  +--- gadget + related sprocket
    */
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){
        instance.SetValue("Description", ECValue("test"));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("test"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), widgetHasGadgetsRelationship, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), gadgetHasSprocketsRelationship, *gadget, *sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupingChildrenByRelatedInstanceProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP relatedInstanceNodesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "sprocket.MyID = parent.Description", "RulesEngineTest:Gadget", false);
    relatedInstanceNodesSpecification->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket", "sprocket"));
    childNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*childNodeRule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Sprocket", "", "", "");
    PropertyGroup* propertyGroupSpec = new PropertyGroup("", "", true, "Description");
    groupingRule->AddGroup(*propertyGroupSpec);
    rules->AddPresentationRule(*groupingRule);

    // make sure we have 1 widget
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // the node should have 1 property grouping node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());

    // the child node should have 1 Gadget child node
    DataContainer<NavNodeCPtr> grandChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, grandChildNodes.GetSize());
    VerifyNodeInstance(*grandChildNodes[0], *gadget);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsingRelatedInstanceOfTheSameClassAsInRelatedInstancesSpecification, R"*(
    <ECEntityClass typeName="ClassA" />
    <ECEntityClass typeName="ClassB" />
    <ECRelationshipClass typeName="A_Has_Bs" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="false">
            <Class class="ClassA"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="false">
            <Class class="ClassB"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, UsingRelatedInstanceOfTheSameClassAsInRelatedInstancesSpecification)
    {
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("A_Has_Bs")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP childSpec = new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0,
        "b.ECInstanceId <> parent.ECInstanceId", RequiredRelationDirection_Backward, "", rel->GetFullName(), classA->GetFullName());
    childSpec->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), classB->GetFullName(), "b"));
    childNodeRule->AddSpecification(*childSpec);
    rules->AddPresentationRule(*childNodeRule);

    // make sure we have 2 instances of ClassB
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b1);
    VerifyNodeInstance(*rootNodes[1], *b2);

    // each node should have 1 ClassA child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *a);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, ReturnsChildNodesWhenTheresOnlyOneLabelGroupingNode)
    {
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // make sure we have 2 widget nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, ReturnsChildrenUsingAllSpecifications)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("a"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"a\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*CreateCustomNodeSpecification("b"));
    childRule->AddSpecification(*CreateCustomNodeSpecification("c"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("a", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 2 child nodes
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("b", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("c", childNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AutoExpandSetsIsExpandedFlagForRootNodes)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    // set auto expand property to true (default false)
    RootNodeRule* rootRule = new RootNodeRule("1=1", 10, false, TargetTree_Both, true);
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "test", "test", "test", "test");
    rootRule->AddSpecification(*spec);
    rules->AddPresentationRule(*rootRule);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->IsExpanded());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByParentNodes_MatchingFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test"));});
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsOfClass(\"Widget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.Description", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*childRule);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.IsOfClass(\"Gadget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.parent.Description", "RulesEngineTest:Sprocket", false));
    rules->AddPresentationRule(*childRule2);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widget).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*gadget).c_str(), childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 child node as well
    DataContainer<NavNodeCPtr> childNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes2.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*sprocket).c_str(), childNodes2[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByParentNodes_NonMatchingParentFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test1"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsOfClass(\"Widget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.Description", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widget).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(0, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByParentNodes_NonMatchingGrandparentFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test1"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test1"));});
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsOfClass(\"Widget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.Description", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*childRule);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.IsOfClass(\"Gadget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.parent.Description", "RulesEngineTest:Sprocket", false));
    rules->AddPresentationRule(*childRule2);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*widget).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*gadget).c_str(), childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(0, childNodes2.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FiltersGroupedNodesByGrandParentNodes_MatchingFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Value" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Value" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, FiltersGroupedNodesByGrandParentNodes_MatchingFilter)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Value", ECValue(1));});
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Value", ECValue(1));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.ClassName=\"%s\"", classA->GetName().c_str()), 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false,
        "", classB->GetFullName(), false));
    rules->AddPresentationRule(*childRule);

    ChildNodeRule* grandChildRule = new ChildNodeRule(Utf8PrintfString("ParentNode.ClassName=\"%s\"", classB->GetName().c_str()), 1, false, TargetTree_Both);
    grandChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Value = parent.parent.Value", classC->GetFullName(), false));
    rules->AddPresentationRule(*grandChildRule);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> aNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, aNodes.GetSize());
    VerifyNodeInstance(*aNodes[0], *a);

    // make sure it has 1 'B' grouping node
    DataContainer<NavNodeCPtr> bGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *aNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, bGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, bGroupingNodes[0]->GetKey()->GetType().c_str());

    // make sure it has 1 'B' instance node
    DataContainer<NavNodeCPtr> bNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *bGroupingNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, bNodes.GetSize());
    VerifyNodeInstance(*bNodes[0], *b);
    
    // make sure it has 1 'C' instance node
    DataContainer<NavNodeCPtr> cNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *bNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, cNodes.GetSize());
    VerifyNodeInstance(*cNodes[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_NoDigitsAfterDecimalPointAppendTwoZeroes)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2.00", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_OneDigitAfterDecimalPointAppendOneZero)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.5));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.5));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2.50", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointSameResult)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(0.00546));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(0.00798));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(0.00899));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("0.01", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 3 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(3, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointDifferentResult)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.505f));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.504f));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("2.50", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("2.51", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", firstNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", secondNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointSameResult)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.59999999));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.59999999));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2.60", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointDifferentResult)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.595));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("DoubleProperty", ECValue(2.594));
        instance.SetValue("MyID", ECValue("WidgetID"));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("2.59", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("2.60", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", firstNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", secondNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPoints)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_AlmostEqualPointsSameResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.121, 1.121, 1.121))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.122, 1.122, 1.122))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_DifferentPointsDifferentResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.11, 1.11, 1.11))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.11 Y: 1.11 Z: 1.11", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), firstNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), secondNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPointsDifferentXCoordinatesDifferentResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.11, 1.12, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.11 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), firstNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), secondNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPointsDifferentYCoordinatesDifferentResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.11, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.11 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), firstNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), secondNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPointsDifferentZCoordinatesDifferentResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.11))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.11", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), firstNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get(); });
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), secondNodeChildrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_YZCoordinatesEqualXCoordinatesAlmostEqualSameResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.119, 1.12, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.124, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_XZCoordinatesEqualYCoordinatesAlmostEqualSameResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.119, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.124, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_XYCoordinatesEqualZCoordinatesAlmostEqualSameResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.119))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.124))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDateTimeProperty)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DateProperty", ECValue(DateTime(2019, 10, 03))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DateProperty", ECValue(DateTime(2019, 10, 03))); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "DateProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", m_widgetClass->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    // make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2019-10-03T00:00:00.000Z", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsFilteredNodesFromNotExpandedHierarchy)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for filtered nodes paths
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    bvector<NodesPathElement> nodes = m_manager->GetFilteredNodePaths(s_project->GetECDb(), "", options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.size());
    EXPECT_EQ(1, nodes[0].GetChildren().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsFilteredNodesMatchingPercentSymbol)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("%")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, m_widgetClass->GetFullName(), "MyID"));

    // request for filtered nodes paths
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    bvector<NodesPathElement> nodes = m_manager->GetFilteredNodePaths(s_project->GetECDb(), "%", options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsFilteredNodesMatchingUnderscoreSymbol)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("_")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, m_widgetClass->GetFullName(), "MyID"));

    // request for filtered nodes paths
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    bvector<NodesPathElement> nodes = m_manager->GetFilteredNodePaths(s_project->GetECDb(), "_", options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DoesNotReturnFilteredNodesNotMatchingPercentSymbol)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for filtered nodes paths
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    bvector<NodesPathElement> nodes = m_manager->GetFilteredNodePaths(s_project->GetECDb(), "%", options).get();

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DoesNotReturnFilteredNodesNotMatchingUnderscoreSymbol)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for filtered nodes paths
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    bvector<NodesPathElement> nodes = m_manager->GetFilteredNodePaths(s_project->GetECDb(), "_", options).get();

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas               10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsFilteredNodesUnderSameParent)
    {
    // insert widget & 2 gadget instances with relationship
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadgetInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Gadget")); });
    IECInstancePtr gadgetInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Gadget")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP widgetSpec = new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "", m_widgetClass->GetFullName(), false);
    rootRule->AddSpecification(*widgetSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Always, false, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", relationshipWidgetHasGadget.GetFullName(), m_gadgetClass->GetFullName()));
    widgetSpec->AddNestedRule(*childRule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, m_gadgetClass->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("MyID") }));

    // request for filtered nodes paths
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    bvector<NodesPathElement> nodes = m_manager->GetFilteredNodePaths(s_project->GetECDb(), "Gadget", options).get();

    // make sure we have 1 root node
    ASSERT_EQ(1, nodes.size());
    // make sure we have 2 child nodes
    EXPECT_EQ(2, nodes[0].GetChildren().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_OverridesInstanceLabelAsFirstNotEmptyParameter)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverride_OverridesInstanceLabelAsFirstNotEmptyParameter", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "Description,MyID"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget2", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget1", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_FindsCorrectPropertyWithLowerCasePropertyNameAndOverridesLabel)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverride_FindsCorrectPropertyWithLowerCasePropertyNameAndOverridesLabel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "myId"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });

    // make sure we have 1 nodes
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_AssertsWhenPropertyNameIsInvalid)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstanceLabelOverride_AssertsWhenPropertyNameIsInvalid", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "my"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str()).GetJson();
    IGNORE_BE_ASSERT();
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options).get(); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_HandlesDefaultBisRulesCorrectlyForRootNodes, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="GeometricElement" displayLabel="Geometric Element">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="CustomElement" displayLabel="Custom Element">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="CustomGeometricElement" displayLabel="Custom Geometric Element">
        <BaseClass>GeometricElement</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_HandlesDefaultBisRulesCorrectlyForRootNodes)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP geometricElementClass = GetClass("GeometricElement");
    ECClassCP customElementClass = GetClass("CustomElement");
    ECClassCP customGeometricElementClass = GetClass("CustomGeometricElement");

    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("UserLabel", ECValue("Custom Element 1 UserLabel"));
        });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("CodeValue", ECValue("Custom Element 2 CodeValue"));
        });
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass);
    IECInstancePtr geometricElement1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customGeometricElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("CodeValue", ECValue("Custom Geometric Element 1 CodeValue"));
        });
    IECInstancePtr geometricElement2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customGeometricElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("UserLabel", ECValue("Custom Geometric Element 2 UserLabel"));
        });
    IECInstancePtr geometricElement3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customGeometricElementClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, geometricElementClass->GetFullName(),
        {
        new InstanceLabelOverridePropertyValueSpecification("CodeValue"),
        new InstanceLabelOverrideCompositeValueSpecification(
            {
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverridePropertyValueSpecification("UserLabel"), true),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideCompositeValueSpecification(
                {
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("[")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideBriefcaseIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("-")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("]"))
                }, ""))
            }, " "),
        new InstanceLabelOverrideCompositeValueSpecification(
            {
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideClassLabelValueSpecification(), true),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideCompositeValueSpecification(
                {
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("[")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideBriefcaseIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("-")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("]"))
                }, ""))
            }, " ")
        }));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, elementClass->GetFullName(),
        {
        new InstanceLabelOverridePropertyValueSpecification("UserLabel"),
        new InstanceLabelOverridePropertyValueSpecification("CodeValue"),
        new InstanceLabelOverrideCompositeValueSpecification(
            {
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideClassLabelValueSpecification(), true),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideCompositeValueSpecification(
                {
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("[")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideBriefcaseIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("-")),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification()),
                new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("]"))
                }, ""))
            })
        }));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", elementClass->GetFullName(), true);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(6, rootNodes.GetSize());

    EXPECT_STREQ("Custom Element 1 UserLabel", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Element 2 CodeValue", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Element [0-3]", rootNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ("Custom Geometric Element 1 CodeValue", rootNodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Geometric Element 2 UserLabel [0-5]", rootNodes[4]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Geometric Element [0-6]", rootNodes[5]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_HandlesDefaultBisRulesCorrectlyForChildNodes, R"*(
    <ECEntityClass typeName="Model">
    </ECEntityClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
        <ECProperty propertyName="CodeValue" typeName="string" />
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="CustomElement" displayLabel="Custom Element">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_HandlesDefaultBisRulesCorrectlyForChildNodes)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP customElementClass = GetClass("CustomElement");
    ECRelationshipClassCP rel = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("CodeValue", ECValue("CodeValue"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element1);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("UserLabel", ECValue("UserLabel"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element2);
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, elementClass->GetFullName(),
        {
        new InstanceLabelOverridePropertyValueSpecification("CodeValue"),
        new InstanceLabelOverrideCompositeValueSpecification(
            {
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverridePropertyValueSpecification("UserLabel"), true),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideBriefcaseIdValueSpecification()),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification()),
            }, "-"),
        new InstanceLabelOverrideCompositeValueSpecification(
            {
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideClassLabelValueSpecification(), true),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideBriefcaseIdValueSpecification()),
            new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification()),
            }, "-"),
        }));

    RootNodeRule* modelsRule = new RootNodeRule();
    modelsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", modelClass->GetFullName(), true));
    rules->AddPresentationRule(*modelsRule);

    ChildNodeRule* elementsRule = new ChildNodeRule();
    elementsRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", rel->GetFullName(), elementClass->GetFullName()));
    rules->AddPresentationRule(*elementsRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> modelNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, modelNodes.GetSize());

    DataContainer<NavNodeCPtr> elementNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *modelNodes[0], PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(3, elementNodes.GetSize());
    EXPECT_STREQ("CodeValue", elementNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Element-0-4", elementNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("UserLabel-0-3", elementNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClassesSpecification_MultipleInheritance_LabelOverridesAppliedPolymorphically, R"*(
    <ECEntityClass typeName="ClassA1">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassA2" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.1.0">
                <AppliesToEntityClass>ClassB</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA1</BaseClass>
        <BaseClass>ClassA2</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClassesSpecification_MultipleInheritance_LabelOverridesAppliedPolymorphically)
    {
    // set up data set
    ECClassCP classA1 = GetClass("ClassA1");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("ClassC_CodeValue"));});
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA1, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("ClassA1_CodeValue"));});
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA1->GetFullName(), "CodeValue"));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", classA1->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("ClassA1_CodeValue", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("ClassC_CodeValue", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClassesSpecification_MultipleInheritance_LabelOverridesAppliedForSpecifiedClass, R"*(
    <ECEntityClass typeName="ClassA1">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassA2" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.1.0">
                <AppliesToEntityClass>ClassB</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA1</BaseClass>
        <BaseClass>ClassA2</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClassesSpecification_MultipleInheritance_LabelOverridesAppliedForSpecifiedClass)
    {
    // set up data set
    ECClassCP classA1 = GetClass("ClassA1");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassC_UserLabel"));});
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA1, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("ClassA1_CodeValue"));});
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA1->GetFullName(), "CodeValue"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "UserLabel"));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", classA1->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("ClassA1_CodeValue", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("ClassC_UserLabel", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodesSpecification_LabelOverridesAppliedPolymorphically, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodesSpecification_LabelOverridesAppliedPolymorphically)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("ClassC_CodeValue"));});
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("ClassA1_CodeValue"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "CodeValue"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, BeTest::GetNameOfCurrentTest());
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("ClassC_CodeValue", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("ClassA1_CodeValue", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodesSpecification_LabelOverridesAppliedForSpecifiedClass, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodesSpecification_LabelOverridesAppliedForSpecifiedClass)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("1_Instance_C"));});
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("2_Instance_A"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "CodeValue"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "UserLabel"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, BeTest::GetNameOfCurrentTest());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("1_Instance_C", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("2_Instance_A", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CreatesValidHierarchyWhenHidingMultipleHierarchyLevelsWithMultipleSpecificationsInARow)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule1 = new RootNodeRule();
    rule1->AddSpecification(*CreateCustomNodeSpecification("1"));
    rules->AddPresentationRule(*rule1);

    ChildNodeRule* rule2 = new ChildNodeRule("ParentNode.Type = \"1\"", 1, false, TargetTree_Both);
    rule2->AddSpecification(*CreateCustomNodeSpecification("1.1", [](CustomNodeSpecificationR spec) { spec.SetHideNodesInHierarchy(true); }));
    rule2->AddSpecification(*CreateCustomNodeSpecification("1.2", [](CustomNodeSpecificationR spec) {  }));
    rules->AddPresentationRule(*rule2);

    ChildNodeRule* rule3 = new ChildNodeRule("ParentNode.Type = \"1.1\"", 1, false, TargetTree_Both);
    rule3->AddSpecification(*CreateCustomNodeSpecification("1.1.1", [](CustomNodeSpecificationR spec) { spec.SetHideNodesInHierarchy(true); }));
    rule3->AddSpecification(*CreateCustomNodeSpecification("1.1.2", [](CustomNodeSpecificationR spec) {}));
    rules->AddPresentationRule(*rule3);

    ChildNodeRule* rule4 = new ChildNodeRule("ParentNode.Type = \"1.1.1\"", 1, false, TargetTree_Both);
    rule4->AddSpecification(*CreateCustomNodeSpecification("1.1.1.1"));
    rule4->AddSpecification(*CreateCustomNodeSpecification("1.1.1.2"));
    rules->AddPresentationRule(*rule4);

    ChildNodeRule* rule5 = new ChildNodeRule("ParentNode.Type = \"1.1.1.1\"", 1, false, TargetTree_Both);
    rule5->AddSpecification(*CreateCustomNodeSpecification("1.1.1.1.1"));
    rules->AddPresentationRule(*rule5);

    ChildNodeRule* rule6 = new ChildNodeRule("ParentNode.Type = \"1.1.2\"", 1, false, TargetTree_Both);
    rule6->AddSpecification(*CreateCustomNodeSpecification("1.1.2.1", [](CustomNodeSpecificationR spec) { spec.SetHideNodesInHierarchy(true); }));
    rule6->AddSpecification(*CreateCustomNodeSpecification("1.1.2.2", [](CustomNodeSpecificationR spec) { }));
    rules->AddPresentationRule(*rule6);

    ChildNodeRule* rule7 = new ChildNodeRule("ParentNode.Type = \"1.1.2.1\"", 1, false, TargetTree_Both);
    rule7->AddSpecification(*CreateCustomNodeSpecification("1.1.2.1.1", [](CustomNodeSpecificationR spec) {}));
    rules->AddPresentationRule(*rule7);

    /*
    1
        [1.1]
            [1.1.1]
                1.1.1.1
                    1.1.1.1.1
                1.1.1.2
            1.1.2
                [1.1.2.1]
                    1.1.2.1.1
                1.1.2.2
        1.2
    */

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("1", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // get child nodes
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(4, childNodes.GetSize());
    EXPECT_STREQ("1.1.1.1", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("1.1.1.2", childNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("1.1.2", childNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("1.2", childNodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AppliesNodeExtendedDataFromPresentationRules, R"*(
    <ECEntityClass typeName="MyClass1">
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClass2">
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AppliesNodeExtendedDataFromPresentationRules)
    {
    ECClassCP ecClass1 = GetClass("MyClass1");
    ECClassCP ecClass2 = GetClass("MyClass2");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass1, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("test value")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule1 = new RootNodeRule();
    rules->AddPresentationRule(*rule1);
    rule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", ecClass1->GetFullName(), true));

    ExtendedDataRule* ex1 = new ExtendedDataRule();
    ex1->AddItem("class_name", "ThisNode.ClassName");
    ex1->AddItem("property_value", "this.CodeValue");
    ex1->AddItem("is_MyClass", Utf8PrintfString("ThisNode.IsOfClass(\"%s\", \"%s\")", ecClass1->GetName().c_str(), ecClass1->GetSchema().GetName().c_str()));
    ex1->AddItem("my_id", "ThisNode.InstanceId");
    rule1->AddCustomizationRule(*ex1);

    ExtendedDataRule* ex2 = new ExtendedDataRule(Utf8PrintfString("ThisNode.IsOfClass(\"%s\", \"%s\")", ecClass1->GetName().c_str(), ecClass1->GetSchema().GetName().c_str()));
    ex2->AddItem("constant_bool", "true");
    ex2->AddItem("constant_int", "1");
    ex2->AddItem("constant_double", "1.23");
    ex2->AddItem("constant_string", "\"Red\"");
    rules->AddPresentationRule(*ex2);

    RootNodeRule* rule2 = new RootNodeRule();
    rules->AddPresentationRule(*rule2);
    rule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", ecClass2->GetFullName(), true));

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());

    RapidJsonAccessor extendedData = rootNodes[0]->GetUsersExtendedData();
    ASSERT_TRUE(extendedData.GetJson().IsObject());
    ASSERT_EQ(8, extendedData.GetJson().MemberCount());

    ASSERT_TRUE(extendedData.GetJson().HasMember("class_name"));
    EXPECT_STREQ(ecClass1->GetName().c_str(), extendedData.GetJson()["class_name"].GetString());

    ASSERT_TRUE(extendedData.GetJson().HasMember("property_value"));
    EXPECT_STREQ("test value", extendedData.GetJson()["property_value"].GetString());

    ASSERT_TRUE(extendedData.GetJson().HasMember("is_MyClass"));
    EXPECT_TRUE(extendedData.GetJson()["is_MyClass"].GetBool());

    ASSERT_TRUE(extendedData.GetJson().HasMember("my_id"));
    EXPECT_STREQ("0x1", extendedData.GetJson()["my_id"].GetString());

    ASSERT_TRUE(extendedData.GetJson().HasMember("constant_bool"));
    EXPECT_TRUE(extendedData.GetJson()["constant_bool"].GetBool());

    ASSERT_TRUE(extendedData.GetJson().HasMember("constant_int"));
    EXPECT_EQ(1, extendedData.GetJson()["constant_int"].GetInt());

    ASSERT_TRUE(extendedData.GetJson().HasMember("constant_double"));
    EXPECT_DOUBLE_EQ(1.23, extendedData.GetJson()["constant_double"].GetDouble());

    ASSERT_TRUE(extendedData.GetJson().HasMember("constant_string"));
    EXPECT_STREQ("Red", extendedData.GetJson()["constant_string"].GetString());

    RapidJsonAccessor extendedData2 = rootNodes[1]->GetUsersExtendedData();
    ASSERT_TRUE(extendedData2.GetJson().IsObject());
    ASSERT_EQ(0, extendedData2.GetJson().MemberCount());
    }

/*---------------------------------------------------------------------------------**//**
* Based on VSTS#150682
* @bsitest                                      Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideNodeWhenItHasNoChildrenOrArtifacts_ShowsWhenThereAreChildren, R"*(
    <ECEntityClass typeName="Subject">
        <ECNavigationProperty propertyName="Parent" relationshipName="SubjectOwnsSubjects" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Partition" />
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="SubjectOwnsSubjects" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="Subject"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Subject"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="SubjectOwnsPartitionElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="Subject"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Partition"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ModelModelsElement" strength="embedding" strengthDirection="Backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="models" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is modeled by" polymorphic="true">
            <Class class="Partition" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, HideNodeWhenItHasNoChildrenOrArtifacts_ShowsWhenThereAreChildren)
    {
    ECClassCP subjectClass = GetClass("Subject");
    ECClassCP partitionClass = GetClass("Partition");
    ECClassCP modelClass = GetClass("Model");
    ECRelationshipClassCP relSubjectOwnsSubjects = GetClass("SubjectOwnsSubjects")->GetRelationshipClassCP();
    ECRelationshipClassCP relSubjectOwnsPartitionElements = GetClass("SubjectOwnsPartitionElements")->GetRelationshipClassCP();
    ECRelationshipClassCP relModelModelsElement = GetClass("ModelModelsElement")->GetRelationshipClassCP();
    IECInstancePtr subject1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *subjectClass);
    IECInstancePtr subject2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *subjectClass);
    IECInstancePtr partition = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *partitionClass);
    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relSubjectOwnsSubjects, *subject1, *subject2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relSubjectOwnsPartitionElements, *subject2, *partition);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relModelModelsElement, *model, *partition);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootSubjectsRule = new RootNodeRule();
    rules->AddPresentationRule(*rootSubjectsRule);
    InstanceNodesOfSpecificClassesSpecification* subjectsSpec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Parent = NULL", subjectClass->GetFullName(), true);
    subjectsSpec->SetHideExpression("NOT (ThisNode.HasChildren OR ThisNode.ChildrenArtifacts.AnyMatches(x => x.IsContentModelPartition))");
    rootSubjectsRule->AddSpecification(*subjectsSpec);

    ChildNodeRule* childSubjectsRule = new ChildNodeRule("ParentNode.ClassName = \"Subject\"", 1000, false, TargetTree_Both);
    childSubjectsRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, true, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", relSubjectOwnsSubjects->GetFullName(), subjectClass->GetFullName()));
    rules->AddPresentationRule(*childSubjectsRule);

    ChildNodeRule* partitionsRule = new ChildNodeRule("ParentNode.ClassName = \"Subject\"", 1000, false, TargetTree_Both);
    partitionsRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", relSubjectOwnsPartitionElements->GetFullName(), partitionClass->GetFullName()));
    rules->AddPresentationRule(*partitionsRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *subject1);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *partition);
    }

/*---------------------------------------------------------------------------------**//**
* Based on VSTS#150682
* @bsitest                                      Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideNodeWhenItHasNoChildrenOrArtifacts_HidesWhenThereAreNoChildren, R"*(
    <ECEntityClass typeName="Subject">
        <ECNavigationProperty propertyName="Parent" relationshipName="SubjectOwnsSubjects" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Partition" />
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="SubjectOwnsSubjects" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="Subject"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Subject"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="SubjectOwnsPartitionElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="Subject"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Partition"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ModelModelsElement" strength="embedding" strengthDirection="Backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="models" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is modeled by" polymorphic="true">
            <Class class="Partition" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, HideNodeWhenItHasNoChildrenOrArtifacts_HidesWhenThereAreNoChildren)
    {
    ECClassCP subjectClass = GetClass("Subject");
    ECClassCP partitionClass = GetClass("Partition");
    ECClassCP modelClass = GetClass("Model");
    ECRelationshipClassCP relSubjectOwnsSubjects = GetClass("SubjectOwnsSubjects")->GetRelationshipClassCP();
    ECRelationshipClassCP relSubjectOwnsPartitionElements = GetClass("SubjectOwnsPartitionElements")->GetRelationshipClassCP();
    ECRelationshipClassCP relModelModelsElement = GetClass("ModelModelsElement")->GetRelationshipClassCP();
    IECInstancePtr subject1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *subjectClass);
    IECInstancePtr subject2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *subjectClass);
    IECInstancePtr partition = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *partitionClass);
    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relSubjectOwnsSubjects, *subject1, *subject2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relSubjectOwnsPartitionElements, *subject2, *partition);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relModelModelsElement, *model, *partition);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootSubjectsRule = new RootNodeRule();
    rules->AddPresentationRule(*rootSubjectsRule);
    InstanceNodesOfSpecificClassesSpecification* subjectsSpec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Parent = NULL", subjectClass->GetFullName(), true);
    subjectsSpec->SetHideExpression("NOT (ThisNode.HasChildren OR ThisNode.ChildrenArtifacts.AnyMatches(x => x.IsContentModelPartition))");
    rootSubjectsRule->AddSpecification(*subjectsSpec);

    ChildNodeRule* childSubjectsRule = new ChildNodeRule("ParentNode.ClassName = \"Subject\"", 1000, false, TargetTree_Both);
    childSubjectsRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, true, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", relSubjectOwnsSubjects->GetFullName(), subjectClass->GetFullName()));
    rules->AddPresentationRule(*childSubjectsRule);

    ChildNodeRule* partitionsRule = new ChildNodeRule("ParentNode.ClassName = \"Subject\"", 1000, false, TargetTree_Both);
    partitionsRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, 0, "FALSE",
        RequiredRelationDirection_Forward, "", relSubjectOwnsPartitionElements->GetFullName(), partitionClass->GetFullName()));
    rules->AddPresentationRule(*partitionsRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* Based on VSTS#150682
* @bsitest                                      Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideNodeWhenItHasNoChildrenOrArtifacts_ShowsWhenThereAreNoChildrenButThereAreArtifacts, R"*(
    <ECEntityClass typeName="Subject">
        <ECNavigationProperty propertyName="Parent" relationshipName="SubjectOwnsSubjects" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Partition" />
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="SubjectOwnsSubjects" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="Subject"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Subject"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="SubjectOwnsPartitionElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="Subject"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Partition"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ModelModelsElement" strength="embedding" strengthDirection="Backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="models" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is modeled by" polymorphic="true">
            <Class class="Partition" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, HideNodeWhenItHasNoChildrenOrArtifacts_ShowsWhenThereAreNoChildrenButThereAreArtifacts)
    {
    ECClassCP subjectClass = GetClass("Subject");
    ECClassCP partitionClass = GetClass("Partition");
    ECClassCP modelClass = GetClass("Model");
    ECRelationshipClassCP relSubjectOwnsSubjects = GetClass("SubjectOwnsSubjects")->GetRelationshipClassCP();
    ECRelationshipClassCP relSubjectOwnsPartitionElements = GetClass("SubjectOwnsPartitionElements")->GetRelationshipClassCP();
    ECRelationshipClassCP relModelModelsElement = GetClass("ModelModelsElement")->GetRelationshipClassCP();
    IECInstancePtr subject1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *subjectClass);
    IECInstancePtr subject2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *subjectClass);
    IECInstancePtr partition = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *partitionClass);
    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relSubjectOwnsSubjects, *subject1, *subject2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relSubjectOwnsPartitionElements, *subject2, *partition);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relModelModelsElement, *model, *partition);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootSubjectsRule = new RootNodeRule();
    rules->AddPresentationRule(*rootSubjectsRule);
    InstanceNodesOfSpecificClassesSpecification* subjectsSpec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Parent = NULL", subjectClass->GetFullName(), true);
    subjectsSpec->SetHideExpression("NOT (ThisNode.HasChildren OR ThisNode.ChildrenArtifacts.AnyMatches(x => x.IsContentModelPartition))");
    rootSubjectsRule->AddSpecification(*subjectsSpec);

    ChildNodeRule* childSubjectsRule = new ChildNodeRule("ParentNode.ClassName = \"Subject\"", 1000, false, TargetTree_Both);
    childSubjectsRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, true, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", relSubjectOwnsSubjects->GetFullName(), subjectClass->GetFullName()));
    rules->AddPresentationRule(*childSubjectsRule);

    ChildNodeRule* partitionsRule = new ChildNodeRule("ParentNode.ClassName = \"Subject\"", 1000, false, TargetTree_Both);
    partitionsRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, true, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", relSubjectOwnsPartitionElements->GetFullName(), partitionClass->GetFullName()));
    rules->AddPresentationRule(*partitionsRule);

    bmap<Utf8String, Utf8String> artifactDefinitions;
    artifactDefinitions.Insert("IsContentModelPartition", "TRUE");
    partitionsRule->AddCustomizationRule(*new NodeArtifactsRule("", artifactDefinitions));

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *subject1);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(0, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CorrectHasChildrenFlagWhenChildSpecificationHasChildrenHintAlways, R"*(
    <ECEntityClass typeName="Element" />
    <ECEntityClass typeName="Subject" />
    <ECRelationshipClass typeName="ElementOwnsSubjects" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="false">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="false">
            <Class class="Subject"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CorrectHasChildrenFlagWhenChildSpecificationHasChildrenHintAlways)
    {
    ECClassCP elementClass = GetClass("Element");
    ECClassCP subjectClass = GetClass("Subject");
    ECRelationshipClassCP relElementOwndSubject = GetClass("ElementOwnsSubjects")->GetRelationshipClassCP();
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", elementClass->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"Element\"", 1000, false, TargetTree_Both);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Always, false, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", relElementOwndSubject->GetFullName(), subjectClass->GetFullName()));

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    NavNodeCPtr rootNode = rootNodes[0];
    EXPECT_FALSE(rootNode->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsByMultipleBaseClasses, R"*(
    <ECEntityClass typeName="Element" />
    <ECEntityClass typeName="GeometricElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="SpatialElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByMultipleBaseClasses)
    {
    ECClassCP base = GetClass("Element");
    ECClassCP geometricClass = GetClass("GeometricElement");
    ECClassCP physicalClass = GetClass("PhysicalElement");
    ECClassCP spatialClass = GetClass("SpatialElement");
    IECInstancePtr geometric1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *geometricClass);
    IECInstancePtr geometric2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *geometricClass);
    IECInstancePtr physical1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *physicalClass);
    IECInstancePtr physical2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *physicalClass);
    IECInstancePtr spatial1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *spatialClass);
    IECInstancePtr spatial2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *spatialClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", base->GetFullName(), true));

    GroupingRuleP geometricGroup = new GroupingRule("", 1, false, geometricClass->GetSchema().GetName(), geometricClass->GetName(), "", "", "");
    geometricGroup->AddGroup(*new ClassGroup("", true, "", ""));

    GroupingRuleP physicalGroup = new GroupingRule("", 1, false, physicalClass->GetSchema().GetName(), physicalClass->GetName(), "", "", "");
    physicalGroup->AddGroup(*new ClassGroup("", true, "", ""));

    GroupingRuleP spatialGroup = new GroupingRule("", 1, false, spatialClass->GetSchema().GetName(), spatialClass->GetName(), "", "", "");
    spatialGroup->AddGroup(*new ClassGroup("", true, "", ""));

    rootRule->AddCustomizationRule(*geometricGroup);
    rootRule->AddCustomizationRule(*physicalGroup);
    rootRule->AddCustomizationRule(*spatialGroup);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());

    NavNodeCPtr geometricGroupingNode = rootNodes[0];
    NavNodeCPtr physicalGroupingNode = rootNodes[1];
    NavNodeCPtr spatialGroupingNode = rootNodes[2];
    EXPECT_TRUE(nullptr != geometricGroupingNode->GetKey()->AsECClassGroupingNodeKey());
    EXPECT_TRUE(nullptr != physicalGroupingNode->GetKey()->AsECClassGroupingNodeKey());
    EXPECT_TRUE(nullptr != spatialGroupingNode->GetKey()->AsECClassGroupingNodeKey());

    DataContainer<NavNodeCPtr> geometricNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDbCR(), *geometricGroupingNode, PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(2, geometricNodes.GetSize());
    VerifyNodeInstance(*geometricNodes[0], *geometric1);
    VerifyNodeInstance(*geometricNodes[1], *geometric2);

    DataContainer<NavNodeCPtr> physicalNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDbCR(), *physicalGroupingNode, PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(2, physicalNodes.GetSize());
    VerifyNodeInstance(*physicalNodes[0], *physical1);
    VerifyNodeInstance(*physicalNodes[1], *physical2);

    DataContainer<NavNodeCPtr> spatialNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDbCR(), *spatialGroupingNode, PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(2, spatialNodes.GetSize());
    VerifyNodeInstance(*spatialNodes[0], *spatial1);
    VerifyNodeInstance(*spatialNodes[1], *spatial2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsByDerivedClassesWhenRequestingBaseClassInstances, R"*(
    <ECEntityClass typeName="Element" />
    <ECEntityClass typeName="DrawingElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement1">
        <BaseClass>PhysicalElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement2">
        <BaseClass>PhysicalElement</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByDerivedClassesWhenRequestingBaseClassInstances)
    {
    ECClassCP elementClass = GetClass("Element");
    ECClassCP drawingElementClass = GetClass("DrawingElement");
    ECClassCP physicalElementClass1 = GetClass("PhysicalElement1");
    ECClassCP physicalElementClass2 = GetClass("PhysicalElement2");
    IECInstancePtr drawingElement1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *drawingElementClass);
    IECInstancePtr physicalElement1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *physicalElementClass1);
    IECInstancePtr physicalElement2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *physicalElementClass2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", elementClass->GetFullName(), true));

    GroupingRuleP drawingGroup = new GroupingRule("", 1, false, drawingElementClass->GetSchema().GetName(), drawingElementClass->GetName(), "", "", "");
    drawingGroup->AddGroup(*new ClassGroup("", true, "", ""));

    GroupingRuleP physicalGroup1 = new GroupingRule("", 1, false, physicalElementClass1->GetSchema().GetName(), physicalElementClass1->GetName(), "", "", "");
    physicalGroup1->AddGroup(*new ClassGroup("", true, "", ""));

    GroupingRuleP physicalGroup2 = new GroupingRule("", 1, false, physicalElementClass2->GetSchema().GetName(), physicalElementClass2->GetName(), "", "", "");
    physicalGroup2->AddGroup(*new ClassGroup("", true, "", ""));

    rootRule->AddCustomizationRule(*drawingGroup);
    rootRule->AddCustomizationRule(*physicalGroup1);
    rootRule->AddCustomizationRule(*physicalGroup2);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());

    NavNodeCPtr drawingElementsGroupingNode = rootNodes[0];
    NavNodeCPtr physicalElements1GroupingNode = rootNodes[1];
    NavNodeCPtr physicalElements2GroupingNode = rootNodes[2];
    ASSERT_TRUE(nullptr != drawingElementsGroupingNode->GetKey()->AsECClassGroupingNodeKey());
    ASSERT_TRUE(nullptr != physicalElements1GroupingNode->GetKey()->AsECClassGroupingNodeKey());
    ASSERT_TRUE(nullptr != physicalElements2GroupingNode->GetKey()->AsECClassGroupingNodeKey());

    DataContainer<NavNodeCPtr> drawingElementNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDbCR(), *drawingElementsGroupingNode, PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, drawingElementNodes.GetSize());
    VerifyNodeInstance(*drawingElementNodes[0], *drawingElement1);

    DataContainer<NavNodeCPtr> physicalElement1Nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDbCR(), *physicalElements1GroupingNode, PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, physicalElement1Nodes.GetSize());
    VerifyNodeInstance(*physicalElement1Nodes[0], *physicalElement1);

    DataContainer<NavNodeCPtr> physicalElement2Nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDbCR(), *physicalElements2GroupingNode, PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, physicalElement2Nodes.GetSize());
    VerifyNodeInstance(*physicalElement2Nodes[0], *physicalElement2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_CreatesHierarchyFromGivenClasses, R"*(
    <ECEntityClass typeName="Element" />
    <ECEntityClass typeName="DrawingElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement1">
        <BaseClass>PhysicalElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement2">
        <BaseClass>PhysicalElement</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_CreatesHierarchyFromGivenClasses)
    {
    ECClassCP elementClass = GetClass("Element");
    ECClassCP drawingElementClass = GetClass("DrawingElement");
    ECClassCP physicalElementClass = GetClass("PhysicalElement");
    ECClassCP physicalElementClass1 = GetClass("PhysicalElement1");
    ECClassCP physicalElementClass2 = GetClass("PhysicalElement2");
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr drawingElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *drawingElementClass);
    IECInstancePtr physicalElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *physicalElementClass);
    IECInstancePtr physicalElement1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *physicalElementClass1);
    IECInstancePtr physicalElement2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *physicalElementClass2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", elementClass->GetFullName(), true));

    GroupingRuleP physicalElementsGroup = new GroupingRule("", 1, false, physicalElementClass->GetSchema().GetName(), physicalElementClass->GetName(), "", "", "");
    physicalElementsGroup->AddGroup(*new ClassGroup("", true, "", ""));
    rootRule->AddCustomizationRule(*physicalElementsGroup);

    GroupingRuleP drawingElementsGroup = new GroupingRule("", 1, false, drawingElementClass->GetSchema().GetName(), drawingElementClass->GetName(), "", "", "");
    drawingElementsGroup->AddGroup(*new ClassGroup("", true, "", ""));
    rootRule->AddCustomizationRule(*drawingElementsGroup);

    GroupingRuleP physicalElements1Group = new GroupingRule("", 1, false, physicalElementClass1->GetSchema().GetName(), physicalElementClass1->GetName(), "", "", "");
    physicalElements1Group->AddGroup(*new ClassGroup("", true, "", ""));
    rootRule->AddCustomizationRule(*physicalElements1Group);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());

    NavNodeCPtr drawingClassGroupingNode = rootNodes[0];
    ASSERT_TRUE(nullptr != drawingClassGroupingNode->GetKey()->AsECClassGroupingNodeKey());

    NavNodeCPtr physicalClassGroupingNode = rootNodes[1];
    ASSERT_TRUE(nullptr != physicalClassGroupingNode->GetKey()->AsECClassGroupingNodeKey());

    NavNodeCPtr elementNode = rootNodes[2];
    VerifyNodeInstance(*elementNode, *element);

    DataContainer<NavNodeCPtr> drawingClassGroupChildren = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDbCR(), *drawingClassGroupingNode, PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, drawingClassGroupChildren.GetSize());
    VerifyNodeInstance(*drawingClassGroupChildren[0], *drawingElement);

    DataContainer<NavNodeCPtr> physicalClassGroupChildren = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDbCR(), *physicalClassGroupingNode, PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(3, physicalClassGroupChildren.GetSize());
    ASSERT_TRUE(nullptr != physicalClassGroupChildren[0]->GetKey()->AsECClassGroupingNodeKey());
    VerifyNodeInstance(*physicalClassGroupChildren[1], *physicalElement);
    VerifyNodeInstance(*physicalClassGroupChildren[2], *physicalElement2);

    DataContainer<NavNodeCPtr> physical1ClassGroupChildren = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDbCR(), *physicalClassGroupChildren[0], PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, physical1ClassGroupChildren.GetSize());
    VerifyNodeInstance(*physical1ClassGroupChildren[0], *physicalElement1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_DateTimePropertyValue, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="DateTimeProp" typeName="dateTime" displayLabel="DateTime" description="DateTime property">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.1.0">
                    <DateTimeKind>Utc</DateTimeKind>
                </DateTimeInfo>
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_DateTimePropertyValue)
    {
    ECClassCP element = GetClass("Element");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *element, [](IECInstanceR instance) {instance.SetValue("DateTimeProp", ECValue(DateTime(2019, 11, 28))); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", element->GetFullName(), true));

    InstanceLabelOverride* labelOverrideRule = new InstanceLabelOverride(1, false, element->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("DateTimeProp") });
    rules->AddPresentationRule(*labelOverrideRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    NavNodeCPtr node = rootNodes[0];
    EXPECT_STREQ("2019-11-28T00:00:00.000Z", node->GetLabelDefinition().GetRawValue()->AsSimpleValue()->GetValue().GetString());
    EXPECT_STREQ("dateTime", node->GetLabelDefinition().GetTypeName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_DateTimeProperty, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="DateTimeProp" typeName="dateTime" displayLabel="DateTime" description="DateTime property">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.1.0">
                    <DateTimeKind>Utc</DateTimeKind>
                </DateTimeInfo>
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_DateTimeProperty)
    {
    ECClassCP element = GetClass("Element");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *element, [](IECInstanceR instance) {instance.SetValue("DateTimeProp", ECValue(DateTime(2019, 11, 28))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *element, [](IECInstanceR instance) {instance.SetValue("DateTimeProp", ECValue(DateTime(2019, 11, 28))); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", element->GetFullName(), true));

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", element->GetSchema().GetName(), element->GetName(), "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "DateTimeProp", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    NavNodeCPtr node = rootNodes[0];
    EXPECT_STREQ("2019-11-28T00:00:00.000Z", node->GetLabelDefinition().GetRawValue()->AsSimpleValue()->GetValue().GetString());
    EXPECT_STREQ("dateTime", node->GetLabelDefinition().GetTypeName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LabelOverride_OverrideWithNullValue, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="IntProperty" typeName="int" displayLabel="IntProperty" description="DateTime property">
        </ECProperty>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LabelOverride_OverrideWithNullValue)
    {
    ECClassCP element = GetClass("Element");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *element);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", element->GetFullName(), true));

    LabelOverride* labelOverride = new LabelOverride("", 1, "this.IntProperty", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    NavNodeCPtr node = rootNodes[0];
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), node->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_CreatesHierarchyWithOneToManyToOneRelationships, R"*(
    <ECEntityClass typeName="Model" />
    <ECEntityClass typeName="Element" />
    <ECEntityClass typeName="Category" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed" description="Associates Elements with a Model">
        <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementIsInCategory" strength="referencing" strengthDirection="Backward" modifier="Sealed" description="Element is in Category">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="Element" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="categorizes" polymorphic="false">
            <Class class="Category"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_CreatesHierarchyWithOneToManyToOneRelationships)
    {
    ECClassCP elementClass = GetClass("Element");
    ECClassCP modelClass = GetClass("Model");
    ECClassCP categoryClass = GetClass("Category");
    ECRelationshipClassCP relModelContainsElements = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP relElementIsInCategory = GetClass("ElementIsInCategory")->GetRelationshipClassCP();

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr category = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *categoryClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relModelContainsElements, *model, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relModelContainsElements, *model, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relElementIsInCategory, *element1, *category);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relElementIsInCategory, *element2, *category);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", modelClass->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"Model\"", 1000, false, TargetTree_Both);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Always, false, false, false, false, "",
        { new RepeatableRelationshipPathSpecification({
            new RepeatableRelationshipStepSpecification(relModelContainsElements->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, elementClass->GetFullName()),
            new RepeatableRelationshipStepSpecification(relElementIsInCategory->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, categoryClass->GetFullName())
            })
        })
    );

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get(); });
    EXPECT_EQ(1, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_CreatesHierarchyWithOneToManyToOneRelationships_WithNavigationProperties, R"*(
    <ECEntityClass typeName="Model" />
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
        <ECNavigationProperty propertyName="Category" relationshipName="ElementIsInCategory" direction="Forward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>
    <ECEntityClass typeName="Category" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed" description="Associates Elements with a Model">
        <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementIsInCategory" strength="referencing" strengthDirection="Backward" modifier="Sealed" description="Element is in Category">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="Element" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="categorizes" polymorphic="false">
            <Class class="Category"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_CreatesHierarchyWithOneToManyToOneRelationships_WithNavigationProperties)
    {
    ECClassCP elementClass = GetClass("Element");
    ECClassCP modelClass = GetClass("Model");
    ECClassCP categoryClass = GetClass("Category");
    ECRelationshipClassCP relModelContainsElements = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP relElementIsInCategory = GetClass("ElementIsInCategory")->GetRelationshipClassCP();

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr category = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *categoryClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass,
        [&](IECInstanceR inst) {
            inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), relModelContainsElements->GetId()));
            inst.SetValue("Category", ECValue(ECInstanceId::FromString(category->GetInstanceId().c_str()), relElementIsInCategory->GetId()));
        });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass,
        [&](IECInstanceR inst) {
            inst.SetValue("Model", ECValue(ECInstanceId::FromString(model->GetInstanceId().c_str()), relModelContainsElements->GetId()));
            inst.SetValue("Category", ECValue(ECInstanceId::FromString(category->GetInstanceId().c_str()), relElementIsInCategory->GetId()));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", modelClass->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"Model\"", 1000, false, TargetTree_Both);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Always, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(relModelContainsElements->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, elementClass->GetFullName()),
            new RepeatableRelationshipStepSpecification(relElementIsInCategory->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, categoryClass->GetFullName())
            })
        })
    );

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest());
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get(); });
    EXPECT_EQ(1, childNodes.GetSize());
    }
