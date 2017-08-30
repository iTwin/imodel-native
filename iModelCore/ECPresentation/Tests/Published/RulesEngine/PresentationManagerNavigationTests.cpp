/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RulesEngine/PresentationManagerNavigationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

        if (!s_project->GetECDb().GetDefaultTransaction()->IsActive())
            s_project->GetECDb().GetDefaultTransaction()->Begin();

        m_schema = s_project->GetECDb().Schemas().GetSchema("RulesEngineTest");
        ASSERT_TRUE(nullptr != m_schema);
        
        m_widgetClass = m_schema->GetClassCP("Widget");
        m_gadgetClass = m_schema->GetClassCP("Gadget");
        m_sprocketClass = m_schema->GetClassCP("Sprocket");
        }

    void TearDown() override
        {
        s_project->GetECDb().GetDefaultTransaction()->Cancel();
        RulesDrivenECPresentationManagerTests::TearDown();
        }
};

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
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_NotGrouped", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("AllInstanceNodes_NotGrouped", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // expect two nodes
    ASSERT_EQ(2, nodes.GetSize());
    for (NavNodeCPtr const& node : nodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, node->GetType().c_str());
        ASSERT_TRUE(node->GetInstance().IsValid());

        if (m_widgetClass == &node->GetInstance()->GetClass())  
            EXPECT_STREQ(widgetInstance->GetInstanceId().c_str(), node->GetInstance()->GetInstanceId().c_str());
        else
            EXPECT_STREQ(gadgetInstance->GetInstanceId().c_str(), node->GetInstance()->GetInstanceId().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByClass)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_GroupedByClass", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("AllInstanceNodes_GroupedByClass", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    for (size_t i = 0; i < classGroupingNodes.GetSize(); i++)
        {
        NavNodeCPtr classGroupingNode = classGroupingNodes[i];
        bool isWidget = (1 == i);
        bool isGadget = (0 == i);

        NavNodeExtendedData extendedData(*classGroupingNode);
        EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNode->GetType().c_str());
        
        ASSERT_TRUE(extendedData.HasECClassId());
        if (isWidget)
            EXPECT_EQ(m_widgetClass->GetId(), extendedData.GetECClassId());
        else if (isGadget)
            EXPECT_EQ(m_gadgetClass->GetId(), extendedData.GetECClassId());
        
        // request for children of those class grouping nodes
        DataContainer<NavNodeCPtr> instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNode, PageOptions(), options.GetJson());
        
        // make sure we get what we expect
        ASSERT_EQ(1, instanceNodes.GetSize());
        NavNodeCPtr instanceNode = instanceNodes[0];
        EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
        ASSERT_TRUE(instanceNode->GetInstance().IsValid());
        ASSERT_EQ(classGroupingNode->GetNodeId(), instanceNode->GetParentNodeId());

        if (isWidget)  
            EXPECT_STREQ(widgetInstance->GetInstanceId().c_str(), instanceNode->GetInstance()->GetInstanceId().c_str());
        else if (isGadget)
            EXPECT_STREQ(gadgetInstance->GetInstanceId().c_str(), instanceNode->GetInstance()->GetInstanceId().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_AlwaysReturnsChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_AlwaysReturnsChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_AlwaysReturnsChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr node = nodes[0];
    ASSERT_TRUE(NavNodeExtendedData(*node).GetAlwaysReturnsChildren());
    ASSERT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_DoNotSort_ReturnsUnsortedNodes)
    {    
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_DoNotSort_ReturnsUnsortedNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_DoNotSort_ReturnsUnsortedNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, true, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, true, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->GetSpecificationsR().push_back(customNodeSpecification);
    allInstanceNodesSpecification->GetNestedRules().push_back(childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideNodesInHierarchy)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_HideNodesInHierarchy", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, true, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->GetSpecificationsR().push_back(customNodeSpecification);
    allInstanceNodesSpecification->GetNestedRules().push_back(childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_HideNodesInHierarchy", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_GroupedByLabel_DoesntGroup1Instance", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_GroupedByLabel_DoesntGroup1Instance", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget", nodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByLabelGroups3InstancesWith1GroupingNode)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_GroupedByLabelGroups3InstancesWith1GroupingNode", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_GroupedByLabelGroups3InstancesWith1GroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Gadget", instanceNode->GetLabel().c_str());

    NavNodeCPtr labelGroupingNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("Widget", labelGroupingNode->GetLabel().c_str());

    //make sure we have 2 widget instances
    DataContainer<NavNodeCPtr> instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options);
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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AllInstanceNodes_GroupedByLabelGroups4InstancesWith2GroupingNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("AllInstanceNodes_GroupedByLabelGroups4InstancesWith2GroupingNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> labelGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, labelGroupingNodes.GetSize());

    NavNodeCPtr labelGroupingNode = labelGroupingNodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("Gadget", labelGroupingNode->GetLabel().c_str());
    
    // make sure we have 2 gadget instances
    DataContainer<NavNodeCPtr> instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options);
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());

    labelGroupingNode = labelGroupingNodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("Widget", labelGroupingNode->GetLabel().c_str());

    // make sure we have 2 widget instances
    instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options);
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
    IECInstancePtr instanceWithUniqueLabel = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, 
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("GetRootNodes_RemovesLabelGroupingNodeIfOnlyOneChild: Unique Label")); });
    IECInstancePtr instanceWithSameLabel = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, 
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("GetRootNodes_RemovesLabelGroupingNodeIfOnlyOneChild: Same Label")); });
    instanceWithSameLabel = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, 
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("GetRootNodes_RemovesLabelGroupingNodeIfOnlyOneChild: Same Label")); });
    instanceWithSameLabel = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, 
        [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("GetRootNodes_RemovesLabelGroupingNodeIfOnlyOneChild: Same Label")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RemovesLabelGroupingNodeIfOnlyOneChild", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    
    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("RemovesLabelGroupingNodeIfOnlyOneChild", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
        
    // make sure we have one instance node and one display label grouping node
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nullptr;
    NavNodeCPtr labelGroupingNode = nullptr;
    if (nodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstanceNode))
        {
        instanceNode = nodes[0];
        labelGroupingNode = nodes[1];
        }
    else
        {
        instanceNode = nodes[1];
        labelGroupingNode = nodes[0];
        }

    ASSERT_TRUE(instanceNode->GetInstance().IsValid());
    ASSERT_STREQ(instanceWithUniqueLabel->GetInstanceId().c_str(), instanceNode->GetInstance()->GetInstanceId().c_str());

    ASSERT_TRUE(labelGroupingNode->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    ASSERT_TRUE(labelGroupingNode->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, AlwaysReturnsResultsFlag_SetToNodeFromSpecification)
    {
    // insert a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AlwaysReturnsResultsFlag", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rule->GetSpecificationsR().push_back(new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("AlwaysReturnsResultsFlag", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());

    // make sure the parent node has the "always returns results" flag
    ASSERT_EQ(2, nodes.GetSize());
    ASSERT_TRUE(NavNodeExtendedData(*nodes[0]).GetAlwaysReturnsChildren());
    ASSERT_FALSE(NavNodeExtendedData(*nodes[1]).GetAlwaysReturnsChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("HideIfNoChildren_ReturnsEmptyListIfNoChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    childRule->SetCondition("ParentNode.IsInstanceNode And ParentNode.ClassName = \"Widget\"");
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());

    // make sure the node was hidden
    ASSERT_TRUE(0 == nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    

    // insert a gadget
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("HideIfNoChildren_ReturnsNodesIfHasChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    childRule->SetCondition("ParentNode.IsInstanceNode And ParentNode.ClassName = \"Widget\"");
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());

    // make sure the node was not hidden 
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideIfNoChildren_IgnoredIfHasAlwaysReturnsNodesFlag)
    {
    // insert a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
        
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("HideIfNoChildren_IgnoredIfHasAlwaysReturnsNodesFlag", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, true, false, true, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("HideIfNoChildren_IgnoredIfHasAlwaysReturnsNodesFlag", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());
    
    // make sure the node was not hidden
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideNodesInHierarchy_ReturnsChildNodes)
    {    
    // add a widget
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // add a gadget    
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("HideNodesInHierarchy_ReturnsChildNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    childRule->SetCondition("ParentNode.IsInstanceNode");
    rules->AddPresentationRule(*childRule);

    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("HideNodesInHierarchy_ReturnsChildNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson());

    // make sure we get gadget, not widget
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_TRUE(nodes[0]->GetInstance().IsValid());
    ASSERT_STREQ(gadgetInstance->GetInstanceId().c_str(), nodes[0]->GetInstance()->GetInstanceId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsSpecifiedNumberOfNodes)
    {
    RulesEngineTestHelpers::DeleteInstances(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Paging_SkipsSpecifiedNumberOfNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_SkipsSpecifiedNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(2), options.GetJson());
    
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
    RulesEngineTestHelpers::DeleteInstances(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Paging_SkippingMoreThanExists", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_SkippingMoreThanExists", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(5), options.GetJson());
    
    // expect 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    ASSERT_TRUE(nodes[0].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_ReturnsSpecifiedNumberOfNodes)
    {
    RulesEngineTestHelpers::DeleteInstances(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Paging_ReturnsSpecifiedNumberOfNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_ReturnsSpecifiedNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(0, 2), options.GetJson());
    
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
    RulesEngineTestHelpers::DeleteInstances(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Paging_PageSizeHigherThanTheNumberOfNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_PageSizeHigherThanTheNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(0, 5), options.GetJson());
    
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
    RulesEngineTestHelpers::DeleteInstances(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Paging_PageSizeHigherThanTheNumberOfNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_PageSizeHigherThanTheNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(0, 2), options.GetJson());
    
    // expect nullptr
    EXPECT_TRUE(nodes[2].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsAndReturnsSpecifiedNumberOfNodes)
    {
    RulesEngineTestHelpers::DeleteInstances(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Paging_SkipsAndReturnsSpecifiedNumberOfNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    RulesDrivenECPresentationManager::NavigationOptions options("Paging_SkipsAndReturnsSpecifiedNumberOfNodes", TargetTree_MainTree);
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(1, 3), options.GetJson());
    
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CustomNodes_Type_Label_Description_ImageId", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "label", "description", "imageid");
    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);
    
    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CustomNodes_Type_Label_Description_ImageId", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CustomNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, true, "type", "label", "description", "imageid");
    customNodeSpecification->SetAlwaysReturnsChildren(false);

    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecificationChild = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->GetSpecificationsR().push_back(customNodeSpecificationChild);
    customNodeSpecification->GetNestedRules().push_back(childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CustomNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 0 nodes
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CustomNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, true, "type", "label", "description", "imageid");
    customNodeSpecification->SetAlwaysReturnsChildren(false);

    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CustomNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_AlwaysReturnsChildren_NodeHasChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CustomNodes_AlwaysReturnsChildren_NodeHasChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "label", "description", "imageid");
    customNodeSpecification->SetAlwaysReturnsChildren(true);
    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    ASSERT_EQ(1, IECPresentationManager::GetManager().GetRootNodesCount(s_project->GetECDb(), options));

    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr node = nodes[0];
    EXPECT_TRUE(node->HasChildren());

    rapidjson::Document nodeJson = node->AsJson();
    ASSERT_TRUE(nodeJson.HasMember(NAVNODE_HasChildren));
    ASSERT_TRUE(nodeJson[NAVNODE_HasChildren].GetBool());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_AlwaysReturnsChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_AlwaysReturnsChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_AlwaysReturnsChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr node = nodes[0];
    ASSERT_TRUE(NavNodeExtendedData(*node).GetAlwaysReturnsChildren());
    ASSERT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_HideNodesInHierarchy)
    {
    // insert some widget instance
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_HideNodesInHierarchy", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->GetSpecificationsR().push_back(customNodeSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_HideNodesInHierarchy", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    ASSERT_STREQ("test", instanceNode->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert widget instance
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert widget instance
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->GetSpecificationsR().push_back(customNodeSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_GroupedByClass)
    {
    // insert some widget & gadget instances
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_GroupedByClass", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_GroupedByClass", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    
    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_GroupedByLabel_DoesntGroup1Instance)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_GroupedByLabel_DoesntGroup1Instance", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_GroupedByLabel_DoesntGroup1Instance", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget", nodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget instance node
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Gadget", instanceNode->GetLabel().c_str());

    // make sure we have 1 widget label grouping node
    NavNodeCPtr labelGroupingNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("Widget", labelGroupingNode->GetLabel().c_str());

    //make sure we have 2 widget instances
    DataContainer<NavNodeCPtr> instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options);
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_GroupedByLabel_Groups4InstancesWith2GroupingNodes)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_GroupedByLabel_Groups4InstancesWith2GroupingNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_GroupedByLabel_Groups4InstancesWith2GroupingNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> labelGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, labelGroupingNodes.GetSize());

    // make sure we have 2 gadget instances in gadget grouping node
    NavNodeCPtr labelGroupingNode = labelGroupingNodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("Gadget", labelGroupingNode->GetLabel().c_str());
    DataContainer<NavNodeCPtr> instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options);
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());

    // make sure we have 2 widget instances in widget grouping node
    labelGroupingNode = labelGroupingNodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("Widget", labelGroupingNode->GetLabel().c_str());
    instanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *labelGroupingNode, PageOptions(), options);
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_GroupedByClassAndByLabel)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget1")); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget1")); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("MyID", ECValue("Widget2")); });

    // insert gadget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_GroupedByClassAndByLabel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, true, false, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_GroupedByClassAndByLabel", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 1 gadget instance node
    DataContainer<NavNodeCPtr> gadgetInstanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options);
    ASSERT_EQ(1, gadgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, gadgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ("Gadget", gadgetInstanceNodes[0]->GetLabel().c_str());

    // make sure we have 2 widget nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options);
    ASSERT_EQ(2, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, widgetNodes[0]->GetType().c_str());

    // make sure we have 2 Widget1 instance nodes
    DataContainer<NavNodeCPtr> widget1Nodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNodes[0], PageOptions(), options);
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
TEST_F (RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_ShowEmptyGroups)
    {
    // insert some widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_ShowEmptyGroups", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, true, "", "RulesEngineTest:Widget,Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_ShowEmptyGroups", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    
    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_DoNotSort_ReturnsUnsortedNodes)
    {    
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_DoNotSort_ReturnsUnsortedNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    instanceNodesOfSpecificClassesSpecification->SetDoNotSort(true);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_DoNotSort_ReturnsUnsortedNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_ArePolymorphic)
    {    
    // insert some ClassE & ClassF instances
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    IECInstancePtr classEInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *classE);
    IECInstancePtr classFInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *classF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_ArePolymorphic", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_ArePolymorphic", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_AreNotPolymorphic)
    {    
    // insert some ClassE & ClassF instances
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    IECInstancePtr classEInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *classE);
    IECInstancePtr classFInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *classF);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_AreNotPolymorphic", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_AreNotPolymorphic", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClassesNodes_InstanceFilter)
    {    
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(10)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(5)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("InstancesOfSpecificClassesNodes_InstanceFilter", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "this.IntProperty<=5", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("InstancesOfSpecificClassesNodes_InstanceFilter", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_AlwaysReturnsChildren)
    {
    // insert widget & gadget instances with relationship
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
   
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_AlwaysReturnsChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, true, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    childRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_AlwaysReturnsChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());

    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options);

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    NavNodeCPtr node = gadgetNodes[0];
    ASSERT_TRUE(NavNodeExtendedData(*node).GetAlwaysReturnsChildren());
    ASSERT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_HideNodesInHierarchy)
    {
    // insert widget & gadget instances with relationship
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_HideNodesInHierarchy", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, true, false, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    ChildNodeRule* customNodeRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    customNodeRule->GetSpecificationsR().push_back(customNodeSpecification);
    relatedInstanceNodesSpecification->GetNestedRules().push_back(customNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_HideNodesInHierarchy", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

     // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());

    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> customNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options);

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
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, true, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());

    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options);

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
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, true, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    ChildNodeRule* customNodeRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    customNodeRule->GetSpecificationsR().push_back(customNodeSpecification);
    relatedInstanceNodesSpecification->GetNestedRules().push_back(customNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options);

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
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_GroupedByClass", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_GroupedByClass", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    
    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options);

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
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_GroupedByLabel_DoesntGroup1Instance", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, true, false, 0, "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_GroupedByLabel_DoesntGroup1Instance", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ("Gadget", gadgetNodes[0]->GetLabel().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options);

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetNodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget", widgetNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode)
    {        
    // insert widget, gadget & sprocket instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass);
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, true, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "RulesEngineTest:Widget,Sprocket");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ("Gadget", gadgetNodes[0]->GetLabel().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options);

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 2 sprocket nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[0]->GetType().c_str());
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
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
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket2"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_DoNotSort_ReturnsUnsortedNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Sprocket\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "RulesEngineTest:Widget,Sprocket");
    relatedInstanceNodesSpecification->SetDoNotSort(true);
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_DoNotSort_ReturnsUnsortedNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ("Gadget", gadgetNodes[0]->GetLabel().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options);

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
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass);
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_SkipRelatedLevel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_SkipRelatedLevel", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

     // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    NavNodeCPtr widgetNode = widgetNodes[0];
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNode, PageOptions(), options);

    // make sure we have 1 sprocket node
    ASSERT_EQ(1, sprocketNodes.GetSize());
    NavNodeCPtr sprocketNode = sprocketNodes[0];
    ASSERT_STREQ("Sprocket", sprocketNode->GetLabel().c_str());
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
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetsHaveGadgets, *widget2, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetsHaveGadgets, *widget2, *gadget2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_SkipRelatedLevel_DoesntDuplicateNodesWhenSkippingMultipleDifferentInstancesWithTheSameEndpointInstance", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "this.IntProperty = 1", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childrenRule = new ChildNodeRule("ParentNode.IsOfClass(\"Widget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childrenRule->GetSpecificationsR().push_back(new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, 
        "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

     // make sure we have 1 widget root node
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeCPtr widget1Node = rootNodes[0];
    ASSERT_EQ(*ECInstanceNodeKey::Create(*widget1), widget1Node->GetKey());

    // make sure we have 1 widget child node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widget1Node, PageOptions(), options);
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr widget2Node = childNodes[0];
    ASSERT_EQ(*ECInstanceNodeKey::Create(*widget2), widget2Node->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_InstanceFilter)
    {    
    // insert some gadget & sprocket instances with relationships
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket123"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_InstanceFilter", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Sprocket\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "this.MyID~\"Sprocket1\"", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket");
    relatedInstanceNodesSpecification->SetDoNotSort(true);
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_InstanceFilter", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    ASSERT_STREQ("Gadget", gadgetNodes[0]->GetLabel().c_str());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options);

    // make sure we have 1 sprocket node
    ASSERT_EQ(1, sprocketNodes.GetSize());
    ASSERT_STREQ("Sprocket1", sprocketNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstancesNodes_GroupedByLabel_GroupsByClassAndByLabel)
    {    
    // insert widget, gadget & sprocket instances with relationships
    ECRelationshipClassCR relationshipWidgetHasGadget = *m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadgetInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipWidgetHasGadget, *widgetInstance, *gadgetInstance);
    ECClassCP sprocketClass = m_schema->GetClassCP("Sprocket");
    IECInstancePtr sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    ECRelationshipClassCR relationshipGadgetHasSprockets = *m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket1"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);
    sprocketInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Sprocket2"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationshipGadgetHasSprockets, *gadgetInstance, *sprocketInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RelatedInstancesNodes_GroupedByLabel_GroupsByClassAndByLabel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Sprocket\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, true, false, true, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "RulesEngineTest:Widget,Sprocket");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->GetNestedRules().push_back(relatedNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RelatedInstancesNodes_GroupedByLabel_GroupsByClassAndByLabel", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 gadget node
    ASSERT_EQ(1, gadgetNodes.GetSize());
    NavNodeCPtr gadgetNode = gadgetNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *gadgetNode, PageOptions(), options);

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 2 sprocket nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> sprocketNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[0], PageOptions(), options);
    ASSERT_EQ(2, sprocketNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, sprocketNodes[0]->GetType().c_str());

    // make sure we have 2 Sprocket1 instance nodes
    DataContainer<NavNodeCPtr> sprocket1Nodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *sprocketNodes[0], PageOptions(), options);
    ASSERT_EQ(2, sprocket1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, sprocket1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("Sprocket1", sprocket1Nodes[0]->GetLabel().c_str());

    // make sure we have 1 Sprocket2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, sprocketNodes[1]->GetType().c_str());
    EXPECT_STREQ("Sprocket2", sprocketNodes[1]->GetLabel().c_str());
    
    // make sure we have 1 widget instance node
    DataContainer<NavNodeCPtr> widgetInstanceNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classGroupingNodes[1], PageOptions(), options);
    ASSERT_EQ(1, widgetInstanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, widgetInstanceNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", widgetInstanceNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SearchResultInstances_HideIfNoChildren_ReturnsNodesIfHasChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, true, false, false);
    searchResultInstanceSpecification->GetQuerySpecificationsR().push_back(new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->GetSpecificationsR().push_back(searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->GetSpecificationsR().push_back(customNodeSpecification);
    searchResultInstanceSpecification->GetNestedRules().push_back(childRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_HideIfNoChildren_ReturnsNodesIfHasChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 widget node
    ASSERT_EQ(1, widgetNodes.GetSize());
    EXPECT_STREQ("Widget", widgetNodes[0]->GetLabel().c_str());

    // make sure we have 1 custom node
    DataContainer<NavNodeCPtr> customNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *widgetNodes[0], PageOptions(), options);
    ASSERT_EQ(1, customNodes.GetSize());
    EXPECT_STREQ("test", customNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SearchResultInstances_HideIfNoChildren_ReturnsEmptyListIfNoChildren", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, true, false, false);
    searchResultInstanceSpecification->GetQuerySpecificationsR().push_back(new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->GetSpecificationsR().push_back(searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_HideIfNoChildren_ReturnsEmptyListIfNoChildren", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByClass)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SearchResultInstances_GroupedByClass", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, true, false);
    searchResultInstanceSpecification->GetQuerySpecificationsR().push_back(new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->GetSpecificationsR().push_back(searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_GroupedByClass", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> classGroupingNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SearchResultInstances_GroupedByLabel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, true);
    searchResultInstanceSpecification->GetQuerySpecificationsR().push_back(new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->GetSpecificationsR().push_back(searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_GroupedByLabel", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SearchResultInstances_GroupedByLabel_Groups3InstancesWith1GroupingNode", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, true);
    searchResultInstanceSpecification->GetQuerySpecificationsR().push_back(new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    searchResultInstanceSpecification->GetQuerySpecificationsR().push_back(new StringQuerySpecification("SELECT * FROM RulesEngineTest.Gadget", "RulesEngineTest", "Gadget"));
    rule->GetSpecificationsR().push_back(searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_GroupedByLabel_Groups3InstancesWith1GroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 gadget instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Gadget", nodes[0]->GetLabel().c_str());

    // make sure we have 2 widget instance nodes with label grouping node
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[1]->GetType().c_str());
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[1], PageOptions(), options);
    ASSERT_EQ(2, widgetNodes.GetSize());
    ASSERT_STREQ("Widget", widgetNodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Widget", widgetNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByLabel_Groups4InstancesWith2GroupingNodes)
    {
    // insert some widget & gadget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SearchResultInstances_GroupedByLabel_Groups4InstancesWith2GroupingNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, true);
    searchResultInstanceSpecification->GetQuerySpecificationsR().push_back(new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    searchResultInstanceSpecification->GetQuerySpecificationsR().push_back(new StringQuerySpecification("SELECT * FROM RulesEngineTest.Gadget", "RulesEngineTest", "Gadget"));
    rule->GetSpecificationsR().push_back(searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_GroupedByLabel_Groups4InstancesWith2GroupingNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 2 gadget instance nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[0]->GetType().c_str());
    DataContainer<NavNodeCPtr> gadgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
    ASSERT_STREQ("Gadget", gadgetNodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Gadget", gadgetNodes[1]->GetLabel().c_str());

    // make sure we have 2 widget instance nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[1]->GetType().c_str());
    DataContainer<NavNodeCPtr> widgetNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[1], PageOptions(), options);
    ASSERT_EQ(2, widgetNodes.GetSize());
    ASSERT_STREQ("Widget", widgetNodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Widget", widgetNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_DoNotSort_ReturnsUnsortedNodes)
    {    
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SearchResultInstances_DoNotSort_ReturnsUnsortedNodes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    searchResultInstanceSpecification->SetDoNotSort(true);
    searchResultInstanceSpecification->GetQuerySpecificationsR().push_back(new StringQuerySpecification("SELECT * FROM RulesEngineTest.Widget", "RulesEngineTest", "Widget"));
    rule->GetSpecificationsR().push_back(searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SearchResultInstances_DoNotSort_ReturnsUnsortedNodes", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ImageIdOverride", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");
    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);

    ImageIdOverrideP imageIdOverride = new ImageIdOverride("ThisNode.Type=\"customType\"", 1, "\"overridedImageId\"");
    rules->AddPresentationRule(*imageIdOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("ImageIdOverride", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("LabelOverride", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");
    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.Type=\"customType\"", 1, "\"overridedLabel\"", "\"overridedDescription\"");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LabelOverride", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("LabelOverrideWithGroupedInstancesCountOnClassGroupingNode", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", "RulesEngineTest:Widget,Gadget", true);
    rule->GetSpecificationsR().push_back(spec);
    rules->AddPresentationRule(*rule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsClassGroupingNode", 1, "\"Count: \" & ThisNode.GroupedInstancesCount", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LabelOverrideWithGroupedInstancesCountOnClassGroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNode", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true);
    rule->GetSpecificationsR().push_back(spec);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->GetGroupsR().push_back(groupingSpec);
    rules->AddPresentationRule(*groupingRule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsPropertyGroupingNode", 1, "\"Count: \" & ThisNode.GroupedInstancesCount", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(*s_project, inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNode", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true);
    rule->GetSpecificationsR().push_back(spec);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingSpec->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->GetGroupsR().push_back(groupingSpec);
    rules->AddPresentationRule(*groupingRule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsPropertyGroupingNode", 1, "\"Count: \" & ThisNode.GroupedInstancesCount", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("StyleOverride", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");
    
    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);

    StyleOverrideP styleOverride = new StyleOverride("ThisNode.Type=\"customType\"", 1, "\"overridedForeColor\"", "\"overridedBackColor\"", "\"overridedFontStyle\"");
    rules->AddPresentationRule(*styleOverride);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("StyleOverride", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("LocalizationResourceKeyDefinition_WithExistingKey", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "@customLabel@", "description", "imageId");
    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);

    LocalizationResourceKeyDefinitionP localizationRecourceKeyDefinition = new LocalizationResourceKeyDefinition(1, "customLabel", "RulesEngine:Test", "notfound");
    rules->AddPresentationRule(*localizationRecourceKeyDefinition);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LocalizationResourceKeyDefinition_WithExistingKey", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("LocalizationResourceKeyDefinition_KeyNotFound", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "@customLabel@", "description", "imageId");
    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);

    LocalizationResourceKeyDefinitionP localizationRecourceKeyDefinition = new LocalizationResourceKeyDefinition(1, "customLabel", "key", "notfound");
    rules->AddPresentationRule(*localizationRecourceKeyDefinition);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("LocalizationResourceKeyDefinition_KeyNotFound", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CheckBoxRule_UsesDefaultValueIfPropertyIsNull", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"Widget\"", 1, false, "BoolProperty", false, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CheckBoxRule_UsesDefaultValueIfPropertyIsNull", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    
    // make sure we have 1 gadget node
    ASSERT_STREQ("Gadget", nodes[0]->GetLabel().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 widget node with CheckBoxRule
    ASSERT_STREQ("Widget", nodes[1]->GetLabel().c_str());
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CheckBoxRule_WithoutProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "customLabel", "description", "imageId");
    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"customLabel\"", 1, false, "", false, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CheckBoxRule_WithoutProperty", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("BoolProperty", ECValue(false));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("BoolProperty", ECValue(false));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CheckBoxRule_UsesInversedPropertyName", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"Widget\"", 1, false, "BoolProperty", true, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CheckBoxRule_UsesInversedPropertyName", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    
    // make sure we have 1 gadget node
    ASSERT_STREQ("Gadget", nodes[0]->GetLabel().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 widget node with CheckBoxRule
    ASSERT_STREQ("Widget", nodes[1]->GetLabel().c_str());
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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("BoolProperty", ECValue(false));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("BoolProperty", ECValue(false));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("CheckBoxRule_DoesNotUseInversedPropertyName", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"Widget\"", 1, false, "BoolProperty", false, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("CheckBoxRule_DoesNotUseInversedPropertyName", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    
    // make sure we have 1 gadget node
    ASSERT_STREQ("Gadget", nodes[0]->GetLabel().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 widget node with CheckBoxRule
    ASSERT_STREQ("Widget", nodes[1]->GetLabel().c_str());
    ASSERT_TRUE(nodes[1]->IsCheckboxVisible());
    ASSERT_TRUE(nodes[1]->IsCheckboxEnabled());
    ASSERT_FALSE(nodes[1]->IsChecked());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingAscending)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget3"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SortingRule_SortingAscending", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "MyID", true, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_SortingAscending", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 3 nodes sorted ascending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Widget2", nodes[1]->GetLabel().c_str());
    ASSERT_STREQ("Widget3", nodes[2]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingDescending)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget3"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SortingRule_SortingDescending", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "MyID", false, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_SortingDescending", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node sorted descending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Widget3", nodes[0]->GetLabel().c_str());
    ASSERT_STREQ("Widget2", nodes[1]->GetLabel().c_str());
    ASSERT_STREQ("Widget1", nodes[2]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_DoNotSort)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget3"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SortingRule_DoNotSort", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "DoNotSortByWidgetLabel", false, true, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_DoNotSort", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *classE, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(2)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classE, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(4)); });
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classF, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(1)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classF, [](IECInstanceR instance) { instance.SetValue("IntProperty", ECValue(3)); });
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SortingRule_SortingAscendingPolymorphically", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "ClassE", "IntProperty", true, false, true);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_SortingAscendingPolymorphically", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 4 sorted ascending ClassE & ClassF nodes
    ASSERT_EQ(4, nodes.GetSize());

    ECValue value;

    ASSERT_STREQ("ClassF", nodes[0]->GetLabel().c_str());
    nodes[0]->GetInstance()->GetValue(value, "IntProperty");
    ASSERT_EQ(1, value.GetInteger());

    ASSERT_STREQ("ClassE", nodes[1]->GetLabel().c_str());
    nodes[1]->GetInstance()->GetValue(value, "IntProperty");
    ASSERT_EQ(2, value.GetInteger());

    ASSERT_STREQ("ClassF", nodes[2]->GetLabel().c_str());
    nodes[2]->GetInstance()->GetValue(value, "IntProperty");
    ASSERT_EQ(3, value.GetInteger());

    ASSERT_STREQ("ClassE", nodes[3]->GetLabel().c_str());
    nodes[3]->GetInstance()->GetValue(value, "IntProperty");
    ASSERT_EQ(4, value.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingByTwoProperties)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("DoubleProperty", ECValue(1.0));
        });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("DoubleProperty", ECValue(2.0));
        });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("DoubleProperty", ECValue(4.0));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SortingRule_SortingByTwoProperties", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "IntProperty", true, false, false);
    rules->AddPresentationRule(*sortingRule);
    sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "DoubleProperty", false, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("SortingRule_SortingByTwoProperties", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 3 nodes sorted ascending by IntProperty & descending by DoubleProperty
    ASSERT_EQ(3, nodes.GetSize());

    ECValue value;
    nodes[0]->GetInstance()->GetValue(value, "IntProperty");
    ASSERT_EQ(1, value.GetInteger());
    nodes[0]->GetInstance()->GetValue(value, "DoubleProperty");
    ASSERT_EQ(4, value.GetDouble());

    nodes[1]->GetInstance()->GetValue(value, "IntProperty");
    ASSERT_EQ(1, value.GetInteger());
    nodes[1]->GetInstance()->GetValue(value, "DoubleProperty");
    ASSERT_EQ(2, value.GetDouble());

    nodes[2]->GetInstance()->GetValue(value, "IntProperty");
    ASSERT_EQ(2, value.GetInteger());
    nodes[2]->GetInstance()->GetValue(value, "DoubleProperty");
    ASSERT_EQ(1, value.GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingByEnumProperty)
    {
    // insert some instances
    ECEntityClassCP classQ = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassQ")->GetEntityClassCP();
    RulesEngineTestHelpers::InsertInstance(*s_project, *classQ, [](IECInstanceR instance)
        {
        instance.SetValue("IntEnum", ECValue(1));
        });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classQ, [](IECInstanceR instance)
        {
        instance.SetValue("IntEnum", ECValue(3));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SortingRule_SortingByEnumProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "ClassQ", "IntEnum", true, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 2 nodes sorted correctly by enum display value
    ASSERT_EQ(2, nodes.GetSize());
    ECValue v;

    nodes[0]->GetInstance()->GetValue(v, "IntEnum");
    EXPECT_EQ(3, v.GetInteger());

    nodes[1]->GetInstance()->GetValue(v, "IntEnum");
    EXPECT_EQ(1, v.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByBaseClass)
    {
    // insert some widget instances
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    ECClassCP classG = m_schema->GetClassCP("ClassG");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classG);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_ClassGroup_GroupsByBaseClass", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", false, "RulesEngineTest", "ClassE");
    groupingRule->GetGroupsR().push_back(classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_ClassGroup_GroupsByBaseClass", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", nodes[0]->GetLabel().c_str());

    // make sure we have 2 classE child nodes
    DataContainer<NavNodeCPtr> classEChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
    ASSERT_EQ(2, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classEChildNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassF", classEChildNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classEChildNodes[1]->GetType().c_str());
    ASSERT_STREQ("ClassG", classEChildNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_DoesNotCreateGroupForSingleItem)
    {
    // insert ClassF instance
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_ClassGroup_DoesNotCreateGroupForSingleItem", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", false, "RulesEngineTest", "ClassE");
    groupingRule->GetGroupsR().push_back(classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_ClassGroup_DoesNotCreateGroupForSingleItem", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 ClassF node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassF", nodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_CreatesGroupForSingleItem)
    {
    // insert ClassF instance
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_ClassGroup_CreatesGroupForSingleItem", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", true, "RulesEngineTest", "ClassE");
    groupingRule->GetGroupsR().push_back(classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_ClassGroup_CreatesGroupForSingleItem", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", nodes[0]->GetLabel().c_str());

    // make sure we have 1 ClassF node
    DataContainer<NavNodeCPtr> classFNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
    ASSERT_EQ(1, classFNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classFNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassF", classFNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", false, "MyID", "");
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 Widget node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Widget1", nodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_CreatesGroupForSingleItem)
    {
    // insert widget instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_CreatesGroupForSingleItem", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "MyID", "");
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_CreatesGroupForSingleItem", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_SetImageIdForGroupingNode", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "changedImageId", true, "MyID", "");
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_SetImageIdForGroupingNode", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *classE);
    RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsByNullProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->GetGroupsR().push_back(new PropertyGroup("", "", true, "IntProperty", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsByNullProperty", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr node = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node->GetType().c_str());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_NotSpecified()).c_str(), node->GetLabel().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
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
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(*s_project, *classD);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(*s_project, *classE);
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *classDHasClassE, *instanceD, *instanceE);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *classDHasClassE, *instanceD, *instanceF);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsPolymorphically", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->GetGroupsR().push_back(new PropertyGroup("", "", true, "ClassD", ""));
    rules->AddPresentationRule(*groupingRule);

    GroupingRuleP groupingRule2 = new GroupingRule("", 1, false, "RulesEngineTest", "ClassF", "", "", "");
    groupingRule2->GetGroupsR().push_back(new PropertyGroup("", "", true, "IntProperty", ""));
    rules->AddPresentationRule(*groupingRule2);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsPolymorphically", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassD", nodes[0]->GetLabel().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
    ASSERT_EQ(2, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, children[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByIntegerEnumAsGroupingValue)
    {
    ECClassCP classQ = m_schema->GetClassCP("ClassQ");

    RulesEngineTestHelpers::InsertInstance(*s_project, *classQ, [](IECInstanceR instance){instance.SetValue("IntEnum", ECValue(3));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *classQ, [](IECInstanceR instance){instance.SetValue("IntEnum", ECValue(1));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsByIntegerEnumAsGroupingValue", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassQ", false));
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassQ", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntEnum", "");
    propertyGroup->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    propertyGroup->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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

    RulesEngineTestHelpers::InsertInstance(*s_project, *classQ, [](IECInstanceR instance){instance.SetValue("StrEnum", ECValue("Three"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *classQ, [](IECInstanceR instance){instance.SetValue("StrEnum", ECValue("One"));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsByStringEnumAsDisplayLabel", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassQ", false));
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassQ", "", "", "");
    groupingRule->GetGroupsR().push_back(new PropertyGroup("", "", true, "StrEnum", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *classE);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsByNullForeignKey", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->GetGroupsR().push_back(new PropertyGroup("", "", true, "ClassD", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsByNullForeignKey", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *widget, *gadget, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsByRelationshipProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->GetSpecificationsR().push_back(new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childNodeRule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "WidgetHasGadget", "", "", "");
    groupingRule->GetGroupsR().push_back(new PropertyGroup("", "", true, "Priority", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsByRelationshipProperty", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // expect 1 widget node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);

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
    IECInstancePtr instanceS = RulesEngineTestHelpers::InsertInstance(*s_project, *classS, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instanceT = RulesEngineTestHelpers::InsertInstance(*s_project, *classT, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instanceU = RulesEngineTestHelpers::InsertInstance(*s_project, *classU, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(3));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel_st, *instanceS, *instanceT, [&](IECInstanceR instance)
        {
        instance.SetValue("InstanceU", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceU).GetInstanceId(), rel_stu));
        });
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsByRelationshipNavigationProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassS", true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->GetSpecificationsR().push_back(new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:ClassSHasClassT", "RulesEngineTest:ClassT"));
    rules->AddPresentationRule(*childNodeRule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassSHasClassT", "", "", "");
    groupingRule->GetGroupsR().push_back(new PropertyGroup("", "", true, "InstanceU", ""));
    rules->AddPresentationRule(*groupingRule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.ClassName=\"ClassU\"", 1, "\"Label \" & this.IntProperty", "");
    rules->AddPresentationRule(*labelOverride);

    // request for root nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // expect 1 ClassS node
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_EQ(*ECInstanceNodeKey::Create(*instanceS), rootNodes[0]->GetKey());

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);

    // expect 1 child property grouping node with label of instanceU
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Label 3", childNodes[0]->GetLabel().c_str());
    
    // request for grandchildren
    DataContainer<NavNodeCPtr> grandchildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options);

    // expect 1 grandchild instance node
    ASSERT_EQ(1, grandchildNodes.GetSize());
    EXPECT_EQ(*ECInstanceNodeKey::Create(*instanceT), grandchildNodes[0]->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsBySkippedRelationshipProperty)
    {
    ECRelationshipClassCP rel1 = m_schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(*s_project, *m_sprocketClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel1, *widget, *gadget, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel2, *gadget, *sprocket);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsBySkippedRelationshipProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->GetSpecificationsR().push_back(new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket"));
    rules->AddPresentationRule(*childNodeRule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "WidgetHasGadget", "", "", "");
    groupingRule->GetGroupsR().push_back(new PropertyGroup("", "", true, "Priority", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_PropertyGroup_GroupsBySkippedRelationshipProperty", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // expect 1 widget node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());

    // request for children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);

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
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(*s_project, *classD);
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    IECInstancePtr instanceG = RulesEngineTestHelpers::InsertInstance(*s_project, *classG);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *instanceD, *instanceF, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *instanceD, *instanceG, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});
        
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_PropertyGroup_GroupsMultipleClassesByTheSameRelationshipProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:ClassD", false));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->GetSpecificationsR().push_back(new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:ClassDReferencesClassE", "RulesEngineTest:ClassF,ClassG"));
    rules->AddPresentationRule(*childNodeRule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassDReferencesClassE", "", "", "");
    groupingRule->GetGroupsR().push_back(new PropertyGroup("", "", true, "Priority", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // expect 1 ClassD node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ(instanceD->GetInstanceId().c_str(), rootNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());

    // expect 1 child property grouping node
    DataContainer<NavNodeCPtr> childNodes1 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, childNodes1.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes1[0]->GetType().c_str());
    ASSERT_STREQ("5", childNodes1[0]->GetLabel().c_str());

    // expect 2 child instance nodes under the grouping node
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes1[0], PageOptions(), options);
    ASSERT_EQ(2, childNodes2.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes2[0]->GetType().c_str());
    ASSERT_STREQ("ClassF", childNodes2[0]->GetLabel().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes2[1]->GetType().c_str());
    ASSERT_STREQ("ClassG", childNodes2[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup)
    {
    // insert some widget instances
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_SameLabelInstanceGroup", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    SameLabelInstanceGroupP sameLabelInstanceGroup = new SameLabelInstanceGroup("");
    groupingRule->GetGroupsR().push_back(sameLabelInstanceGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_SameLabelInstanceGroup", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

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
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(7));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_GroupsByProperty_WithRanges", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("", "", "1", "5");
    propertyGroup->GetRangesR().push_back(propertyRangeGroupSpecification);
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_GroupsByProperty_WithRanges", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ(RulesEngineL10N::GetString(RulesEngineL10N::LABEL_General_Other()).c_str(), nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ("Widget", children[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByIntegersRange)
    {
    // insert an instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(4));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_GroupsByProperty_WithRanges_FilteringByIntegersRange", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "1", "5");
    propertyGroup->GetRangesR().push_back(propertyRangeGroupSpecification);
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ("Widget", children[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByDoublesRange)
    {
    // insert an instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("DoubleProperty", ECValue(4.5));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_GroupsByProperty_WithRanges_FilteringByIntegersRange", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "DoubleProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "1", "5");
    propertyGroup->GetRangesR().push_back(propertyRangeGroupSpecification);
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ("Widget", children[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByLongsRange)
    {
    // insert an instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("LongProperty", ECValue((int64_t)4));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_GroupsByProperty_WithRanges_FilteringByLongsRange", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "LongProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "1", "5");
    propertyGroup->GetRangesR().push_back(propertyRangeGroupSpecification);
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ("Widget", children[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByDateTimeRange)
    {
    // insert an instance
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("DateProperty", ECValue(DateTime(2017, 5, 30)));});
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_GroupsByProperty_WithRanges_FilteringByDateTimeRange", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "DateProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "2017-05-01", "2017-06-01");
    propertyGroup->GetRangesR().push_back(propertyRangeGroupSpecification);
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_STREQ("Range", nodes[0]->GetLabel().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
    ASSERT_EQ(1, children.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, children[0]->GetType().c_str());
    EXPECT_STREQ("Widget", children[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_WithHideIfNoChildrenParent)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(4));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetClass, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_GroupsByProperty_WithRanges_Nested", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false,
        "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->GetSpecificationsR().push_back(new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childRule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "IntProperty", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("", "", "1", "5");
    propertyGroup->GetRangesR().push_back(propertyRangeGroupSpecification);
    groupingRule->GetGroupsR().push_back(propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // expect 1 gadget node
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget", rootNodes[0]->GetLabel().c_str());

    // request children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("1 - 5", childNodes[0]->GetLabel().c_str());
    }

#ifdef wip
/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016   RenameNodeRule       
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RenameNodeRule)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("RenameNodeRule", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");
    
    rule->GetSpecificationsR().push_back(customNodeSpecification);
    rules->AddPresentationRule(*rule);
    
    RenameNodeRuleP renameNodeRule = new RenameNodeRule("ThisNode.Type=\"customType\"", 1);
    rules->AddPresentationRule(*renameNodeRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("RenameNodeRule", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedLabel", nodes[0]->GetCollapsedImageId().c_str());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas               02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByBaseClassAndByInstancesClasses)
    {
    // insert some widget instances
    ECClassCP classF = m_schema->GetClassCP("ClassF");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classF);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Grouping_ClassGroup_GroupsByBaseClassAndByInstancesClasses", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, true, false, "RulesEngineTest");
    rule->GetSpecificationsR().push_back(allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", true, "RulesEngineTest", "ClassE");
    groupingRule->GetGroupsR().push_back(classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Grouping_ClassGroup_GroupsByBaseClassAndByInstancesClasses", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> nodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", nodes[0]->GetLabel().c_str());

    // make sure we have 2 classE child nodes
    DataContainer<NavNodeCPtr> classEChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *nodes[0], PageOptions(), options);
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
    IECInstancePtr classDInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *classD);
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    IECInstancePtr classEInstance = RulesEngineTestHelpers::InsertInstance(*s_project, *classE);
    RulesEngineTestHelpers::InsertRelationship(*s_project, relationship, *classDInstance, *classEInstance);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("DoesNotReturnECInstanceNodesOfTheSameSpecificationAlreadyExistingInHierarchy", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassD", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassD,ClassE");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*relatedNodeRule);
    
    // make sure we have 1 ClassD root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("DoesNotReturnECInstanceNodesOfTheSameSpecificationAlreadyExistingInHierarchy", TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassD", rootNodes[0]->GetLabel().c_str());

    // ClassD instance node should have 1 ClassE instance child node
    DataContainer<NavNodeCPtr> classDChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, classDChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classDChildNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", classDChildNodes[0]->GetLabel().c_str());
    
    // ClassE instance node has 1 ClassD instance node. There's such a node already in the hierarchy, but it's based on
    // different (root node rule) specification, so we allow it
    DataContainer<NavNodeCPtr> classEChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classDChildNodes[0], PageOptions(), options);
    ASSERT_EQ(1, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, classEChildNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassD", classEChildNodes[0]->GetLabel().c_str());

    // This time ClassD instance node has no children because there's such a node up in the
    // hierarchy and it's also based on the same specification.
    DataContainer<NavNodeCPtr> classDChildNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *classEChildNodes[0], PageOptions(), options);
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, widgetHasGadgetsRelationship, *widget, *gadget);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupingWorksCorrectlyWithRelatedInstancesSpecificationWhenParentRelatedInstanceNodeLevelIsHidden", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget");
    relatedNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*relatedNodeRule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Gadget", "", "", "");
    PropertyGroup* propertyGroupSpec = new PropertyGroup("", "", true, "MyID");
    groupingRule->GetGroupsR().push_back(propertyGroupSpec);
    rules->AddPresentationRule(*groupingRule);
    
    // make sure we have 1 property grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());

    // the node should have 1 Gadget child node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ(gadget->GetInstanceId().c_str(), childNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomizesNodesWhenCustomizationRulesDefinedInSupplementalRuleset)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    
    // create the rule sets
    PresentationRuleSetPtr primaryRules = PresentationRuleSet::CreateInstance("CustomizesNodesWhenCustomizationRulesDefinedInSupplementalRuleset", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*primaryRules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    primaryRules->AddPresentationRule(*rule);

    PresentationRuleSetPtr supplementalRules = PresentationRuleSet::CreateInstance(primaryRules->GetRuleSetId(), 1, 0, true, "Customization", "", "", false);
    m_locater->AddRuleSet(*supplementalRules);
    
    LabelOverride* customizationRule = new LabelOverride("", 1, "\"Test\"", "");
    supplementalRules->AddPresentationRule(*customizationRule);

    // make sure we have 1 node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(primaryRules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), rootNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("test"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(*s_project, *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("test"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, widgetHasGadgetsRelationship, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(*s_project, gadgetHasSprocketsRelationship, *gadget, *sprocket);
    
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupingChildrenByRelatedInstanceProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP relatedInstanceNodesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "sprocket.MyID = parent.Description", "RulesEngineTest:Gadget", false);
    relatedInstanceNodesSpecification->GetRelatedInstances().push_back(new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket", "sprocket"));
    childNodeRule->GetSpecificationsR().push_back(relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*childNodeRule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Sprocket", "", "", "");
    PropertyGroup* propertyGroupSpec = new PropertyGroup("", "", true, "Description");
    groupingRule->GetGroupsR().push_back(propertyGroupSpec);
    rules->AddPresentationRule(*groupingRule);
    
    // make sure we have 1 widget
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // the node should have 1 property grouping node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("Description", childNodes[0]->GetKey().AsECPropertyGroupingNodeKey()->GetPropertyName().c_str());

    // the child node should have 1 Gadget child node
    DataContainer<NavNodeCPtr> grandChildNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options);
    ASSERT_EQ(1, grandChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, grandChildNodes[0]->GetType().c_str());
    EXPECT_STREQ(gadget->GetInstanceId().c_str(), grandChildNodes[0]->GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, ReturnsChildNodesWhenTheresOnlyOneLabelGroupingNode)
    {
    RulesEngineTestHelpers::DeleteInstances(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupingChildrenByRelatedInstanceProperty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:Widget", false);
    rule->GetSpecificationsR().push_back(instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // make sure we have 2 widget nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("Widget", rootNodes[1]->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstanceNode, rootNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, ReturnsChildrenUsingAllSpecifications)
    {
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ReturnsChildrenUsingAllSpecifications", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "a", "a", "a", "a");
    spec->SetAlwaysReturnsChildren(false);
    rootRule->GetSpecificationsR().push_back(spec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"a\"", 1, false, TargetTree_Both);
    childRule->GetSpecificationsR().push_back(new CustomNodeSpecification(1, false, "b", "b", "b", "b"));
    childRule->GetSpecificationsR().push_back(new CustomNodeSpecification(1, false, "c", "c", "c", "c"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("a", rootNodes[0]->GetLabel().c_str());

    // make sure it has 2 child nodes
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("AutoExpandSetsIsExpandedFlag", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    // set auto expand property to true (default false)
    RootNodeRule* rootRule = new RootNodeRule("1=1", 10, false, TargetTree_Both, true);
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "test", "test", "test", "test");
    rootRule->GetSpecificationsR().push_back(spec);
    rules->AddPresentationRule(*rootRule);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->IsExpanded());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByParentNodes_MatchingFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test"));});
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(*s_project, *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("FiltersNodesByParentNodes_MatchingFilter", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsOfClass(\"Widget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.Description", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*childRule);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.IsOfClass(\"Gadget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule2->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.parent.Description", "RulesEngineTest:Sprocket", false));
    rules->AddPresentationRule(*childRule2);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("Gadget", childNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node as well
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options);
    ASSERT_EQ(1, childNodes2.GetSize());
    EXPECT_STREQ("Sprocket", childNodes2[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByParentNodes_NonMatchingParentFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test1"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("FiltersNodesByParentNodes_NonMatchingFilter", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsOfClass(\"Widget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.Description", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(0, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByParentNodes_NonMatchingGrandparentFilter)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test1"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test1"));});
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(*s_project, *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Test2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("FiltersNodesByParentNodes_NonMatchingGrandparentFilter", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsOfClass(\"Widget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.Description", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*childRule);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.IsOfClass(\"Gadget\", \"RulesEngineTest\")", 1, false, TargetTree_Both);
    childRule2->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description = parent.parent.Description", "RulesEngineTest:Sprocket", false));
    rules->AddPresentationRule(*childRule2);

    // make sure we have 1 root node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Widget", rootNodes[0]->GetLabel().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("Gadget", childNodes[0]->GetLabel().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childNodes2 = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *childNodes[0], PageOptions(), options);
    ASSERT_EQ(0, childNodes2.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_NoDigitsAfterDecimalPointAppendTwoZeroes)
    {
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.)); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByDoubleProperty_NoDigitsAfterDecimalPointAppendTwoZeroes", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->GetGroupsR().push_back(groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2.00", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("Widget", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("Widget", childrenNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_OneDigitAfterDecimalPointAppendOneZero)
    {
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.5)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.5)); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByDoubleProperty_OneDigitAfterDecimalPointAppendOneZero", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->GetGroupsR().push_back(groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2.50", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("Widget", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("Widget", childrenNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointSameResult)
    {
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(0.00546)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(0.00798)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(0.00899)); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointSameResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->GetGroupsR().push_back(groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("0.01", rootNodes[0]->GetLabel().c_str());

    //make sure it has 3 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(3, childrenNodes.GetSize());
    EXPECT_STREQ("Widget", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("Widget", childrenNodes[1]->GetLabel().c_str());
    EXPECT_STREQ("Widget", childrenNodes[2]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointDifferentResult)
    {
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.505f)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.504f)); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointDifferentResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->GetGroupsR().push_back(groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("2.50", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("2.51", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("Widget", firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options);
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("Widget", secondNodeChildrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointSameResult)
    {
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.59999999)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.59999999)); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointSameResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->GetGroupsR().push_back(groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("2.60", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("Widget", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("Widget", childrenNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointDifferentResult)
    {
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.595)); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("DoubleProperty", ECValue(2.594)); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointDifferentResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "DoubleProperty", "");
    groupingRule->GetGroupsR().push_back(groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("2.59", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("2.60", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("Widget", firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options);
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("Widget", secondNodeChildrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPoints)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByPointProperty_EqualPoints", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->GetGroupsR().push_back(groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("ClassH", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("ClassH", childrenNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_AlmostEqualPointsSameResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.121, 1.121, 1.121))); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.122, 1.122, 1.122))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByPointProperty_AlmostEqualPointsSameResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->GetGroupsR().push_back(groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("ClassH", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("ClassH", childrenNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_DifferentPointsDifferentResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.11, 1.11, 1.11))); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByPointProperty_DifferentPointsDifferentResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->GetGroupsR().push_back(groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.11 Y: 1.11 Z: 1.11", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("ClassH", firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options);
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("ClassH", secondNodeChildrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPointsDifferentXCoordinatesDifferentResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.11, 1.12, 1.12))); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByPointProperty_EqualPointsDifferentXCoordinatesDifferentResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->GetGroupsR().push_back(groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.11 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("ClassH", firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options);
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("ClassH", secondNodeChildrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPointsDifferentYCoordinatesDifferentResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.11, 1.12))); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByPointProperty_EqualPointsDifferentYCoordinatesDifferentResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->GetGroupsR().push_back(groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.11 Z: 1.12", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("ClassH", firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options);
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("ClassH", secondNodeChildrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPointsDifferentZCoordinatesDifferentResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.11))); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByPointProperty_EqualPointsDifferentZCoordinatesDifferentResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->GetGroupsR().push_back(groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.11", rootNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabel().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    EXPECT_STREQ("ClassH", firstNodeChildrenNodes[0]->GetLabel().c_str());

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[1], PageOptions(), options);
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    EXPECT_STREQ("ClassH", secondNodeChildrenNodes[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_YZCoordinatesEqualXCoordinatesAlmostEqualSameResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.119, 1.12, 1.12))); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.124, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByPointProperty_YZCoordinatesEqualXCoordinatesAlmostEqualSameResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->GetGroupsR().push_back(groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("ClassH", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("ClassH", childrenNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_XZCoordinatesEqualYCoordinatesAlmostEqualSameResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.119, 1.12))); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.124, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByPointProperty_XZCoordinatesEqualYCoordinatesAlmostEqualSameResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->GetGroupsR().push_back(groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("ClassH", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("ClassH", childrenNodes[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_XYCoordinatesEqualZCoordinatesAlmostEqualSameResult)
    {
    ECClassCP classH = m_schema->GetClassCP("ClassH");
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.119))); });
    RulesEngineTestHelpers::InsertInstance(*s_project, *classH, [](IECInstanceR instance) {instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.12, 1.12, 1.124))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("GroupsNodesByPointProperty_XYCoordinatesEqualZCoordinatesAlmostEqualSameResult", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", "RulesEngineTest", "ClassH", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "PointProperty", "");
    groupingRule->GetGroupsR().push_back(groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->GetSpecificationsR().push_back(new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", "RulesEngineTest:ClassH", true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rules->GetRuleSetId().c_str(), TargetTree_MainTree).GetJson();
    DataContainer<NavNodeCPtr> rootNodes = IECPresentationManager::GetManager().GetRootNodes(s_project->GetECDb(), PageOptions(), options);
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabel().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = IECPresentationManager::GetManager().GetChildren(s_project->GetECDb(), *rootNodes[0], PageOptions(), options);
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("ClassH", childrenNodes[0]->GetLabel().c_str());
    EXPECT_STREQ("ClassH", childrenNodes[1]->GetLabel().c_str());
    }
