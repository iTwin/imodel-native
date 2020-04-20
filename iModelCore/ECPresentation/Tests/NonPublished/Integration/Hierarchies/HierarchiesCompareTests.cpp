/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../PresentationManagerIntegrationTests.h"
#include "../../../../Source/RulesDriven/RulesEngine/PresentationManagerImpl.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
struct HierarchiesCompareTests : PresentationManagerIntegrationTests
    {
    std::shared_ptr<TestUpdateRecordsHandler> m_updateRecordsHandler;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2020
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _ConfigureManagerParams(RulesDrivenECPresentationManager::Params& params) override
        {
        PresentationManagerIntegrationTests::_ConfigureManagerParams(params);
        params.SetMode(RulesDrivenECPresentationManager::Mode::ReadOnly);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2020
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetUp() override
        {
        m_updateRecordsHandler = std::make_shared<TestUpdateRecordsHandler>();
        PresentationManagerIntegrationTests::SetUp();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2020
    +---------------+---------------+---------------+---------------+---------------+------*/
    folly::Future<NavNodesContainer> GetNodes(PageOptions const& pageOptions, JsonValueCR options, NavNodeCP parentNode)
        {
        if (nullptr == parentNode)
            return m_manager->GetRootNodes(s_project->GetECDb(), pageOptions, options);
        return m_manager->GetChildren(s_project->GetECDb(), *parentNode, pageOptions, options);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2020
    +---------------+---------------+---------------+---------------+---------------+------*/
    folly::Future<folly::Unit> TraverseHierarchy(RulesDrivenECPresentationManager::NavigationOptions const& options, NavNodeCP parent = nullptr)
        {
        return GetNodes(PageOptions(), options.GetJson(), parent).then([this, options](NavNodesContainer nodes)
            {
            bvector<folly::Future<folly::Unit>> childrenFutures;
            for (NavNodeCPtr node : nodes)
                {
                if (node->HasChildren())
                    childrenFutures.push_back(TraverseHierarchy(options, node.get()));
                }
            return folly::collectAll(childrenFutures).then();
            });
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void VerifyNodeInstances(NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> instances)
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
static void VerifyNodeInstance(NavNodeCR node, IECInstanceCR instance)
    {
    VerifyNodeInstances(node, {&instance});
    }

enum Side { LHS, RHS };
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateRulesetName(Side side)
    {
    return Utf8PrintfString("%s:%s", BeTest::GetNameOfCurrentTest(), side == LHS ? "LHS" : side == RHS ? "RHS" : "?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchiesCompareTests, DetectsNoCustomNodeChangesWhenRulesetsAreEqual)
    {
    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*lhs);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new CustomNodeSpecification(1, false, "T_NODE", "LABEL", "", ""));
    lhs->AddPresentationRule(*rootRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS), 1, 0, false, "", "", "", false);
    rhs->AddPresentationRule(*new RootNodeRule(*rootRule));
    m_locater->AddRuleSet(*rhs);

    // prepare for comparison
    TraverseHierarchy(RulesDrivenECPresentationManager::NavigationOptions(lhs->GetRuleSetId().c_str())).wait();

    // compare
    m_manager->CompareHierarchies(m_updateRecordsHandler, s_project->GetECDb(), lhs->GetRuleSetId(), rhs->GetRuleSetId(), Json::Value()).wait();

    // verify
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsNoECInstanceNodeChangesWhenRulesetsAreEqual, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(HierarchiesCompareTests, DetectsNoECInstanceNodeChangesWhenRulesetsAreEqual)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*lhs);

    RootNodeRule* rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*rootRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS), 1, 0, false, "", "", "", false);
    rhs->AddPresentationRule(*new RootNodeRule(*rootRule));
    m_locater->AddRuleSet(*rhs);

    // prepare for comparison
    TraverseHierarchy(RulesDrivenECPresentationManager::NavigationOptions(lhs->GetRuleSetId().c_str())).wait();

    // compare
    m_manager->CompareHierarchies(m_updateRecordsHandler, s_project->GetECDb(), lhs->GetRuleSetId(), rhs->GetRuleSetId(), Json::Value()).wait();

    // verify
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsRootNodeChangesWhenInstanceFilterIsModified, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, DetectsRootNodeChangesWhenInstanceFilterIsModified)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(3));});

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Prop = 1 OR this.Prop = 3", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Prop = 2 OR this.Prop = 3", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsRule);

    // prepare for comparison
    TraverseHierarchy(RulesDrivenECPresentationManager::NavigationOptions(lhs->GetRuleSetId().c_str())).wait();

    // compare
    m_manager->CompareHierarchies(m_updateRecordsHandler, s_project->GetECDb(), lhs->GetRuleSetId(), rhs->GetRuleSetId(), Json::Value()).wait();

    // verify
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[recordIndex].GetNode(), *element1);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[recordIndex].GetNode(), *element2);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[recordIndex].GetNode(), *element3);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsChildNodeChangesWhenInstanceFilterIsModified, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, DetectsChildNodeChangesWhenInstanceFilterIsModified)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(2)); });

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRootRule->AddSpecification(*new CustomNodeSpecification(1, false, "T_NODE", "LABEL", "", ""));
    lhs->AddPresentationRule(*lhsRootRule);
    ChildNodeRule* lhsChildRule = new ChildNodeRule("ParentNode.Type = \"T_NODE\"", 1, false);
    lhsChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Prop = 1", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsChildRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRootRule->AddSpecification(*new CustomNodeSpecification(1, false, "T_NODE", "LABEL", "", ""));
    rhs->AddPresentationRule(*rhsRootRule);
    ChildNodeRule* rhsChildRule = new ChildNodeRule("ParentNode.Type = \"T_NODE\"", 1, false);
    rhsChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "this.Prop = 2", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsChildRule);

    // prepare for comparison
    TraverseHierarchy(RulesDrivenECPresentationManager::NavigationOptions(lhs->GetRuleSetId().c_str())).wait();

    // compare
    m_manager->CompareHierarchies(m_updateRecordsHandler, s_project->GetECDb(), lhs->GetRuleSetId(), rhs->GetRuleSetId(), Json::Value()).wait();

    // verify
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[recordIndex].GetNode(), *element1);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[recordIndex].GetNode(), *element2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsClassGroupingNodeChangesWhenTheirChildNodesChange, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, DetectsClassGroupingNodeChangesWhenTheirChildNodesChange)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(2)); });

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false,
        "this.Prop = 1", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false,
        "this.Prop = 2", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsRule);

    // prepare for comparison
    TraverseHierarchy(RulesDrivenECPresentationManager::NavigationOptions(lhs->GetRuleSetId().c_str())).wait();

    // compare
    m_manager->CompareHierarchies(m_updateRecordsHandler, s_project->GetECDb(), lhs->GetRuleSetId(), rhs->GetRuleSetId(), Json::Value()).wait();

    // verify
    EXPECT_EQ(3, m_updateRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Update, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode));
    EXPECT_EQ(elementClass, &m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetKey()->AsECClassGroupingNodeKey()->GetECClass());
    ++recordIndex;
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[recordIndex].GetNode(), *element1);
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[recordIndex].GetPosition());
    VerifyNodeInstance(*m_updateRecordsHandler->GetRecords()[recordIndex].GetNode(), *element2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsLabelGroupingNodeChangesWhenTheirChildNodesChange, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="PropA" typeName="string" />
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, DetectsLabelGroupingNodeChangesWhenTheirChildNodesChange)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("PropA", ECValue("A"));
        instance.SetValue("PropB", ECValue("B"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("PropA", ECValue("A"));
        instance.SetValue("PropB", ECValue("B"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("PropA", ECValue("C"));
        instance.SetValue("PropB", ECValue("C"));
        });

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true,
        "", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsRule);
    lhs->AddPresentationRule(*new InstanceLabelOverride(1, false, elementClass->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("PropA") }));

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true,
        "", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsRule);
    rhs->AddPresentationRule(*new InstanceLabelOverride(1, false, elementClass->GetFullName(), {new InstanceLabelOverridePropertyValueSpecification("PropB")}));

    // prepare for comparison
    TraverseHierarchy(RulesDrivenECPresentationManager::NavigationOptions(lhs->GetRuleSetId().c_str())).wait();

    // compare
    m_manager->CompareHierarchies(m_updateRecordsHandler, s_project->GetECDb(), lhs->GetRuleSetId(), rhs->GetRuleSetId(), Json::Value()).wait();

    // verify
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;

    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    EXPECT_STREQ("A", m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
    EXPECT_STREQ("B", m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsPropertyGroupingNodeChangesWhenTheirChildNodesChange, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="PropA" typeName="string" />
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(HierarchiesCompareTests, DetectsPropertyGroupingNodeChangesWhenTheirChildNodesChange)
    {
    // prepare the dataset
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("PropA", ECValue("A"));
        instance.SetValue("PropB", ECValue("B"));
        });

    // create lhs ruleset
    PresentationRuleSetPtr lhs = PresentationRuleSet::CreateInstance(CreateRulesetName(LHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*lhs);
    RootNodeRule* lhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    lhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true,
        "", elementClass->GetFullName(), false));
    lhs->AddPresentationRule(*lhsRule);
    GroupingRule* lhsGroupingRule = new GroupingRule("", 1, false, elementClass->GetSchema().GetName(), elementClass->GetName(), "", "", "");
    lhsGroupingRule->AddGroup(*new PropertyGroup("", "", true, "PropA"));
    lhs->AddPresentationRule(*lhsGroupingRule);

    // create rhs ruleset
    PresentationRuleSetPtr rhs = PresentationRuleSet::CreateInstance(CreateRulesetName(RHS), 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rhs);
    RootNodeRule* rhsRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_Both, false);
    rhsRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true,
        "", elementClass->GetFullName(), false));
    rhs->AddPresentationRule(*rhsRule);
    GroupingRule* rhsGroupingRule = new GroupingRule("", 1, false, elementClass->GetSchema().GetName(), elementClass->GetName(), "", "", "");
    rhsGroupingRule->AddGroup(*new PropertyGroup("", "", true, "PropB"));
    rhs->AddPresentationRule(*rhsGroupingRule);

    // prepare for comparison
    TraverseHierarchy(RulesDrivenECPresentationManager::NavigationOptions(lhs->GetRuleSetId().c_str())).wait();

    // compare
    m_manager->CompareHierarchies(m_updateRecordsHandler, s_project->GetECDb(), lhs->GetRuleSetId(), rhs->GetRuleSetId(), Json::Value()).wait();

    // verify
    EXPECT_EQ(2, m_updateRecordsHandler->GetRecords().size());
    size_t recordIndex = 0;
    
    EXPECT_EQ(ChangeType::Delete, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode));
    EXPECT_STREQ("A", m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    ++recordIndex;

    EXPECT_EQ(ChangeType::Insert, m_updateRecordsHandler->GetRecords()[recordIndex].GetChangeType());
    EXPECT_EQ(0, m_updateRecordsHandler->GetRecords()[recordIndex].GetPosition());
    EXPECT_TRUE(m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode));
    EXPECT_STREQ("B", m_updateRecordsHandler->GetRecords()[recordIndex].GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    }