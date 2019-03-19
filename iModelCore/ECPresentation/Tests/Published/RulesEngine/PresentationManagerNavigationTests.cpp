/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RulesEngine/PresentationManagerNavigationTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationManagerTests.h"
#include "../../../Source/RulesDriven/RulesEngine/LocalizationHelper.h"
#include "../../NonPublished/RulesEngine/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerNavigationTests : RulesDrivenECPresentationManagerTests
{
    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;
    ECSchemaCP m_schema;

    void SetUp() override
        {
        RulesDrivenECPresentationManagerTests::SetUp();
        m_manager->SetLocalizationProvider(new SQLangLocalizationProvider());
        m_schema = s_project->GetECDb().Schemas().GetSchema("RulesEngineTest");
        ASSERT_TRUE(nullptr != m_schema);

        m_widgetClass = m_schema->GetClassCP("Widget");
        m_gadgetClass = m_schema->GetClassCP("Gadget");
        m_sprocketClass = m_schema->GetClassCP("Sprocket");
        }
};

#ifdef WIP_DETERMINE_IF_USEFUL
/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static DataContainer<NavNodeCPtr> GetVerifiedNodes(std::function<DataContainer<NavNodeCPtr>()> getter)
    {
    // allow the test to verify the nodes
    DataContainer<NavNodeCPtr> initialNodes = getter();

    // additionally, make sure another get, which returns nodes from cache,
    // returns the same result
    DataContainer<NavNodeCPtr> cachedNodes = getter();

    EXPECT_EQ(initialNodes.GetSize(), cachedNodes.GetSize())
        << "Results of initial request (generator) should match results of second request (cached)";
    if (!initialNodes.GetSize(), cachedNodes.GetSize())
        return initialNodes;

    for (size_t i = 0; i < initialNodes.GetSize(); ++i)
        {
        EXPECT_TRUE(cachedNodes[i]->Equals(*initialNodes[i]))
            << "Nodes from initial request should match nodes from cached request";
        }

    return initialNodes;
    }
#endif

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
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, NavigationOptions_GetRuleSetId)
    {
    RulesDrivenECPresentationManager::NavigationOptions options("test id", RuleTargetTree::TargetTree_Both);
    ASSERT_STREQ("test id", options.GetRulesetId());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, NavigationOptions_GetTargetTree)
    {
    RulesDrivenECPresentationManager::NavigationOptions options("test id", RuleTargetTree::TargetTree_MainTree);
    ASSERT_EQ(RuleTargetTree::TargetTree_MainTree, options.GetRuleTargetTree());
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
    RulesDrivenECPresentationManager::NavigationOptions options("AllInstanceNodes_NotGrouped", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();

    // expect two nodes
    ASSERT_EQ(2, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    EXPECT_EQ(gadgetInstance->GetInstanceId(), nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());

    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[1]->GetType().c_str());
    EXPECT_EQ(widgetInstance->GetInstanceId(), nodes[1]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
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
    RulesDrivenECPresentationManager::NavigationOptions options("AllInstanceNodes_GroupedByClass", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_EQ(m_gadgetClass->GetId(), nodes[0]->GetKey()->AsECClassGroupingNodeKey()->GetECClassId());
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, gadgetNodes[0]->GetType().c_str());
    ASSERT_EQ(nodes[0]->GetNodeId(), gadgetNodes[0]->GetParentNodeId());
    EXPECT_EQ(gadgetInstance->GetInstanceId(), gadgetNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());

    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[1]->GetType().c_str());
    EXPECT_EQ(m_widgetClass->GetId(), nodes[1]->GetKey()->AsECClassGroupingNodeKey()->GetECClassId());
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[1], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetNodes[0]->GetType().c_str());
    ASSERT_EQ(nodes[1]->GetNodeId(), widgetNodes[0]->GetParentNodeId());
    EXPECT_EQ(widgetInstance->GetInstanceId(), widgetNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_AlwaysReturnsChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_DoNotSort_ReturnsUnsortedNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget2", instanceNode->GetLabel().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget1", instanceNode->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    ASSERT_STREQ("test", instanceNode->GetType().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_GroupedByLabel_DoesntGroup1Instance", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_GroupedByLabelGroups3InstancesWith1GroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("GadgetID", instanceNode->GetLabel().c_str());

    NavNodeCPtr labelGroupingNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", labelGroupingNode->GetLabel().c_str());

    //make sure we have 2 widget instances
    DataContainer<NavNodeCPtr> instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get();
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_GroupedByLabelGroups4InstancesWith2GroupingNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> labelGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, labelGroupingNodes.GetSize());

    NavNodeCPtr labelGroupingNode = labelGroupingNodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("GadgetID", labelGroupingNode->GetLabel().c_str());

    // make sure we have 2 gadget instances
    DataContainer<NavNodeCPtr> instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get();
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());

    labelGroupingNode = labelGroupingNodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", labelGroupingNode->GetLabel().c_str());

    // make sure we have 2 widget instances
    instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get();
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, RemovesLabelGroupingNodeIfOnlyOneChild)
    {
    // make sure there are instances with unique labels and instances with same labels
    IECInstancePtr instanceWithUniqueLabel = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("GetRootNodes_RemovesLabelGroupingNodeIfOnlyOneChild: Unique Label")); });
    IECInstancePtr instanceWithSameLabel = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("GetRootNodes_RemovesLabelGroupingNodeIfOnlyOneChild: Same Label")); });
    instanceWithSameLabel = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("GetRootNodes_RemovesLabelGroupingNodeIfOnlyOneChild: Same Label")); });
    instanceWithSameLabel = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass,
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("GetRootNodes_RemovesLabelGroupingNodeIfOnlyOneChild: Same Label")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RemovesLabelGroupingNodeIfOnlyOneChild", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();

    // make sure we have one instance node and one display label grouping node
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr labelGroupingNode = nodes[0];
    NavNodeCPtr instanceNode = nodes[1];

    ASSERT_TRUE(nullptr != instanceNode->GetKey()->AsECInstanceNodeKey());
    EXPECT_EQ(instanceWithUniqueLabel->GetInstanceId(), instanceNode->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());

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
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();

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
    RulesDrivenECPresentationManager::NavigationOptions options("HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();

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
    RulesDrivenECPresentationManager::NavigationOptions options("HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();

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
    RulesDrivenECPresentationManager::NavigationOptions options("HideIfNoChildren_IgnoredIfHasAlwaysReturnsNodesFlag", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();

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
    RulesDrivenECPresentationManager::NavigationOptions options("HideNodesInHierarchy_ReturnsChildNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();

    // make sure we get gadget, not widget
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_TRUE(nullptr != nodes[0]->GetKey()->AsECInstanceNodeKey());
    EXPECT_EQ(gadgetInstance->GetInstanceId(), nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
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
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_MainTree);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();

    // expect 1 widget node
    ASSERT_EQ(1, rootNodes.GetSize());

    // request for child nodes
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get();

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
    RulesDrivenECPresentationManager::NavigationOptions options(rules->GetRuleSetId().c_str(), TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(2), options.GetJson()).get();

    // expect 3 nodes: C, D, E
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("C", nodes[0]->GetLabel().c_str());
    EXPECT_STREQ("D", nodes[1]->GetLabel().c_str());
    EXPECT_STREQ("E", nodes[2]->GetLabel().c_str());
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
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_SkipsSpecifiedNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(2), options.GetJson()).get();

    // expect 3 nodes: C, D, E
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("C", nodes[0]->GetLabel().c_str());
    EXPECT_STREQ("D", nodes[1]->GetLabel().c_str());
    EXPECT_STREQ("E", nodes[2]->GetLabel().c_str());
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
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_SkippingMoreThanExists", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(5), options.GetJson()).get();

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
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_ReturnsSpecifiedNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(0, 2), options.GetJson()).get();

    // expect 2 nodes: A, B
    ASSERT_EQ(2, nodes.GetSize());
    EXPECT_STREQ("A", nodes[0]->GetLabel().c_str());
    EXPECT_STREQ("B", nodes[1]->GetLabel().c_str());
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
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_PageSizeHigherThanTheNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(0, 5), options.GetJson()).get();

    // expect 2 nodes: A, B
    ASSERT_EQ(2, nodes.GetSize());
    EXPECT_STREQ("A", nodes[0]->GetLabel().c_str());
    EXPECT_STREQ("B", nodes[1]->GetLabel().c_str());
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
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_PageSizeHigherThanTheNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(0, 2), options.GetJson()).get();

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
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_SkipsAndReturnsSpecifiedNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(1, 3), options.GetJson()).get();

    // expect 3 nodes: B, C, D
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("B", nodes[0]->GetLabel().c_str());
    EXPECT_STREQ("C", nodes[1]->GetLabel().c_str());
    EXPECT_STREQ("D", nodes[2]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CustomNodes_Type_Label_Description_ImageId", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    ASSERT_STREQ("type", instanceNode->GetType().c_str());
    ASSERT_STREQ("label", instanceNode->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CustomNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CustomNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideNodesInHierarchy_ReturnsEmptyListForRootNode)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("type", [](CustomNodeSpecificationR spec) {spec.SetHideNodesInHierarchy(true); }));
    rules->AddPresentationRule(*rootRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    ASSERT_EQ(1, IECPresentationManager::GetManager().GetRootNodesCount(s_project->GetECDb(), options).get());

    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("child2", childNodes[0]->GetType().c_str());
    EXPECT_STREQ("child3", childNodes[1]->GetType().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_AlwaysReturnsChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_HideNodesInHierarchy", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_GroupedByClass", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_GroupedByLabel_DoesntGroup1Instance", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_GroupedByLabel_Groups3InstancesWith1GroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget instance node
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("GadgetID", instanceNode->GetLabel().c_str());

    // make sure we have 1 widget label grouping node
    NavNodeCPtr labelGroupingNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", labelGroupingNode->GetLabel().c_str());

    //make sure we have 2 widget instances
    DataContainer<NavNodeCPtr> instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get();
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_GroupedByLabel_Groups4InstancesWith2GroupingNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> labelGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, labelGroupingNodes.GetSize());

    // make sure we have 2 gadget instances in gadget grouping node
    NavNodeCPtr labelGroupingNode = labelGroupingNodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("GadgetID", labelGroupingNode->GetLabel().c_str());
    DataContainer<NavNodeCPtr> instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get();
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());

    // make sure we have 2 widget instances in widget grouping node
    labelGroupingNode = labelGroupingNodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", labelGroupingNode->GetLabel().c_str());
    instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options).get();
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());
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
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, true, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_GroupedByClassAndByLabel", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 1 gadget instance node
    DataContainer<NavNodeCPtr> gadgetInstanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, gadgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, gadgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadget).c_str(), gadgetInstanceNodes[0]->GetLabel().c_str());

    // make sure we have 2 widget nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options).get();
    ASSERT_EQ(2, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, widgetNodes[0]->GetType().c_str());

    // make sure we have 2 Widget1 instance nodes
    DataContainer<NavNodeCPtr> widget1Nodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, widget1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widget1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget1", widget1Nodes[0]->GetLabel().c_str());

    // make sure we have 1 Widget2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetNodes[1]->GetType().c_str());
    EXPECT_STREQ("Widget2", widgetNodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_GroupedByClassAndByLabel", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 1 gadget instance node
    DataContainer<NavNodeCPtr> gadgetInstanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, gadgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, gadgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadget).c_str(), gadgetInstanceNodes[0]->GetLabel().c_str());

    // make sure we have 2 widget nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options).get();
    ASSERT_EQ(2, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, widgetNodes[0]->GetType().c_str());

    // make sure we have 2 Widget1 instance nodes
    DataContainer<NavNodeCPtr> widget1Nodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, widget1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widget1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget1", widget1Nodes[0]->GetLabel().c_str());

    // make sure we have 1 Widget2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetNodes[1]->GetType().c_str());
    EXPECT_STREQ("Widget2", widgetNodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_DoNotSort_ReturnsUnsortedNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget2", instanceNode->GetLabel().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget1", instanceNode->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_ArePolymorphic", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_AreNotPolymorphic", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstanceNodesOfSpecificClasses_InstanceFilter", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("child2", childNodes[0]->GetType().c_str());
    EXPECT_STREQ("child3", childNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_AlwaysReturnsChildren)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_AlwaysReturnsChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());

    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get();

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    NavNodeCPtr node = gadgetNodes[0];
    EXPECT_EQ(ChildrenHint::Always, NavNodeExtendedData(*node).GetChildrenHint());
    EXPECT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_HideNodesInHierarchy)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_HideNodesInHierarchy", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

     // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());

    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> customNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get();

    // make sure we have 1 custom node
    ASSERT_EQ(1, customNodes.GetSize());

    NavNodeCPtr instanceNode = customNodes[0];
    ASSERT_STREQ("test", instanceNode->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());

    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get();

    // make sure we have 0 gadget nodes
    ASSERT_EQ(0, gadgetNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_HideIfNoChildren_ReturnsNodesIfHasChildren)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get();

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_GroupedByClass)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_GroupedByClass", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get();

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_GroupedByLabel_DoesntGroup1Instance)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_GroupedByLabel_DoesntGroup1Instance", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ("GadgetID", gadgetNodes[0]->GetLabel().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get();

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetNodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", widgetNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadgetInstance).c_str(), gadgetNodes[0]->GetLabel().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 2 sprocket nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[0]->GetType().c_str());
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, sprocketNodes.GetSize());

    // make sure we have 1 widget node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_DoNotSort_ReturnsUnsortedNodes)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_DoNotSort_ReturnsUnsortedNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadgetInstance).c_str(), gadgetNodes[0]->GetLabel().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get();

    // make sure we have 2 sprocket nodes
    ASSERT_EQ(2, sprocketNodes.GetSize());

    ASSERT_STREQ("Sprocket2", sprocketNodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Sprocket1", sprocketNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_SkipRelatedLevel)
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

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_SkipRelatedLevel", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

     // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options).get();

    // make sure we have 1 sprocket node
    ASSERT_EQ(1, sprocketNodes.GetSize());
    NavNodeCPtr sprocketNode = sprocketNodes[0];
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*sprocketInstance).c_str(), sprocketNode->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#711486
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_SkipRelatedLevel_DoesntDuplicateNodesWhenSkippingMultipleDifferentInstancesWithTheSameEndpointInstance)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

     // make sure we have 1 widget root node
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeCPtr widget1Node = rootNodes[0];
    ECInstanceId widgetId;
    ECInstanceId::FromString(widgetId, widget1->GetInstanceId().c_str());
    ASSERT_EQ(ECInstanceKey(widget1->GetClass().GetId(), widgetId), widget1Node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());

    // make sure we have 1 widget child node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widget1Node, PageOptions(), options).get();
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr widget2Node = childNodes[0];
    ECInstanceId widget2Id;
    ECInstanceId::FromString(widget2Id, widget2->GetInstanceId().c_str());
    ASSERT_EQ(ECInstanceKey(widget2->GetClass().GetId(), widget2Id), widget2Node->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_InstanceFilter)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_InstanceFilter", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadgetInstance).c_str(), gadgetNodes[0]->GetLabel().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get();

    // make sure we have 1 sprocket node
    ASSERT_EQ(1, sprocketNodes.GetSize());
    ASSERT_STREQ("Sprocket1", sprocketNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstancesNodes_InstanceFilter_AppliesOnlyToMatchingInstances, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_InstanceFilter_AppliesOnlyToMatchingInstances)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(2, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_GroupedByLabel_GroupsByClassAndByLabel)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_GroupedByLabel_GroupsByClassAndByLabel", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get();

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 2 sprocket nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, sprocketNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, sprocketNodes[0]->GetType().c_str());

    // make sure we have 2 Sprocket1 instance nodes
    DataContainer<NavNodeCPtr> sprocket1Nodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *sprocketNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, sprocket1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, sprocket1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("Sprocket1", sprocket1Nodes[0]->GetLabel().c_str());

    // make sure we have 1 Sprocket2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, sprocketNodes[1]->GetType().c_str());
    EXPECT_STREQ("Sprocket2", sprocketNodes[1]->GetLabel().c_str());

    // make sure we have 1 widget instance node
    DataContainer<NavNodeCPtr> widgetInstanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options).get();
    ASSERT_EQ(1, widgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widgetInstance).c_str(), widgetInstanceNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_GroupedByLabel_GroupsByClassAndByLabel_InstanceLabelOverride)
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_GroupedByLabel_GroupsByClassAndByLabel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_GroupedByLabel_GroupsByClassAndByLabel", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options).get();

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 2 sprocket nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, sprocketNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, sprocketNodes[0]->GetType().c_str());

    // make sure we have 2 Sprocket1 instance nodes
    DataContainer<NavNodeCPtr> sprocket1Nodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *sprocketNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, sprocket1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, sprocket1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("Sprocket1", sprocket1Nodes[0]->GetLabel().c_str());

    // make sure we have 1 Sprocket2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, sprocketNodes[1]->GetType().c_str());
    EXPECT_STREQ("Sprocket2", sprocketNodes[1]->GetLabel().c_str());

    // make sure we have 1 widget instance node
    DataContainer<NavNodeCPtr> widgetInstanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options).get();
    ASSERT_EQ(1, widgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widgetInstance).c_str(), widgetInstanceNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstancesNodes_FindsAllMixinSubclassesAndCustomizesNodesCorrectly, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_FindsAllMixinSubclassesAndCustomizesNodesCorrectly)
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(BeTest::GetNameOfCurrentTest(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_STREQ("test label", childNodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), widgetNodes[0]->GetLabel().c_str());

    // make sure we have 1 custom node
    DataContainer<NavNodeCPtr> customNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, customNodes.GetSize());
    EXPECT_STREQ("test", customNodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_GroupedByClass", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 instance node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_GroupedByLabel_Groups3InstancesWith1GroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("GadgetID", nodes[0]->GetLabel().c_str());

    // make sure we have 2 widget instance nodes with label grouping node
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[1]->GetType().c_str());
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[1], PageOptions(), options).get();
    ASSERT_EQ(2, widgetNodes.GetSize());
    ASSERT_STREQ("WidgetID", widgetNodes[0]->GetLabel().c_str());
    ASSERT_STREQ("WidgetID", widgetNodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_GroupedByLabel_Groups4InstancesWith2GroupingNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 2 gadget instance nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[0]->GetType().c_str());
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_STREQ("GadgetID", gadgetNodes[0]->GetLabel().c_str());
    ASSERT_STREQ("GadgetID", gadgetNodes[1]->GetLabel().c_str());

    // make sure we have 2 widget instance nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[1]->GetType().c_str());
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[1], PageOptions(), options).get();
    ASSERT_EQ(2, widgetNodes.GetSize());
    ASSERT_STREQ("WidgetID", widgetNodes[0]->GetLabel().c_str());
    ASSERT_STREQ("WidgetID", widgetNodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_DoNotSort_ReturnsUnsortedNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget2", instanceNode->GetLabel().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget1", instanceNode->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("ImageIdOverride", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LabelOverride", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node with overrided Label & Description
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedLabel", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LabelOverrideWithGroupedInstancesCountOnClassGroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // verify there're two grouping nodes with correct labels
    ASSERT_EQ(2, nodes.GetSize());
    ASSERT_STREQ("Count: 2", nodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Count: 3", nodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // verify there're three grouping nodes with correct labels
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Count: 1", nodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Count: 2", nodes[1]->GetLabel().c_str());
    ASSERT_STREQ("Count: 3", nodes[2]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNodeSortedByPropertyValue", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // verify there're three grouping nodes with correct labels
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Count: 2", nodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Count: 3", nodes[1]->GetLabel().c_str());
    ASSERT_STREQ("Count: 1", nodes[2]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("StyleOverride", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LocalizationResourceKeyDefinition_WithExistingKey", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("T35T", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LocalizationResourceKeyDefinition_KeyNotFound", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("notfound", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CheckBoxRule_UsesDefaultValueIfPropertyIsNull", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget node
    ASSERT_STREQ("GadgetID", nodes[0]->GetLabel().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 widget node with CheckBoxRule
    ASSERT_STREQ("WidgetID", nodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CheckBoxRule_WithoutProperty", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("customLabel", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CheckBoxRule_UsesInversedPropertyName", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget node
    ASSERT_STREQ("GadgetID", nodes[0]->GetLabel().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 widget node with CheckBoxRule
    ASSERT_STREQ("WidgetID", nodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CheckBoxRule_DoesNotUseInversedPropertyName", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget node
    ASSERT_STREQ("GadgetID", nodes[0]->GetLabel().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 widget node with CheckBoxRule
    ASSERT_STREQ("WidgetID", nodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_SortingAscending", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 3 nodes sorted ascending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Widget2", nodes[1]->GetLabel().c_str());
    ASSERT_STREQ("Widget3", nodes[2]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_SortingAscending", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 3 nodes sorted ascending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabel().c_str());
    ASSERT_STREQ("widget2", nodes[1]->GetLabel().c_str());
    ASSERT_STREQ("Widget3", nodes[2]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_SortingDescending", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 node sorted descending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget3", nodes[0]->GetLabel().c_str());
    ASSERT_STREQ("widget2", nodes[1]->GetLabel().c_str());
    ASSERT_STREQ("Widget1", nodes[2]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_DoNotSort", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 3 unsorted nodes
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget2", nodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Widget3", nodes[1]->GetLabel().c_str());
    ASSERT_STREQ("Widget1", nodes[2]->GetLabel().c_str());
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

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "ClassE", "IntProperty", true, false, true);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_SortingAscendingPolymorphically", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 4 sorted ascending ClassE & ClassF nodes
    ASSERT_EQ(4, nodes.GetSize());

    EXPECT_EQ(instance3->GetInstanceId(), nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance3).c_str(), nodes[0]->GetLabel().c_str());

    EXPECT_EQ(instance1->GetInstanceId(), nodes[1]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), nodes[1]->GetLabel().c_str());

    EXPECT_EQ(instance4->GetInstanceId(), nodes[2]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance4).c_str(), nodes[2]->GetLabel().c_str());

    EXPECT_EQ(instance2->GetInstanceId(), nodes[3]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), nodes[3]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_SortingByTwoProperties", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 3 nodes sorted ascending by IntProperty & descending by DoubleProperty
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_EQ(instance3->GetInstanceId(), nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
    EXPECT_EQ(instance2->GetInstanceId(), nodes[1]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
    EXPECT_EQ(instance1->GetInstanceId(), nodes[2]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes sorted correctly by enum display value
    ASSERT_EQ(2, nodes.GetSize());
    EXPECT_EQ(instance2->GetInstanceId(), nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
    EXPECT_EQ(instance1->GetInstanceId(), nodes[1]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString());
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

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", false, "RulesEngineTest", "ClassE");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_ClassGroup_GroupsByBaseClass", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", nodes[0]->GetLabel().c_str());

    // make sure we have 2 classE child nodes
    DataContainer<NavNodeCPtr> classEChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classEChildNodes[0]->GetType().c_str());
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceF).c_str(), classEChildNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classEChildNodes[1]->GetType().c_str());
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceG).c_str(), classEChildNodes[1]->GetLabel().c_str());
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

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", false, "RulesEngineTest", "ClassE");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_ClassGroup_DoesNotCreateGroupForSingleItem", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 ClassF node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceF).c_str(), nodes[0]->GetLabel().c_str());
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

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", true, "RulesEngineTest", "ClassE");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_ClassGroup_CreatesGroupForSingleItem", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", nodes[0]->GetLabel().c_str());

    // make sure we have 1 ClassF node
    DataContainer<NavNodeCPtr> classFNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, classFNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classFNodes[0]->GetType().c_str());
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceF).c_str(), classFNodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 Widget node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 Widget node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 widget property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_SetImageIdForGroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsByNullProperty", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr node = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node->GetType().c_str());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), node->GetLabel().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsPolymorphically", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassD_Label", nodes[0]->GetLabel().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, children[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[1]->GetType().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("My Value", nodes[0]->GetLabel().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, children.GetSize());
    for (size_t i = 0; i < 2; ++i)
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[i]->GetType().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 property grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("A", nodes[0]->GetLabel().c_str());

    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[1]->GetType().c_str());
    ASSERT_STREQ("Z", nodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 property grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("1", nodes[0]->GetLabel().c_str());

    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[1]->GetType().c_str());
    ASSERT_STREQ("3", nodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsByNullForeignKey", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsByRelationshipProperty", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // expect 1 widget node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();

    // expect 1 child property grouping node
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    ASSERT_STREQ("5", childNodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // expect 1 ClassS node
    ASSERT_EQ(1, rootNodes.GetSize());
    ECInstanceId sId;
    ECInstanceId::FromString(sId, instanceS->GetInstanceId().c_str());
    EXPECT_EQ(ECInstanceKey(instanceS->GetClass().GetId(), sId), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();

    // expect 1 child property grouping node with label of instanceU
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Label 3", childNodes[0]->GetLabel().c_str());

    // request for grandchildren
    DataContainer<NavNodeCPtr> grandchildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get();

    // expect 1 grandchild instance node
    ASSERT_EQ(1, grandchildNodes.GetSize());
    ECInstanceId tId;
    ECInstanceId::FromString(tId, instanceT->GetInstanceId().c_str());
    EXPECT_EQ(ECInstanceKey(instanceT->GetClass().GetId(), tId), grandchildNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // expect 1 ClassS node
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*instanceS), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();

    // expect 1 child property grouping node with label of instanceU
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("3", childNodes[0]->GetLabel().c_str());

    // request for grandchildren
    DataContainer<NavNodeCPtr> grandchildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get();

    // expect 1 grandchild instance node
    ASSERT_EQ(1, grandchildNodes.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*instanceT), grandchildNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKey());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsBySkippedRelationshipProperty", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // expect 1 widget node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();

    // expect 1 child property grouping node
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    ASSERT_STREQ("5", childNodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // expect 1 ClassD node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ(instanceD->GetInstanceId().c_str(), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());

    // expect 1 child property grouping node
    DataContainer<NavNodeCPtr> childNodes1 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, childNodes1.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes1[0]->GetType().c_str());
    ASSERT_STREQ("5", childNodes1[0]->GetLabel().c_str());

    // expect 2 child instance nodes under the grouping node
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes1[0], PageOptions(), options).get();
    ASSERT_EQ(2, childNodes2.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes2[0]->GetType().c_str());
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceF).c_str(), childNodes2[0]->GetLabel().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes2[1]->GetType().c_str());
    ASSERT_STREQ(CommonTools::GetDefaultDisplayLabel(*instanceG).c_str(), childNodes2[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
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
    SameLabelInstanceGroupP sameLabelInstanceGroup = new SameLabelInstanceGroup("");
    groupingRule->AddGroup(*sameLabelInstanceGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_SameLabelInstanceGroup", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 widget instance node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_SameLabelInstanceGroup", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 widget instance node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_GroupsByProperty_WithRanges", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Other()).c_str(), nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), children[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // expect 1 gadget node
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadget).c_str(), rootNodes[0]->GetLabel().c_str());

    // request children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("1 - 5", childNodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_ClassGroup_GroupsByBaseClassAndByInstancesClasses", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", nodes[0]->GetLabel().c_str());

    // make sure we have 1 classE child nodes
    DataContainer<NavNodeCPtr> classEChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classEChildNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassF", classEChildNodes[0]->GetLabel().c_str());
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

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassD", false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassD,ClassE");
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*relatedNodeRule);

    // make sure we have 1 ClassD root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("DoesNotReturnECInstanceNodesOfTheSameSpecificationAlreadyExistingInHierarchy", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*classDInstance).c_str(), rootNodes[0]->GetLabel().c_str());

    // ClassD instance node should have 1 ClassE instance child node
    DataContainer<NavNodeCPtr> classDChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, classDChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classDChildNodes[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*classEInstance).c_str(), classDChildNodes[0]->GetLabel().c_str());

    // ClassE instance node has 1 ClassD instance node. There's such a node already in the hierarchy, but it's based on
    // different (root node rule) specification, so we allow it
    DataContainer<NavNodeCPtr> classEChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classDChildNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classEChildNodes[0]->GetType().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*classDInstance).c_str(), rootNodes[0]->GetLabel().c_str());

    // This time ClassD instance node has no children because there's such a node up in the
    // hierarchy and it's also based on the same specification.
    DataContainer<NavNodeCPtr> classDChildNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classEChildNodes[0], PageOptions(), options).get();
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());

    // the node should have 1 Gadget child node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ(gadget->GetInstanceId().c_str(), childNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(primaryRules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    EXPECT_STREQ("Test", rootNodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // the node should have 1 property grouping node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());

    // the child node should have 1 Gadget child node
    DataContainer<NavNodeCPtr> grandChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, grandChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, grandChildNodes[0]->GetType().c_str());
    EXPECT_STREQ(gadget->GetInstanceId().c_str(), grandChildNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());

    // the node should have 1 property grouping node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());

    // the child node should have 1 Gadget child node
    DataContainer<NavNodeCPtr> grandChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, grandChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, grandChildNodes[0]->GetType().c_str());
    EXPECT_STREQ(gadget->GetInstanceId().c_str(), grandChildNodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[1]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[1]->GetType().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("a", rootNodes[0]->GetLabel().c_str());

    // make sure it has 2 child nodes
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("b", childNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("c", childNodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadget).c_str(), childNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node as well
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, childNodes2.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*sprocket).c_str(), childNodes2[0]->GetLabel().c_str());
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

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsOfClass(\"Widget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.Description", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), rootNodes[0]->GetLabel().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*widget).c_str(), rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*gadget).c_str(), childNodes[0]->GetLabel().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options).get();
    ASSERT_EQ(0, childNodes2.GetSize());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2.00", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2.50", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("0.01", rootNodes[0]->GetLabel().c_str());

    //make sure it has 3 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(3, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[1]->GetLabel().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[2]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("2.50", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("2.51", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get();
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", secondNodeChildrenNodes[0]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2.60", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("WidgetID", childrenNodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("2.59", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("2.60", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get();
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("WidgetID", secondNodeChildrenNodes[0]->GetLabel().c_str());
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

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabel().c_str());
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

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabel().c_str());
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

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.11 Y: 1.11 Z: 1.11", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get();
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), secondNodeChildrenNodes[0]->GetLabel().c_str());;
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

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.11 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get();
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), secondNodeChildrenNodes[0]->GetLabel().c_str());;
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

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.11 Z: 1.12", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get();
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), secondNodeChildrenNodes[0]->GetLabel().c_str());;
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

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.11", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options).get();
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), secondNodeChildrenNodes[0]->GetLabel().c_str());;
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

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabel().c_str());
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

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabel().c_str());
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

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options).get();
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance1).c_str(), childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(CommonTools::GetDefaultDisplayLabel(*instance2).c_str(), childrenNodes[1]->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    bvector<NodesPathElement> nodes = IECPresentationManager::GetManager().GetFilteredNodesPaths(s_project->GetECDb(), "", options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    bvector<NodesPathElement> nodes = IECPresentationManager::GetManager().GetFilteredNodesPaths(s_project->GetECDb(), "%", options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    bvector<NodesPathElement> nodes = IECPresentationManager::GetManager().GetFilteredNodesPaths(s_project->GetECDb(), "_", options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    bvector<NodesPathElement> nodes = IECPresentationManager::GetManager().GetFilteredNodesPaths(s_project->GetECDb(), "%", options).get();

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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    bvector<NodesPathElement> nodes = IECPresentationManager::GetManager().GetFilteredNodesPaths(s_project->GetECDb(), "_", options).get();

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.size());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget2", instanceNode->GetLabel().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Widget1", instanceNode->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();

    // make sure we have 1 nodes
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("WidgetID", instanceNode->GetLabel().c_str());
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
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    IGNORE_BE_ASSERT();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options).get();
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
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest(), TargetTree_MainTree);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("ClassA1_CodeValue", rootNodes[0]->GetLabel().c_str());

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("ClassC_CodeValue", childNodes[0]->GetLabel().c_str());
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
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest(), TargetTree_MainTree);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("ClassA1_CodeValue", rootNodes[0]->GetLabel().c_str());

    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options.GetJson()).get();
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("ClassC_UserLabel", childNodes[0]->GetLabel().c_str());
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
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest(), TargetTree_MainTree);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("ClassC_CodeValue", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("ClassA1_CodeValue", rootNodes[1]->GetLabel().c_str());
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
    RulesDrivenECPresentationManager::NavigationOptions options(BeTest::GetNameOfCurrentTest(), TargetTree_MainTree);
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).get();
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("1_Instance_C", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("2_Instance_A", rootNodes[1]->GetLabel().c_str());
    }
