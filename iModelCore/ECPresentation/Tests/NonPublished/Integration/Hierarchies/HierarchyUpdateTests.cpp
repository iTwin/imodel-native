/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../PresentationManagerIntegrationTests.h"
#include "../../../../Source/PresentationManagerImpl.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestUiStateProvider : IUiStateProvider
{
private:
    std::function<bvector<IConnectionCP>()> m_connectionsGetter;
    std::function<bvector<PresentationRuleSetCPtr>()> m_rulesetsGetter;
    std::shared_ptr<UiState> m_state;
protected:
    std::shared_ptr<UiState> _GetUiState(IConnectionCR, Utf8StringCR) const override {return m_state;}
    bvector<RulesetUiState> _GetUiState(IConnectionCR) const override
        {
        return ContainerHelpers::TransformContainer<bvector<RulesetUiState>>(m_rulesetsGetter(), [&](auto const& ruleset)
            {
            return RulesetUiState(ruleset->GetRuleSetId(), m_state);
            });
        }
    bvector<ConnectionUiState> _GetUiState(Utf8StringCR) const override
        {
        return ContainerHelpers::TransformContainer<bvector<ConnectionUiState>>(m_connectionsGetter(), [&](auto const& connection)
            {
            return ConnectionUiState(*connection, m_state);
            });
        }
public:
    TestUiStateProvider(std::function<bvector<IConnectionCP>()> connectionsGetter, std::function<bvector<PresentationRuleSetCPtr>()> rulesetsGetter)
        : m_connectionsGetter(connectionsGetter), m_rulesetsGetter(rulesetsGetter), m_state(std::make_shared<UiState>())
        {}
    void AddExpandedNode(NavNodeKeyCPtr key) {m_state->GetHierarchyLevelState(key.get()).SetIsExpanded(true);}
    void RemoveExpandedNode(NavNodeKeyCPtr key) {m_state->GetHierarchyLevelState(key.get()).SetIsExpanded(false);}
    void SetInstanceFilter(NavNodeCP parentNode, Utf8String instanceFilter) {m_state->GetHierarchyLevelState(parentNode).SetInstanceFilters({ instanceFilter });}
    void SetInstanceFilters(NavNodeCP parentNode, bvector<Utf8String> instanceFilters) {m_state->GetHierarchyLevelState(parentNode).SetInstanceFilters(instanceFilters);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyUpdateTests : UpdateTests
    {
    std::shared_ptr<TestUiStateProvider> m_uiState;

    void SetUp() override
        {
        m_uiState = std::make_shared<TestUiStateProvider>(
            [this]() -> bvector<IConnectionCP>
                {
                return { m_manager->GetConnections().GetConnection(m_db) };
                },
            [this]() -> bvector<PresentationRuleSetCPtr>
                {
                return ContainerHelpers::TransformContainer<bvector<PresentationRuleSetCPtr>>(m_locater->LocateRuleSets());
                });
        UpdateTests::SetUp();
        }

    void _ConfigureManagerParams(ECPresentationManager::Params& params) override
        {
        UpdateTests::_ConfigureManagerParams(params);
        params.SetUiStateProvider(m_uiState);
        }

    void SetNodeExpanded(NavNodeCR node, bool isExpanded = true)
        {
        return isExpanded ? m_uiState->AddExpandedNode(node.GetKey()) : m_uiState->RemoveExpandedNode(node.GetKey());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesECInstanceNodeAfterECInstanceDelete)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    ASSERT_TRUE(nodes[0].IsValid());
    ASSERT_TRUE(nodes[1].IsValid());
    NavNodeCPtr removedNode = nodes[0];
    NavNodeCPtr retainedNode = nodes[1];

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget1);

    // expect 1 node
    nodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_EQ(*retainedNode->GetKey(), *nodes[0]->GetKey());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesECClassGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 class grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request its children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });

    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    NavNodeCPtr removedNode = childNodes[0];

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget1);

    // expect the same one class grouping node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    EXPECT_EQ(1, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expect it to have only one instance node
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    VerifyNodeInstance(*childNodes[0], *widget2);

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    NavNodeCPtr removedClassNode;
    bvector<NavNodeCPtr> removedChildNodes;

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 class grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
    removedClassNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request its children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });

    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    removedChildNodes.push_back(childNodes[0]);
    removedChildNodes.push_back(childNodes[1]);

    // delete both instances
    ECInstanceDeleter deleter(m_db, *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);
    m_db.SaveChanges();

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    m_eventsSource->NotifyECInstancesDeleted(m_db, deletedInstances);

    // expect the class grouping node to be gone as there're no instances left to group
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect one update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesECClassGroupingNodeAfterECInstanceDeleteWhenNoECInstancesLeft_WhenChildrenNotCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, true, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 class grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
    NavNodeCPtr removedNode = rootNodes[0];

    // delete both instances
    ECInstanceDeleter deleter(m_db, *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);
    m_db.SaveChanges();

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    m_eventsSource->NotifyECInstancesDeleted(m_db, deletedInstances);

    // expect the class grouping node to be gone as there're no instances left to group
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect one update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeChildrenAfterECInstanceDeleteWhenInstancesLeft)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
    NavNodeCPtr displayLabelGroupingNode = rootNodes[0];
    NavNodeCPtr retainedNode = rootNodes[1];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request label grouping nodes children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    EXPECT_EQ(2, childNodes.GetSize());

    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    bvector<NavNodeCPtr> instanceNodes;
    instanceNodes.push_back(childNodes[0]);
    instanceNodes.push_back(childNodes[1]);

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget1);

    // expect the display label grouping node to be gone (single instance is not grouped under a display label grouping node)
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(rootNodes[0]->GetLabelDefinition().GetDisplayValue().Equals("My Label"));
    ASSERT_EQ(*retainedNode->GetKey(), *rootNodes[1]->GetKey());

    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    EXPECT_EQ(0, childNodes.GetSize());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    NavNodeCPtr removedLabelGroupingNode;
    bvector<NavNodeCPtr> removedInstanceNodes;

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_TRUE(rootNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    removedLabelGroupingNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request label grouping nodes children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });

    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    removedInstanceNodes.push_back(childNodes[0]);
    removedInstanceNodes.push_back(childNodes[1]);

    // delete both instances
    ECInstanceDeleter deleter(m_db, *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);
    m_db.SaveChanges();

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    m_eventsSource->NotifyECInstancesDeleted(m_db, deletedInstances);

    // expect the display label grouping node to be gone as there're no instances left to group
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(rootNodes[0]->GetLabelDefinition().GetDisplayValue().Equals("Other Label"));

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeAfterECInstanceDeleteWhenNoInstancesLeft_WhenChildrenNotCached)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
    NavNodeCPtr removedNode = rootNodes[0];

    // delete both instances
    ECInstanceDeleter deleter(m_db, *m_widgetClass, nullptr);
    deleter.Delete(*widget1);
    deleter.Delete(*widget2);
    m_db.SaveChanges();

    bvector<IECInstanceCP> deletedInstances;
    deletedInstances.push_back(widget1.get());
    deletedInstances.push_back(widget2.get());
    m_eventsSource->NotifyECInstancesDeleted(m_db, deletedInstances);

    // expect the display label grouping node to be gone as there're no instances left to group
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(rootNodes[0]->GetLabelDefinition().GetDisplayValue().Equals("Other Label"));

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesDisplayLabelGroupingNodeAfterECInstanceLabelChange)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("My Label"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Other Label"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 display label grouping node & 1 ECInstance node
    ASSERT_EQ(2, rootNodes.GetSize());

    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
    NavNodeCPtr displayLabelGroupingNode = rootNodes[0];
    SetNodeExpanded(*rootNodes[0]);

    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_TRUE(rootNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));

    // request label grouping nodes children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });

    // expect 2 instance nodes
    ASSERT_EQ(2, childNodes.GetSize());
    ASSERT_TRUE(childNodes[0].IsValid());
    ASSERT_TRUE(childNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(childNodes[1].IsValid());
    ASSERT_TRUE(childNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));

    // change the label of one of the instances
    widget1->SetValue("MyID", ECValue("My Other Label"));
    ECInstanceUpdater updater(m_db, *widget1, nullptr);
    updater.Update(*widget1);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget1);

    // expect the display label grouping node to be gone (single instance is not grouped under a display label grouping node)
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(rootNodes[0]->GetLabelDefinition().GetDisplayValue().Equals("My Label"));
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_TRUE(rootNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(rootNodes[1]->GetLabelDefinition().GetDisplayValue().Equals("My Other Label"));

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, SetsCorrectInsertPositionWhenMultipleSpecificationsUsedAtTheSameLevel1)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect no nodes
    ASSERT_EQ(0, rootNodes.GetSize());

    // create a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect one widget instance node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(rootNodes[0]->GetLabelDefinition().GetDisplayValue().Equals("WidgetID"));

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, SetsCorrectInsertPositionWhenMultipleSpecificationsUsedAtTheSameLevel2)
    {
    // insert some gadgets
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 2 gadget nodes
    ASSERT_EQ(2, rootNodes.GetSize());

    // create a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect one additional widget instance node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[2].IsValid());
    ASSERT_TRUE(rootNodes[2]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(rootNodes[2]->GetLabelDefinition().GetDisplayValue().Equals("WidgetID"));

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CreatesDisplayLabelGroupingNodeAfterECInstanceLabelChange)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 3"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 3 ECInstance nodes
    ASSERT_EQ(3, rootNodes.GetSize());
    for (NavNodeCPtr node : rootNodes)
        {
        ASSERT_TRUE(node.IsValid());
        ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        }

    bvector<NavNodeCPtr> deletedNodes;
    deletedNodes.push_back(rootNodes[0]);
    deletedNodes.push_back(rootNodes[1]);

    // change the label of one of the instances
    widget1->SetValue("MyID", ECValue("Label 2"));
    ECInstanceUpdater updater(m_db, *widget1, nullptr);
    updater.Update(*widget1);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget1);

    // expect the display label grouping node to be created
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    ASSERT_TRUE(rootNodes[0]->GetLabelDefinition().GetDisplayValue().Equals("Label 2"));
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_TRUE(rootNodes[1]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(rootNodes[1]->GetLabelDefinition().GetDisplayValue().Equals("Label 3"));

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CreatesDisplayLabelGroupingNodeAfterECInstanceInsert)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());
    for (NavNodeCPtr node : rootNodes)
        {
        ASSERT_TRUE(node.IsValid());
        ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        }

    NavNodeCPtr deletedNode = rootNodes[1];

    // change the label of one of the instances
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget3);

    // expect the display label grouping node to be created
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_TRUE(rootNodes[0]->GetType().Equals(NAVNODE_TYPE_ECInstancesNode));
    ASSERT_TRUE(rootNodes[0]->GetLabelDefinition().GetDisplayValue().Equals("Label 1"));
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_TRUE(rootNodes[1]->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    ASSERT_TRUE(rootNodes[1]->GetLabelDefinition().GetDisplayValue().Equals("Label 2"));
    EXPECT_EQ(2, rootNodes[1]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDisplayLabelGroupingNodeAfterECInstanceInsert)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 2"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());

    ASSERT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request display label grouping node children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    for (NavNodeCPtr const& node : childNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabelDefinition().GetDisplayValue().c_str());
        }

    // change the label of one of the instances
    IECInstancePtr widget4 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget4);

    // expect the display label grouping node to have 3 children now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_EQ(3, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(3, childNodes.GetSize());
    for (NavNodeCPtr const& node : childNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabelDefinition().GetDisplayValue().c_str());
        }

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceInsert)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabelDefinition().GetDisplayValue().c_str());
        }

    // insert another widget
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget3);

    // expect 3 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabelDefinition().GetDisplayValue().c_str());
        }

    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesVirtualDisplayLabelGroupingNodeAfterECInstanceDelete)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabelDefinition().GetDisplayValue().c_str());
        }
    NavNodeCPtr deletedNode = rootNodes[1];

    // delete one of the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget2, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget2);

    // expect 2 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    for (NavNodeCPtr const& node : rootNodes)
        {
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        EXPECT_STREQ("Label 1", node->GetLabelDefinition().GetDisplayValue().c_str());
        }

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesECInstanceNodeAfterECInstanceChange)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Label 1"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());
    for (NavNodeCPtr node : rootNodes)
        {
        ASSERT_TRUE(node.IsValid());
        ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        }

    // change the label of one of the instances
    widget1->SetValue("MyID", ECValue("Label 2"));
    ECInstanceUpdater updater(m_db, *widget1, nullptr);
    updater.Update(*widget1);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget1);

    // expect the label of the node to be changed
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ("Label 2", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_AllInstanceNodesSpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("2"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // expect 2 nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("1", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str());
    ASSERT_STREQ("2", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("2"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // expect 2 nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("1", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(rootNodes[1].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str());
    ASSERT_STREQ("2", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification_NonPolymorphicMatch)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert some instances
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(m_db, *classE, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert F instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(m_db, *classF, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *instanceF);

    // still expect 1 node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());

    // expect 0 update records
    ASSERT_TRUE(m_updateRecordsHandler->GetRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesRootDataSourceAfterECInstanceInsert_InstancesOfSpecificClassesSpecification_PolymorphicMatch)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert some instances
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(m_db, *classE, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert F instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(m_db, *classF, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *instanceF);

    // expect 2 nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, DoesntUpdateRootDataSourceAfterECInstanceInsertIfClassDoesntMatch_InstancesOfSpecificClassesSpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert a gadget instance
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget);

    // still expect 1 node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());

    // expect 0 update records
    ASSERT_TRUE(m_updateRecordsHandler->GetRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_RelatedInstancesSpecification)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationshipClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    // insert the root instance
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.IsInstanceNode", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection::RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // insert a gadget instance
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsRelationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget);

    // expect 1 node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithSingleQuerySpecification)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(12));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(22));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT [Widget].[MyID] FROM [RulesEngineTest].[Widget] WHERE [Widget].[IntProperty] > 10", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*spec);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(32));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget3);
    ECValue v;

    // expect 3 nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecificationWithMultipleQuerySpecifications)
    {
    // insert some widget instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(12));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(22));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName=\"Widget\"", 1, "\"Widget-\" & ThisNode.InstanceId", ""));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT [Widget].[MyID] FROM [RulesEngineTest].[Widget] WHERE [Widget].[IntProperty] > 20", "RulesEngineTest", "Widget"));
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT [Widget].[MyID] FROM [RulesEngineTest].[Widget] WHERE [Widget].[IntProperty] < 15", "RulesEngineTest", "Widget"));
    rule->AddSpecification(*spec);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());

    // insert another instance
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(32));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget3);
    ECValue v;

    // expect 3 nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(3, rootNodes.GetSize());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterECInstanceInsert_SearchResultInstanceNodesSpecification_Polymorphic)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert some ClassE instances
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(m_db, *classE, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(12));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new StringQuerySpecification("SELECT * FROM [RulesEngineTest].[ClassE] WHERE [ClassE].[IntProperty] > 10", "RulesEngineTest", "ClassE"));
    rule->AddSpecification(*spec);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());

    // insert ClassF instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(m_db, *classF, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(32));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *instanceF);
    ECValue v;

    // expect 2 nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterParentECInstanceUpdate_SearchResultInstanceNodesSpecification)
    {
    ECRelationshipClassCP relationship = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    static Utf8CP s_query1 = "SELECT * FROM [RulesEngineTest].[Gadget]";
    static Utf8CP s_query2 = "SELECT * FROM [RulesEngineTest].[Gadget] LIMIT 1";

    // insert some widgets and gadgets
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("WidgetID"));
        instance.SetValue("Description", ECValue(s_query1));
        });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationship, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationship, *widget, *gadget2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    rules->AddPresentationRule(*childRule);

    SearchResultInstanceNodesSpecificationP spec = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    spec->AddQuerySpecification(*new ECPropertyValueQuerySpecification("RulesEngineTest", "Gadget", "Description"));
    childRule->AddSpecification(*spec);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 ECInstance node
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());

    // change the query in widget Description property
    widget->SetValue("Description", ECValue(s_query2));
    ECInstanceUpdater updater(m_db, *m_widgetClass, nullptr);
    updater.Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);
    ECValue v;

    // expect 1 child node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CreatesNodeAfterECInstanceInsertWhenPreviouslyNotCreatedDueToHideIfNoChildrenFlag)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rules->AddPresentationRule(*rule);

    InstanceNodesOfSpecificClassesSpecificationP rootSpec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false,
        "", "RulesEngineTest:Widget", false);
    rule->AddSpecification(*rootSpec);

    ChildNodeRuleP childRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    rootSpec->AddNestedRule(*childRule);

    InstanceNodesOfSpecificClassesSpecificationP childSpec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Gadget", false);
    childRule->AddSpecification(*childSpec);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 0 nodes
    ASSERT_EQ(0, rootNodes.GetSize());

    // insert a gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget);

    // expect 1 node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, SameLabelInstanceGroupIsCreatedWhenAdditionalInstancesAreInserted)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 widget
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], {widget});

    // insert another widget
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // still expect 1 node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], {widget, widget2});

    // expect 2 update records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, BaseClassGroupIsUpdatedWhenAdditionalInstancesAreInserted)
    {
    ECClassCP classE = m_schema->GetClassCP("ClassE");
    ECClassCP classF = m_schema->GetClassCP("ClassF");

    // insert a base class instance
    RulesEngineTestHelpers::InsertInstance(m_db, *classE, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, "RulesEngineTest", "ClassE"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 instance
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(1, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 1 child
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    // insert a derived instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(m_db, *classF, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *instanceF);

    // still expect 1 ECClassGrouping node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("ClassE", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expect it to have 2 children now
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, ValuePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted)
    {
    // insert a widget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 property grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(1, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 1 child
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    // insert another widget
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // still expect 1 ECProperty grouping node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expect it to have 2 children now
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RangePropertyGroupIsUpdatedWhenAdditionalInstancesAreInserted)
    {
    // insert a widget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    rules->AddPresentationRule(*groupingRule);

    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->AddRange(*new PropertyRangeGroupSpecification("5 to 10", "", "5", "10"));
    groupingRule->AddGroup(*groupingSpec);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 property grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("5 to 10", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(1, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 1 child
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    // insert another widget
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget2);

    // still expect 1 ECProperty grouping node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("5 to 10", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expect it to have 2 children now
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, PropertyGroupIsCreatedWhenInstanceValuesChange)
    {
    // insert some widgets
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 property grouping node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    // expect it to have 2 children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    NavNodeKeyCPtr deletedNode = childNodes[1]->GetKey();

    // change one of the widgets
    widget2->SetValue("IntProperty", ECValue(9));
    ECInstanceUpdater updater(m_db, *widget2, nullptr);
    updater.Update(*widget2);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget2);

    // expect 2 ECProperty grouping nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(1, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[1]->GetType().c_str());
    ASSERT_STREQ("9", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(1, rootNodes[1]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expect each of them to have 1 child
    DataContainer<NavNodeCPtr> childNodes1 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes1.GetSize());
    DataContainer<NavNodeCPtr> childNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get())).get(); });
    ASSERT_EQ(1, childNodes2.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, PropertyGroupIsCreatedWhenInstanceValuesChangeWithCreateGroupForSingleItemFalse)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(9));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "IntProperty"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 2 ECInstance nodes
    ASSERT_EQ(2, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str());

    // change one of the widgets
    widget2->SetValue("IntProperty", ECValue(8));
    ECInstanceUpdater updater(m_db, *widget2, nullptr);
    updater.Update(*widget2);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget2);

    // expect 1 ECProperty grouping node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    ASSERT_STREQ("8", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expect it to have 2 children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());

    // expect 1 update records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesCustomNodeWithHideIfNoChildrenFlagWhenNothingChanges)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "MyType", "MyLabel", "", "MyImageId");
    spec->SetHasChildren(ChildrenHint::Unknown);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // insert a widget to force custom spec update
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect 1 root node now (still no custom node)
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesCustomNodeWithHideIfNoChildrenFlagWhenChildrenAdded)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "MyType", "MyLabel", "", "MyImageId");
    spec->SetHasChildren(ChildrenHint::Unknown);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"MyType\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // insert a widget to force custom spec update
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect 2 root nodes now (including the custom node)
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("MyType", rootNodes[1]->GetType().c_str());
    EXPECT_STREQ("MyLabel", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesCustomNodeWithHideIfNoChildrenFlagWhenChildrenRemoved)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "MyType", "MyLabel", "", "MyImageId");
    spec->SetHasChildren(ChildrenHint::Unknown);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"MyType\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, rootNodes.GetSize());
    bvector<NavNodeCPtr> deletedNodes = {rootNodes[0], rootNodes[1]};

    // insert a widget to force custom spec update
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // expect 0 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromCustomNode)
    {
    // insert 2 widgets and a gadget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new CustomNodeSpecification(1, false, "MyCustomType", "MyLabel", "", "MyImageId"));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"MyCustomType\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("MyCustomType", rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("MyLabel", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(2, childNodes[1]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expand node
    SetNodeExpanded(*childNodes[1]);

    DataContainer<NavNodeCPtr> widgetChildren = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), childNodes[1].get())).get(); });
    ASSERT_EQ(2, widgetChildren.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetChildren[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetChildren[1]->GetType().c_str());

    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);

    // expect 2 children now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, HidesDisplayLabelGroupingNodeWhenSiblingIsRemovedFromGroupingNode)
    {
    // insert 2 widgets and a gadget
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr differentWidget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("ZZZ"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "Description,MyID"));
    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, true, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty", "Default Label"));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_EQ(3, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_EQ(2, childNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("ZZZ", childNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    bvector<NavNodeCPtr> deletedNodes = {childNodes[0], childNodes[1]};

    // expand node
    SetNodeExpanded(*childNodes[0]);

    DataContainer<NavNodeCPtr> widgetChildren = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); });
    ASSERT_EQ(2, widgetChildren.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetChildren[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, widgetChildren[1]->GetType().c_str());

    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *differentWidget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *differentWidget);

    // expect 2 children now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());
    EXPECT_EQ(2, rootNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[1]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 child node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 child node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyForwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 children now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterOneToManyBackwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 children now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 child node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 child node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyForwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childrenRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 children now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RelatedInstanceNodesAreUpdatedAfterManyToManyBackwardRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget, relate them
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childrenRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "RulesEngineTest", "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childrenRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, childNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr deletedNode = childNodes[0];

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 children now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenBackwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\")", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenForwardHasRelatedInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.HasRelatedInstance(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\")", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenBackwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\") > 0", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipInsertWhenForwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\") > 0", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenBackwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetsHaveGadgets\", \"Backward\", \"RulesEngineTest:Widget\") > 0", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipDeleteWhenForwardGetRelatedInstancesCountInstanceECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\") > 0", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());
    m_eventsSource->NotifyECInstancesChanged(m_db, instances, ChangeType::Update); // both instances are notified in many-to-many relationship case

    // expect 0 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenBackwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\") > 0", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipInsertWhenForwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\") > 0", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenBackwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetHasGadgets\", \"Backward\", \"RulesEngineTest:Widget\") > 0", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // unrelate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterOneToManyRelationshipDeleteWhenForwardGetRelatedInstancesCountECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    ECInstanceKey relationshipKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.GetRelatedInstancesCount(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\") > 0", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // relate the instances
    RulesEngineTestHelpers::DeleteInstance(m_db, relationshipKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget); // gadget is notified because it has a WidgetId property

    // expect 0 root nodes now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterManyToManyRelationshipRelatedInstanceUpdateWhenGetRelatedValueECExpressionIsUsedInInstanceFilter)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    bvector<IECInstanceCP> instances;
    instances.push_back(widget.get());
    instances.push_back(gadget.get());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "this.GetRelatedValue(\"RulesEngineTest:WidgetsHaveGadgets\", \"Forward\", \"RulesEngineTest:Gadget\", \"MyID\") = \"123\"", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    gadget->SetValue("MyID", ECValue("123"));
    ECInstanceUpdater updater(m_db, *gadget, nullptr);
    updater.Update(*gadget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget);

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterSkippedOneToManyRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClassWG = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClassGS = m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(m_db, *m_sprocketClass);

    // relate gadget to sprocket
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClassGS, *gadget, *sprocket, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "",
        RequiredRelationDirection_Forward, "", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // relate widget to gadget
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClassWG, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget);

    // expect 1 root node with 1 child
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *widget);
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

#ifdef wip_skipped_instance_keys_performance_issue
/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterSkippedOneToManyRelationshipDelete)
    {
    ECRelationshipClassCP relationshipClassWG = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClassGS = m_schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(m_db, *m_sprocketClass);

    // relate the instances
    ECInstanceKey relWGKey = RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClassWG, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClassGS, *gadget, *sprocket, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "",
        RequiredRelationDirection_Forward, "", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr deletedNode = childNodes[0];

    RulesEngineTestHelpers::DeleteInstance(m_db, relWGKey, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget);

    // expect 1 root node with no children
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *widget);
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[0].GetChangeType());
    EXPECT_EQ(*deletedNode->GetKey(), *m_updateRecordsHandler->GetRecords()[0].GetNode()->GetKey());

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[1].GetChangeType());
    EXPECT_EQ(*rootNodes[0]->GetKey(), *m_updateRecordsHandler->GetRecords()[1].GetNode()->GetKey());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterSkippedManyToManyRelationshipInsert)
    {
    ECRelationshipClassCP relationshipClass1 = m_schema->GetClassCP("WidgetsHaveGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClass2 = m_schema->GetClassCP("WidgetsHaveGadgets2")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass1, *widget1, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.IntProperty = 1", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "",
        RequiredRelationDirection_Both, "", "RulesEngineTest:WidgetsHaveGadgets2", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *widget1);

    // expand node
    SetNodeExpanded(*rootNodes[0]);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // insert the relationship
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass2, *widget2, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstancesUpdated(m_db, {widget2.get(), gadget.get()});

    // expect the child node to exist now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *widget1);
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *widget2);

    // verify records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterRelatedInstanceInsert)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "", "RulesEngineTest:Gadget", false));
    rule->GetSpecifications()[0]->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"Gadget_\" & IIF(NOT IsNull(widget.IntProperty), widget.IntProperty, \"No_Widget\")", ""));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label override with related instance property got applied
    EXPECT_STREQ("Gadget_No_Widget", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceUpdated(m_db, *gadget);

    // still expect 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label has changed
    EXPECT_STREQ("Gadget_1", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterRelatedInstanceDelete)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "", "RulesEngineTest:Gadget", false));
    rule->GetSpecifications()[0]->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"Gadget_\" & IIF(NOT IsNull(widget.IntProperty), widget.IntProperty, \"No_Widget\")", ""));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label override with related instance property got applied
    EXPECT_STREQ("Gadget_1", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstancesChanged(m_db, bvector<ECInstanceChangeEventSource::ChangedECInstance>
        {
        ECInstanceChangeEventSource::ChangedECInstance(*m_widgetClass, (ECInstanceId)ECInstanceId::FromString(widget->GetInstanceId().c_str()), ChangeType::Delete),
        ECInstanceChangeEventSource::ChangedECInstance(*m_gadgetClass, (ECInstanceId)ECInstanceId::FromString(gadget->GetInstanceId().c_str()), ChangeType::Update),
        });

    // still expect 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label has changed
    EXPECT_STREQ("Gadget_No_Widget", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdateAfterRelatedInstanceUpdate)
    {
    ECRelationshipClassCP relationshipClass = m_schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();

    // insert a widget and a gadget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass);

    // relate the instances
    RulesEngineTestHelpers::InsertRelationship(m_db, *relationshipClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false,
        "", "RulesEngineTest:Gadget", false));
    rule->GetSpecifications()[0]->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"Gadget_\" & IIF(NOT IsNull(widget.IntProperty), widget.IntProperty, \"No_Widget\")", ""));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label override with related instance property got applied
    EXPECT_STREQ("Gadget_1", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    widget->SetValue("IntProperty", ECValue(2));
    ECInstanceUpdater(m_db, *widget, nullptr).Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // still expect 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // verify the label has changed
    EXPECT_STREQ("Gadget_2", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAllAffectedRootHierarchies)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("123"));}, true);

    // create 3 rulesets
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedRootHierarchies_1");
    m_locater->AddRuleSet(*rules1);
    RootNodeRule* rule1 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules1->AddPresentationRule(*rule1);
    rules1->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedRootHierarchies_2");
    m_locater->AddRuleSet(*rules2);
    RootNodeRule* rule2 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules2->AddPresentationRule(*rule2);
    rules2->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    PresentationRuleSetPtr rules3 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedRootHierarchies_3");
    m_locater->AddRuleSet(*rules3);
    RootNodeRule* rule3 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule3->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules3->AddPresentationRule(*rule3);
    rules3->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes1 = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules1->GetRuleSetId(), RulesetVariables())).get(); });
    DataContainer<NavNodeCPtr> rootNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules2->GetRuleSetId(), RulesetVariables())).get(); });
    DataContainer<NavNodeCPtr> rootNodes3 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules3->GetRuleSetId(), RulesetVariables())).get(); });

    // verify expected results
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("123", rootNodes1[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, rootNodes2.GetSize());
    EXPECT_STREQ("123", rootNodes2[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, rootNodes3.GetSize());

    // update the widget
    widget->SetValue("MyID", ECValue("456"));
    ECInstanceUpdater updater(m_db, *widget, nullptr);
    updater.Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // verify expected results
    rootNodes1 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules1->GetRuleSetId(), RulesetVariables())).get(); });
    rootNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules2->GetRuleSetId(), RulesetVariables())).get(); });
    rootNodes3 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules3->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("456", rootNodes1[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, rootNodes2.GetSize());
    EXPECT_STREQ("456", rootNodes2[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, rootNodes3.GetSize());

    // expect 2 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesAllAffectedChildHierarchies)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("123"));}, true);

    // create 3 rulesets
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedChildHierarchies_1");
    m_locater->AddRuleSet(*rules1);
    rules1->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    RootNodeRule* rootRule1 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule1->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_Type", "Root", "descr", "imageid"));
    rules1->AddPresentationRule(*rootRule1);
    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"TEST_Type\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules1->AddPresentationRule(*childRule1);

    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedChildHierarchies_2");
    m_locater->AddRuleSet(*rules2);
    rules2->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    RootNodeRule* rootRule2 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule2->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_Type", "Root", "descr", "imageid"));
    rules2->AddPresentationRule(*rootRule2);
    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.Type = \"TEST_Type\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules2->AddPresentationRule(*childRule2);

    PresentationRuleSetPtr rules3 = PresentationRuleSet::CreateInstance("UpdatesAllAffectedChildHierarchies_3");
    m_locater->AddRuleSet(*rules3);
    rules3->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));
    RootNodeRule* rootRule3 = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule3->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_Type", "Root", "descr", "imageid"));
    rules3->AddPresentationRule(*rootRule3);
    ChildNodeRule* childRule3 = new ChildNodeRule("ParentNode.Type = \"TEST_Type\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule3->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules3->AddPresentationRule(*childRule3);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes1 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules1->GetRuleSetId(), RulesetVariables())).get(); });
    DataContainer<NavNodeCPtr> rootNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules2->GetRuleSetId(), RulesetVariables())).get(); });
    DataContainer<NavNodeCPtr> rootNodes3 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules3->GetRuleSetId(), RulesetVariables())).get(); });

    // verify expected results
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("Root", rootNodes1[0]->GetLabelDefinition().GetDisplayValue().c_str());
    // expand node
    SetNodeExpanded(*rootNodes1[0]);
    DataContainer<NavNodeCPtr> childNodes1 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules1->GetRuleSetId(), RulesetVariables(), rootNodes1[0].get())).get(); });
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("123", childNodes1[0]->GetLabelDefinition().GetDisplayValue().c_str());

    ASSERT_EQ(1, rootNodes2.GetSize());
    EXPECT_STREQ("Root", rootNodes2[0]->GetLabelDefinition().GetDisplayValue().c_str());
    // expand node
    SetNodeExpanded(*rootNodes2[0]);
    DataContainer<NavNodeCPtr> childNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules2->GetRuleSetId(), RulesetVariables(), rootNodes2[0].get())).get(); });
    ASSERT_EQ(1, childNodes2.GetSize());
    EXPECT_STREQ("123", childNodes2[0]->GetLabelDefinition().GetDisplayValue().c_str());

    ASSERT_EQ(1, rootNodes3.GetSize());
    EXPECT_STREQ("Root", rootNodes3[0]->GetLabelDefinition().GetDisplayValue().c_str());
    // expand node
    SetNodeExpanded(*rootNodes3[0]);
    DataContainer<NavNodeCPtr> childNodes3 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules3->GetRuleSetId(), RulesetVariables(), rootNodes3[0].get())).get(); });
    ASSERT_EQ(0, childNodes3.GetSize());

    // update the widget
    widget->SetValue("MyID", ECValue("456"));
    ECInstanceUpdater updater(m_db, *widget, nullptr);
    updater.Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // verify expected results
    rootNodes1 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules1->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes1 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules1->GetRuleSetId(), RulesetVariables(), rootNodes1[0].get())).get(); });
    rootNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules2->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules2->GetRuleSetId(), RulesetVariables(), rootNodes2[0].get())).get(); });
    rootNodes3 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules3->GetRuleSetId(), RulesetVariables())).get(); });
    childNodes3 = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules3->GetRuleSetId(), RulesetVariables(), rootNodes3[0].get())).get(); });
    ASSERT_EQ(1, childNodes1.GetSize());
    EXPECT_STREQ("456", childNodes1[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(1, childNodes2.GetSize());
    EXPECT_STREQ("456", childNodes2[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_EQ(0, childNodes3.GetSize());

    // expect 2 update records
    // note: the order of records between different rulesets is undefined
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    auto rules1Records = ContainerHelpers::FindSubset<HierarchyUpdateRecord>(m_updateRecordsHandler->GetRecords(), [&](auto const& rec){return rec.GetRulesetId() == rules1->GetRuleSetId();});
    ASSERT_EQ(1, rules1Records.size());
    EXPECT_TRUE(rules1Records[0].GetParentNode()->Equals(*rootNodes1[0]));
    EXPECT_EQ(1, rules1Records[0].GetNodesCount());
    ASSERT_EQ(0, rules1Records[0].GetExpandedNodes().size());

    auto rules2Records = ContainerHelpers::FindSubset<HierarchyUpdateRecord>(m_updateRecordsHandler->GetRecords(), [&](auto const& rec){return rec.GetRulesetId() == rules2->GetRuleSetId();});
    ASSERT_EQ(1, rules2Records.size());
    EXPECT_TRUE(rules2Records[0].GetParentNode()->Equals(*rootNodes2[0]));
    EXPECT_EQ(1, rules2Records[0].GetNodesCount());
    ASSERT_EQ(0, rules2Records[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesParentsHasChildrenFlagWhenChildNodeIsInserted_WhenParentIsRoot)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, false, "custom", "custom", "custom", "custom");
    rootSpec->SetHasChildren(ChildrenHint::Unknown);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_FALSE(rootNodes[0]->HasChildren());
    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect the root node to have a "has children" flag set to "true"
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    // expect 1 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#759626
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesParentsHasChildrenFlagWhenChildNodeIsInserted_WhenParentIsNotRoot)
    {
    // set up
    RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR g){g.SetValue("Description", ECValue("1"));});
    RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR g){g.SetValue("Description", ECValue("2"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule1 = new RootNodeRule();
    rule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description=\"1\"", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rule1);

    ChildNodeRule* childRule1 = new ChildNodeRule();
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description=\"2\"", "RulesEngineTest:Gadget", false));
    rule1->GetSpecifications().front()->AddNestedRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule();
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Description=\"3\"", "RulesEngineTest:Gadget", false));
    childRule1->GetSpecifications().front()->AddNestedRule(*childRule2);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_FALSE(childNodes[0]->HasChildren());
    SetNodeExpanded(*childNodes[0]);
    DataContainer<NavNodeCPtr> grandchildNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); });
    ASSERT_EQ(0, grandchildNodes.GetSize());

    // insert another gadget
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR g){g.SetValue("Description", ECValue("3"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget);

    // expect the middle child node to have a "has children" flag set to "true"
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_TRUE(childNodes[0]->HasChildren());
    grandchildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); });
    ASSERT_EQ(1, grandchildNodes.GetSize());

    // verify records
    ASSERT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes()[0].GetNode()->Equals(*childNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[2].GetParentNode()->Equals(*childNodes[0]));
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[2].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesParentsHasChildrenFlagWhenChildNodeIsDeleted)
    {
    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, false, "custom", "custom", "custom", "custom");
    rootSpec->SetHasChildren(ChildrenHint::Unknown);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr deletedNode = childNodes[0];

    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // expect the root node to have a "has children" flag set to "false"
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_FALSE(rootNodes[0]->HasChildren());
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // expect 1 update records
    ASSERT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode()->Equals(*rootNodes[0]));

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, ShowsParentNodeWithHideIfNoChildrenFlagWhenChildNodeIsInserted)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, true, "custom", "custom", "custom", "custom");
    rootSpec->SetHasChildren(ChildrenHint::Unknown);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect the root node to get inserted
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // expect 1 update records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, RemovesParentNodeWithHideIfNoChildrenFlagWhenTheLastChildNodeIsDeleted)
    {
    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, TargetTree_Both, false);
    CustomNodeSpecification* rootSpec = new CustomNodeSpecification(1, true, "custom", "custom", "custom", "custom");
    rootSpec->SetHasChildren(ChildrenHint::Unknown);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"custom\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    NavNodeCPtr deletedRootNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr deletedChildNode = childNodes[0];

    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // expect the root node to be gone
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, DoesNotUpdateChildHierarchyIfParentIsRemoved)
    {
    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);

    // create the ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new CustomNodeSpecification(1, false, "TEST_TYPE", "Custom label", "", "ImageId"));
    rootRule->GetSpecifications()[0]->AddNestedRule(*childRule);

    ChildNodeRule* grandchildRule = new ChildNodeRule("ParentNode.Type = \"TEST_TYPE\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandchildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*grandchildRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // verify expected results
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr deletedRootNode = rootNodes[0];

    // expand node
    SetNodeExpanded(*rootNodes[0]);
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("Custom label", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expand node
    SetNodeExpanded(*childNodes[0]);
    DataContainer<NavNodeCPtr> grandchildNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); });
    ASSERT_EQ(1, grandchildNodes.GetSize());
    EXPECT_STREQ("WidgetID", grandchildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // delete the widget
    RulesEngineTestHelpers::DeleteInstance(m_db, *widget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *widget);

    // verify expected results
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, CustomizesInsertedNodes)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    RootNodeRule* rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, "RulesEngineTest"));
    rules->AddPresentationRule(*rule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(0, rootNodes.GetSize());

    // insert a widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *widget);

    // expect 1 root node now
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenNodeRemovedFromCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget2, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the root node is set as collapsed
    SetNodeExpanded(*rootNodes[0], false);

    // delete one gadget
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget2, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget2);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it now has 1 child node
    childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenNodeInsertedIntoCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget1, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", true));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the root node is set as collapsed
    SetNodeExpanded(*rootNodes[0], false);

    // relate second gadget
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget2, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget2);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it now has 2 children
    childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("GadgetID", childrenNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenNodeUpdatedInCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Widget_Label"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("Description", ECValue("Gadget_Label"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID,Description"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "Description"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Gadget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget_Label", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("Widget_Label", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the root node is set as collapsed
    SetNodeExpanded(*rootNodes[0], false);

    // update widget
    widget->SetValue("MyID", ECValue("New_Widget_Label"));
    ECInstanceUpdater updater(m_db, *widget, nullptr);
    updater.Update(*widget);
    m_db.SaveChanges();
    m_eventsSource->NotifyECInstanceUpdated(m_db, *widget);

    // expect no updates
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Gadget_Label", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it still has 1 child node but label is changed
    childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("New_Widget_Label", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, DoesNotUpdateHierarchyWhenLastChildrenRemovedForNodeThatHasCollapsedParent)
    {
    // insert some instances and create hierarchy
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCP gadgetHasSprocketsClass = m_db.Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(m_db, *m_sprocketClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("SprocketID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget);
    RulesEngineTestHelpers::InsertRelationship(m_db, *gadgetHasSprocketsClass, *gadget, *sprocket, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Sprocket", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    // gadget rule
    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule1->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule1);

    // sprocket rule
    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.ClassName=\"Gadget\"", 1, false, TargetTree_Both);
    childRule2->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket"));
    rules->AddPresentationRule(*childRule2);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    SetNodeExpanded(*rootNodes[0], false);

    // make sure it has 1 gadget node
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ("GadgetID", gadgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 sprocket node
    DataContainer<NavNodeCPtr> sprocketNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), gadgetNodes[0].get())).get(); });
    ASSERT_EQ(1, sprocketNodes.GetSize());
    EXPECT_STREQ("SprocketID", sprocketNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // remove sprocker
    RulesEngineTestHelpers::DeleteInstance(m_db, *sprocket, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *sprocket);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it still has 1 gadget node
    gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ("GadgetID", gadgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it now has 0 sprocket nodes
    sprocketNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), gadgetNodes[0].get())).get(); });
    ASSERT_EQ(0, sprocketNodes.GetSize());

    // expect no updates, because widget was collapsed
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, UpdateHierarchyWhenLastNodeRemovedFromCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the root node is set as collapsed
    SetNodeExpanded(*rootNodes[0], false);

    // delete gadget
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(rootNodes[0]->HasChildren());

    // make sure now it has 0 children
    childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childrenNodes.GetSize());

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, UpdateHierarchyWhenNodeInsertedIntoEmptyCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));}, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(rootNodes[0]->HasChildren());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childrenNodes.GetSize());

    // make sure the root node is set as collapsed
    SetNodeExpanded(*rootNodes[0], false);

    // relate gadget
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget, nullptr, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // make sure now it has 1 child node
    childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ("GadgetID", childrenNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdateTests, UpdateHierarchyWhenLastGroupedNodeDeletedFromCollapsedHierarchy)
    {
    // insert some instances
    ECRelationshipClassCP widgetHasGadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Widget\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // make sure it has 1 child grouping node
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childrenNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, childrenNodes[0]->GetType().c_str());
    EXPECT_EQ(1, childrenNodes[0]->GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());

    // make sure it has 1 child gadget node
    DataContainer<NavNodeCPtr> gadgetNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), childrenNodes[0].get())).get(); });
    ASSERT_EQ(1, gadgetNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, gadgetNodes[0]->GetType().c_str());
    EXPECT_STREQ("GadgetID", gadgetNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the root node is set as collapsed
    SetNodeExpanded(*rootNodes[0], false);

    // delete gadget
    RulesEngineTestHelpers::DeleteInstance(m_db, *gadget, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *gadget);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("WidgetID", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(rootNodes[0]->HasChildren());

    // make sure now it has 0 children
    childrenNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childrenNodes.GetSize());

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#887406
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HierarchyUpdateTests, UpdatesDataSourceAfterInsertWhenItAlreadyHasManyToManyRelatedInstanceNodeGroupedUnderVirtualPropertyGroupingNode)
    {
    ECRelationshipClassCP widgetHasGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP widgetsHaveGadgetsClass = m_db.Schemas().GetClass("RulesEngineTest", "WidgetsHaveGadgets")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("1"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetsHaveGadgetsClass, *widget, *gadget, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Gadget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "MyID"));
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Widget.Id = parent.ECInstanceId", "RulesEngineTest:Gadget", false));
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification("RulesEngineTest:WidgetsHaveGadgets", RequiredRelationDirection_Forward, "RulesEngineTest:Gadget"))}));
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });

    // expect 1 root node
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(rootNodes[0].IsValid());

    // expand root node
    SetNodeExpanded(*rootNodes[0]);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });

    // expect 1 child
    ASSERT_EQ(1, childNodes.GetSize());

    // add a new gadget
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(m_db, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("2"));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *widgetHasGadgetsClass, *widget, *gadget2, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *gadget2);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // make sure now it has 2 children nodes
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&](){ return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(2, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *gadget2);
    VerifyNodeInstance(*childNodes[1], *gadget);

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode()->Equals(*rootNodes[0]));
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesParentWhenNotFinalizedChildDatasourceBecomesEmpty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(HierarchyUpdateTests, UpdatesParentWhenNotFinalizedChildDatasourceBecomesEmpty)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(m_db, *classC, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "", classB->GetFullName(), false));
    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.ClassName = \"B\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*childRule1);
    rules->AddPresentationRule(*childRule2);

    // request root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // expect to have 1 children
    EXPECT_EQ(1, GetValidatedResponse(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()))));

    // remove the C instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *c, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *c);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // make sure we have 0 child nodes
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }


/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdateParentNodeWhenChildInstanceIsInserted_WithRulesetVariables, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(HierarchyUpdateTests, UpdateParentNodeWhenChildInstanceIsInserted_WithRulesetVariables)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "GetVariableBoolValue(\"show_nodes\")", classA->GetFullName(), false));
    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "GetVariableBoolValue(\"show_nodes\")", classB->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*childRule);

    // request root nodes
    m_manager->GetUserSettings().GetSettings(rules->GetRuleSetId()).SetSettingBoolValue("show_nodes", true);
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // expect to have 0 children
    EXPECT_EQ(0, GetValidatedResponse(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()))));

    // add the B instance
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(m_db, *classB, nullptr, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *b);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // make sure we have 1 child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesRootNodeWhenChildrenCountIsAffectedByChangeInGrandChildrenLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(HierarchyUpdateTests, UpdatesRootNodeWhenChildrenCountIsAffectedByChangeInGrandChildrenLevel)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(m_db, *classC, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false, RuleTargetTree::TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "", classB->GetFullName(), false));
    ChildNodeRule* grandChildRule = new ChildNodeRule("ParentNode.ClassName = \"B\"", 1, false, RuleTargetTree::TargetTree_Both);
    grandChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*childRule);
    rules->AddPresentationRule(*grandChildRule);

    // request root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(1, childNodes.GetSize());

    DataContainer<NavNodeCPtr> grandChildNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); });
    ASSERT_EQ(1, grandChildNodes.GetSize());

    // remove C instance
    RulesEngineTestHelpers::DeleteInstance(m_db, *c, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *c);

    // make sure we still have 1 root node
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, rootNodes.GetSize());

    // make sure we have 0 child node
    childNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); });
    ASSERT_EQ(0, childNodes.GetSize());

    // verify records
    ASSERT_EQ(1, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdateFilteredRootHierarchyLevel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, UpdateFilteredRootHierarchyLevel)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(2));});
    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";
    m_uiState->SetInstanceFilter(nullptr, filter);

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    params.SetInstanceFilter(filter);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    // add an instance that doesn't match the filter
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(3)); }, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a3);

    // verify hierarchy didn't change
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    // expect 1 update record - the hierarchy level gets reported even when it doesn't change (we don't want to compare individual nodes)
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    m_updateRecordsHandler->Clear();

    // add another instance matching the filter
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1));}, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a4);

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a4 }),
        });

    // expect 1 update record
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords().size());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdateFilteredChildHierarchyLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdateFilteredChildHierarchyLevel)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1);

    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";
    auto aChildrenFilterSetter = [&](AsyncHierarchyRequestParams& p)
        {
        NavNodeKeyCP parentKey = p.GetParentNode() ? p.GetParentNode()->GetKey().get() : p.GetParentNodeKey() ? p.GetParentNodeKey() : nullptr;
        if (parentKey && ContainerHelpers::Contains(parentKey->AsECInstanceNodeKey()->GetInstanceKeys(), [&](auto const& k){return k.GetClass()->GetName().Equals("A");}))
            p.SetInstanceFilter(filter);
        };

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    m_uiState->SetInstanceFilter(hierarchy[0].node.get(), filter);
    m_uiState->AddExpandedNode(hierarchy[0].node->GetKey());

    // add an instance that doesn't match the filter
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(3));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b3, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a.get(), b3.get() });

    // verify the hierarchy didn't change
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    // expect 2 update records - the hierarchy levels gets reported even when they don't change (we don't want to compare individual nodes)
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode(), *a);

    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[1].GetParentNode(), *a);
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());

    m_updateRecordsHandler->Clear();

    // add another instance matching the filter
    IECInstancePtr b4 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b4, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a.get(), b4.get() });

    // validate hierarchy post-update
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b4 }),
            }),
        });

    // expect 2 update records
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode(), *a);

    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[1].GetParentNode(), *a);
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }
    
/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesFilteredParentNodeWhenItHasHideIfNoChildrenFlagAndChildrenAreRemoved, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdatesFilteredParentNodeWhenItHasHideIfNoChildrenFlagAndChildrenAreRemoved)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1);

    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";
    auto aChildrenFilterSetter = [&](AsyncHierarchyRequestParams& p)
        {
        NavNodeKeyCP parentKey = p.GetParentNode() ? p.GetParentNode()->GetKey().get() : p.GetParentNodeKey() ? p.GetParentNodeKey() : nullptr;
        if (parentKey && ContainerHelpers::Contains(parentKey->AsECInstanceNodeKey()->GetInstanceKeys(), [&](auto const& k){return k.GetClass()->GetName().Equals("A");}))
            p.SetInstanceFilter(filter);
        };

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    m_uiState->SetInstanceFilter(hierarchy[0].node.get(), filter);
    m_uiState->AddExpandedNode(hierarchy[0].node->GetKey());

    // remove the instance matching the filter
    RulesEngineTestHelpers::DeleteInstance(m_db, *b1, true);
    m_eventsSource->NotifyECInstanceDeleted(m_db, *b1);

    // validate hierarchy post-update
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        // notes: 
        // - even though 'a' has 'hide if no children' flag and has no children, we still show it
        //   or otherwise there would be no way to clear the filter and get it back
        // - even though the node has no children, it still has the 'has children' flag set to 
        //   'true' - we don't know the filter for 'a' node when creating it
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }), true,
            {
            }),
        });

    // expect 2 update records
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode(), *a);

    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[1].GetParentNode(), *a);
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesFilteredParentNodeWhenItHasHideIfNoChildrenFlagAndChildrenAreInserted, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdatesFilteredParentNodeWhenItHasHideIfNoChildrenFlagAndChildrenAreInserted)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1)); });
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 2";
    auto aChildrenFilterSetter = [&](AsyncHierarchyRequestParams& p)
        {
        NavNodeKeyCP parentKey = p.GetParentNode() ? p.GetParentNode()->GetKey().get() : p.GetParentNodeKey() ? p.GetParentNodeKey() : nullptr;
        if (parentKey && ContainerHelpers::Contains(parentKey->AsECInstanceNodeKey()->GetInstanceKeys(), [&](auto const& k){return k.GetClass()->GetName().Equals("A"); }))
            p.SetInstanceFilter(filter);
        };

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }), true,
            {
            }),
        });

    m_uiState->SetInstanceFilter(hierarchy[0].node.get(), filter);
    m_uiState->AddExpandedNode(hierarchy[0].node->GetKey());

    // insert an instance matching the filter
    auto b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a.get(), b2.get() });

    // validate hierarchy post-update
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // expect 2 update records
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode(), *a);

    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[1].GetParentNode(), *a);
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesFilteredHierarchyLevelWhenNodesUnderVirtualParentAreUpdated, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdatesFilteredHierarchyLevelWhenNodesUnderVirtualParentAreUpdated)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(m_db, *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b1);

    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b2);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    auto groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), classB->GetName(), "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "Prop"));
    rules->AddPresentationRule(*groupingRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";
    auto aChildrenFilterSetter = [&](AsyncHierarchyRequestParams& p)
        {
        NavNodeKeyCP parentKey = p.GetParentNode() ? p.GetParentNode()->GetKey().get() : p.GetParentNodeKey() ? p.GetParentNodeKey() : nullptr;
        if (!parentKey)
            return;

        bool isParentAInstanceNode = parentKey->AsECInstanceNodeKey()
            && ContainerHelpers::Contains(parentKey->AsECInstanceNodeKey()->GetInstanceKeys(), [&](auto const& k){return k.GetClass()->GetName().Equals("A");});
        bool isParentAPropertyGroupingNode = parentKey->AsECPropertyGroupingNodeKey()
            && &parentKey->AsECPropertyGroupingNodeKey()->GetECClass() == classA;
        if (isParentAInstanceNode || isParentAPropertyGroupingNode)
            {
            p.SetInstanceFilter(filter);
            }
        };

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    m_uiState->SetInstanceFilter(hierarchy[0].node.get(), filter);
    m_uiState->AddExpandedNode(hierarchy[0].node->GetKey());

    // insert an instance matching the filter
    auto b3 = RulesEngineTestHelpers::InsertInstance(m_db, *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a, *b3, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a.get(), b3.get() });

    // validate hierarchy post-update
    ValidateHierarchy(params, aChildrenFilterSetter,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ b1.get(), b3.get() }, { ECValue(1) }),
                {
                CreateInstanceNodeValidator({ b1 }),
                CreateInstanceNodeValidator({ b3 }),
                }),
            }),
        });

    // expect 2 update records
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode(), *a);

    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[1].GetParentNode(), *a);
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdatesHierarchyLevelWhenNodesUnderFilteredParentAreUpdated, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(HierarchyUpdateTests, UpdatesHierarchyLevelWhenNodesUnderFilteredParentAreUpdated)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr a0 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(0));});

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b1);

    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up hierarchy level instance filter
    Utf8CP filter = "this.Prop = 1";
    m_uiState->SetInstanceFilter(nullptr, filter);
    
    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());
    params.SetInstanceFilter(filter);
    auto hierarchy = ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });

    m_uiState->AddExpandedNode(hierarchy[0].node->GetKey());

    // insert another B instance
    auto b2 = RulesEngineTestHelpers::InsertInstance(m_db, *classB);
    RulesEngineTestHelpers::InsertRelationship(m_db, *relAB, *a1, *b2, nullptr, true);
    m_eventsSource->NotifyECInstancesInserted(m_db, { a1.get(), b2.get() });

    // validate hierarchy post-update
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // expect 2 update records
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes()[0].GetNode(), *a1);

    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[1].GetParentNode(), *a1);
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UpdateHierarchyLevelFilteredWithMultipleFilters, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchyUpdateTests, UpdateHierarchyLevelFilteredWithMultipleFilters)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(2)); });
    m_db.SaveChanges();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), false, { classA->GetName() }),
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up hierarchy level instance filter
    Utf8CP filter1 = "this.Prop = 1";
    Utf8CP filter2 = "this.Prop = 2";
    m_uiState->SetInstanceFilters(nullptr, { filter1, filter2, ""});

    // validate hierarchy pre-update
    auto params = AsyncHierarchyRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables());

    params.SetInstanceFilter(filter1);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    params.SetInstanceFilter(filter2);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });
    
    params.SetInstanceFilter("");
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        });

    // add an instance that doesn't match any of the filters
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(3)); }, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a3);

    // verify hierarchy changes
    params.SetInstanceFilter(filter1);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        });

    params.SetInstanceFilter(filter2);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });

    params.SetInstanceFilter("");
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        CreateInstanceNodeValidator({ a3 }),
        });

    // expect 3 update records, 1 for each variation
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_STREQ(filter1, m_updateRecordsHandler->GetRecords()[0].GetInstanceFilter().c_str());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode().IsNull());
    EXPECT_STREQ(filter2, m_updateRecordsHandler->GetRecords()[1].GetInstanceFilter().c_str());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[2].GetParentNode().IsNull());
    EXPECT_STREQ("", m_updateRecordsHandler->GetRecords()[2].GetInstanceFilter().c_str());
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords()[2].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetExpandedNodes().size());

    m_updateRecordsHandler->Clear();

    // add another instance matching one of the filters
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(m_db, *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); }, true);
    m_eventsSource->NotifyECInstanceInserted(m_db, *a4);

    // verify hierarchy changes
    params.SetInstanceFilter(filter1);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a4 }),
        });

    params.SetInstanceFilter(filter2);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });

    params.SetInstanceFilter("");
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        CreateInstanceNodeValidator({ a3 }),
        CreateInstanceNodeValidator({ a4 }),
        });

    // expect 3 update records again
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[0].GetParentNode().IsNull());
    EXPECT_STREQ(filter1, m_updateRecordsHandler->GetRecords()[0].GetInstanceFilter().c_str());
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords()[0].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[0].GetExpandedNodes().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[1].GetParentNode().IsNull());
    EXPECT_STREQ(filter2, m_updateRecordsHandler->GetRecords()[1].GetInstanceFilter().c_str());
    EXPECT_EQ(1, m_updateRecordsHandler->GetRecords()[1].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[1].GetExpandedNodes().size());

    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[2].GetParentNode().IsNull());
    EXPECT_STREQ("", m_updateRecordsHandler->GetRecords()[2].GetInstanceFilter().c_str());
    EXPECT_EQ(4, m_updateRecordsHandler->GetRecords()[2].GetNodesCount());
    ASSERT_EQ(0, m_updateRecordsHandler->GetRecords()[2].GetExpandedNodes().size());
    }
