/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "HierarchyIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodeSpecificationP RulesDrivenECPresentationManagerNavigationTests::CreateCustomNodeSpecification(Utf8String typeAndLabel, std::function<void(CustomNodeSpecificationR)> configure)
    {
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, false, typeAndLabel, typeAndLabel, typeAndLabel, typeAndLabel);
    spec->SetHasChildren(ChildrenHint::Unknown);
    if (nullptr != configure)
        configure(*spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_NotGrouped, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_NotGrouped)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // expect two nodes
    ASSERT_EQ(2, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    VerifyNodeInstance(*nodes[0], *instanceA);

    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[1]->GetType().c_str());
    VerifyNodeInstance(*nodes[1], *instanceB);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupedByClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, true, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    EXPECT_EQ(classA->GetId(), nodes[0]->GetKey()->AsECClassGroupingNodeKey()->GetECClassId());
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classANodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classANodes[0]->GetType().c_str());
    VerifyNodeInstance(*classANodes[0], *instanceA);

    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[1]->GetType().c_str());
    EXPECT_EQ(classB->GetId(), nodes[1]->GetKey()->AsECClassGroupingNodeKey()->GetECClassId());
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get())).get(); }
        );
    ASSERT_EQ(1, classBNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classBNodes[0]->GetType().c_str());
    VerifyNodeInstance(*classBNodes[0], *instanceB);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_AlwaysReturnsChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_AlwaysReturnsChildren)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr node = nodes[0];
    EXPECT_EQ(ChildrenHint::Always, NavNodeExtendedData(*node).GetChildrenHint());
    EXPECT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_DoNotSort_ReturnsUnsortedNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_DoNotSort_ReturnsUnsortedNodes)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Instance2", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Instance1", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, true, false, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, true, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    allInstanceNodesSpecification->AddNestedRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_HideNodesInHierarchy, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideNodesInHierarchy)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, true, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    allInstanceNodesSpecification->AddNestedRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    ASSERT_STREQ("test", instanceNode->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ParentIdSpecifiedUsingHexInteger_ReturnsNode, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ParentIdSpecifiedUsingHexInteger_ReturnsNode)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    ECInstanceId aInstanceId;
    ECInstanceId::FromString(aInstanceId, a->GetInstanceId().c_str());

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false));
    childRule->SetCondition("ParentNode.IsInstanceNode And ParentNode.InstanceId = " + aInstanceId.ToHexStr());
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions) { return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&]() { return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
    );

    // make sure we have one root node
    ASSERT_EQ(1, rootNodes.GetSize());

    // request for child nodes
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions) { return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&]() { return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
    );

    // make sure we have one child node
    ASSERT_EQ(1, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ECInstanceIdSpecifiedUsingHexInteger_ReturnsFilteredNode, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ECInstanceIdSpecifiedUsingHexInteger_ReturnsFilteredNode)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    ECInstanceId a2InstanceId;
    ECInstanceId::FromString(a2InstanceId, a2->GetInstanceId().c_str());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.ECInstanceId = " + a2InstanceId.ToHexStr(), classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions) { return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&]() { return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
    );

    // make sure we get correct node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *a2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_HideExpression_ReturnsEmptyListIfExpressionEvalutesToTrue, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideExpression_ReturnsEmptyListIfExpressionEvalutesToTrue)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP spec = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    spec->SetHideExpression(Utf8PrintfString("ThisNode.IsOfClass(\"A\", \"%s\")", GetSchema()->GetName().c_str()));
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_HideExpression_ReturnsNodesIfExpressionEvaluatesToFalse, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_HideExpression_ReturnsNodesIfExpressionEvaluatesToFalse)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP spec = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    spec->SetHideExpression(Utf8PrintfString("ThisNode.IsOfClass(\"B\", \"%s\"", GetSchema()->GetName().c_str()));
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupedByLabel_DoesntGroup1Instance, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByLabel_DoesntGroup1Instance)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("TestData"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, true, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("TestData", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupedByLabelGroups3InstancesWith1GroupingNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByLabelGroups3InstancesWith1GroupingNode)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, true, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("InstanceA", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    NavNodeCPtr labelGroupingNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("InstanceB", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure we have two B instances
    DataContainer<NavNodeCPtr> instanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get())).get(); }
        );
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupedByLabelGroups4InstancesWith2GroupingNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllInstanceNodes_GroupedByLabelGroups4InstancesWith2GroupingNodes)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, true, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> labelGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, labelGroupingNodes.GetSize());

    NavNodeCPtr labelGroupingNode = labelGroupingNodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("InstanceA", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 A instances
    DataContainer<NavNodeCPtr> instanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get())).get(); }
        );
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());

    labelGroupingNode = labelGroupingNodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("InstanceB", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 B instances
    instanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get())).get(); }
        );
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RemovesLabelGroupingNodeIfOnlyOneChild, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, RemovesLabelGroupingNodeIfOnlyOneChild)
    {
    ECClassCP classA = GetClass("A");

    // make sure there are instances with unique labels and instances with same labels
    IECInstancePtr instanceWithUniqueLabel = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance) { instance.SetValue("Property", ECValue("Unique Label")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance) { instance.SetValue("Property", ECValue("Same Label")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance) { instance.SetValue("Property", ECValue("Same Label")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance) { instance.SetValue("Property", ECValue("Same Label")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AlwaysReturnsResultsFlag_SetToNodeFromSpecification, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, AlwaysReturnsResultsFlag_SetToNodeFromSpecification)
    {
    ECClassCP classA = GetClass("A");

    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, true, false, false, false, false, GetSchema()->GetName()));
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure the parent node has the "always returns results" flag
    ASSERT_EQ(2, nodes.GetSize());
    EXPECT_EQ(ChildrenHint::Always, NavNodeExtendedData(*nodes[0]).GetChildrenHint());
    EXPECT_EQ(ChildrenHint::Unknown, NavNodeExtendedData(*nodes[1]).GetChildrenHint());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideIfNoChildren_ReturnsEmptyListIfNoChildren, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert an instance
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    childRule->SetCondition("ParentNode.IsInstanceNode And ParentNode.ClassName = \"B\"");
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure the node was hidden
    ASSERT_TRUE(0 == nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideIfNoChildren_ReturnsNodesIfHasChildren, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert instance B
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // insert instance A
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    childRule->SetCondition("ParentNode.IsInstanceNode And ParentNode.ClassName = \"B\"");
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure the node was not hidden
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideIfNoChildren_IgnoredIfHasAlwaysReturnsNodesFlag, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideIfNoChildren_IgnoredIfHasAlwaysReturnsNodesFlag)
    {
    ECClassCP classA = GetClass("A");

    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, true, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure the node was not hidden
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideNodesInHierarchy_ReturnsChildNodes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideNodesInHierarchy_ReturnsChildNodes)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // add B instance
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // add A instance
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    childRule->SetCondition("ParentNode.IsInstanceNode");
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we get instance A, not instance B
    ASSERT_EQ(1, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *instanceA);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideNodesInHierarchy_ReturnsNoChildrenWhenThereAreNoChildSpecifications, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, HideNodesInHierarchy_ReturnsNoChildrenWhenThereAreNoChildSpecifications)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // add B instance
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // add A instance
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", classA->GetFullName(), false));
    childRule->SetCondition("ParentNode.IsInstanceNode AND ParentNode.ClassName=\"B\"");
    rules->AddPresentationRule(*childRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // expect 1 B node
    ASSERT_EQ(1, rootNodes.GetSize());

    // request for child nodes
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );

    // expect empty container
    ASSERT_EQ(0, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideNodesInHierarchy_WithGrouping_ReturnsGroupingButNotInstanceNodes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, HideNodesInHierarchy_WithGrouping_ReturnsGroupingButNotInstanceNodes)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, true, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"B\"", 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Backward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    ASSERT_TRUE(nullptr != rootNodes[0]->GetKey()->AsECClassGroupingNodeKey());
    EXPECT_EQ(classB, &rootNodes[0]->GetKey()->AsECClassGroupingNodeKey()->GetECClass());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *a);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_Type_Label_Description_ImageId)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "label", "description", "imageid");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    ASSERT_STREQ("type", instanceNode->GetType().c_str());
    ASSERT_STREQ("label", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("description", instanceNode->GetDescription().c_str());
    ASSERT_STREQ("imageid", instanceNode->GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("a"));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*CreateCustomNodeSpecification("b"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("type", [](CustomNodeSpecificationR spec) { spec.SetHideIfNoChildren(true); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideNodesInHierarchy_ReturnsEmptyList)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("type", [](CustomNodeSpecificationR spec) {spec.SetHideNodesInHierarchy(true); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideNodesInHierarchy_ReturnsEmptyListForRootNode)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("type", [](CustomNodeSpecificationR spec) {spec.SetHideNodesInHierarchy(true); }));
    rules->AddPresentationRule(*rootRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideExpression_ReturnsEmptyListIfExpressionEvalutesToTrue)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("T_MyType", [](CustomNodeSpecificationR spec) {spec.SetHideExpression("ThisNode.Type = \"T_MyType\""); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_HideExpression_ReturnsNodesIfExpressionEvaluatesToFalse)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("T_MyType", [](CustomNodeSpecificationR spec) {spec.SetHideExpression("ThisNode.Type = \"T_OtherType\""); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CustomNodes_AlwaysReturnsChildren_NodeHasChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_AlwaysReturnsChildren_NodeHasChildren)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*CreateCustomNodeSpecification("type", [](CustomNodeSpecificationR spec) {spec.SetHasChildren(ChildrenHint::Always); }));
    rules->AddPresentationRule(*rule);

    // request for nodes
    ASSERT_EQ(1, GetValidatedResponse(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()))));

    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr node = nodes[0];
    EXPECT_TRUE(node->HasChildren());
    EXPECT_TRUE(node->DeterminedChildren());
    }

/*---------------------------------------------------------------------------------**//**
* Note: In this test we initialize data providers for the first 2 specs when determining
* root node's children. The first spec returns 0 nodes and the second one returns 1 node.
* Overall the root node should have 2 children - 2'nd and 3'rd.
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CustomNodes_ReturnsCorrectChildNodesWhenOneOfThemIsHiddenAndCombinedHierarchyLevelIsNotInitialized, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_ReturnsCorrectChildNodesWhenOneOfThemIsHiddenAndCombinedHierarchyLevelIsNotInitialized)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("child2", childNodes[0]->GetType().c_str());
    EXPECT_STREQ("child3", childNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Recursion prevention (VSTS#102711)
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_ReturnNodesOfTheSameSpecificationAlreadyExistingInHierarchyWithoutChildren)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("T_ROOT"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*CreateCustomNodeSpecification("T_CHILD"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 T_ROOT node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("T_ROOT", rootNodes[0]->GetType().c_str());

    // T_ROOT node should have 1 T_CHILD node and it should have no children
    DataContainer<NavNodeCPtr> rootChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, rootChildNodes.GetSize());
    EXPECT_STREQ("T_CHILD", rootChildNodes[0]->GetType().c_str());
    EXPECT_TRUE(rootChildNodes[0]->HasChildren());

    // T_CHILD node should have 1 T_CHILD node and it should have no children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootChildNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootChildNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("T_CHILD", childNodes[0]->GetType().c_str());
    EXPECT_FALSE(childNodes[0]->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomNodes_CustomizesLabelAfterFirstRequestingNodesCount)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root"));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.Type = \"root\"", 1, "\"custom label\"", ""));

    // first, request the count
    EXPECT_EQ(1, GetValidatedResponse(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()))));

    // then verify the label is customized
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("root", rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("custom label", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_AlwaysReturnsChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_AlwaysReturnsChildren)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr node = nodes[0];
    EXPECT_EQ(ChildrenHint::Always, NavNodeExtendedData(*node).GetChildrenHint());
    EXPECT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_HideNodesInHierarchy, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_HideNodesInHierarchy)
    {
    ECClassCP classA = GetClass("A");

    // insert some instance
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    ASSERT_STREQ("test", instanceNode->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_HideIfNoChildren_ReturnsEmptyListIfNoChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_HideIfNoChildren_ReturnsNodesIfHasChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_GroupedByClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", Utf8PrintfString("%s,%s", classB->GetFullName(), classA->GetName().c_str()), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_GroupedByLabel_DoesntGroup1Instance, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByLabel_DoesntGroup1Instance)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("InstanceA", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_GroupedByLabel_Groups3InstancesWith1GroupingNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByLabel_Groups3InstancesWith1GroupingNode)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", Utf8PrintfString("%s,%s", classB->GetFullName(), classA->GetName().c_str()), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 A instance node
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("InstanceA", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 B label grouping node
    NavNodeCPtr labelGroupingNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("InstanceB", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure we have 2 B instances
    DataContainer<NavNodeCPtr> instanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get())).get(); }
        );
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_GroupedByLabel_Groups4InstancesWith2GroupingNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByLabel_Groups4InstancesWith2GroupingNodes)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", Utf8PrintfString("%s,%s", classB->GetFullName(), classA->GetName().c_str()), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> labelGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, labelGroupingNodes.GetSize());

    // make sure we have 2 A instances in A grouping node
    NavNodeCPtr labelGroupingNode = labelGroupingNodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("InstanceA", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());
    DataContainer<NavNodeCPtr> instanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get())).get(); }
        );
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());

    // make sure we have 2 B instances in B grouping node
    labelGroupingNode = labelGroupingNodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, labelGroupingNode->GetType().c_str());
    ASSERT_STREQ("InstanceB", labelGroupingNode->GetLabelDefinition().GetDisplayValue().c_str());
    instanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), labelGroupingNode.get())).get(); }
        );
    ASSERT_EQ(2, instanceNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[0]->GetType().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_GroupedByClassAndByLabel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByClassAndByLabel)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some B instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("Property", ECValue("InstanceB1")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("Property", ECValue("InstanceB1")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("Property", ECValue("InstanceB2")); });

    // insert A instance
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"B\"", 1, "this.Property", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, true, false, "", Utf8PrintfString("%s,%s", classB->GetFullName(), classA->GetName().c_str()), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 1 A instance node
    DataContainer<NavNodeCPtr> instanceANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, instanceANodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceANodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), instanceANodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 B nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> instanceBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get())).get(); }
        );
    ASSERT_EQ(2, instanceBNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, instanceBNodes[0]->GetType().c_str());

    // make sure we have 2 InstanceB1 instance nodes
    DataContainer<NavNodeCPtr> instanceB1 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), instanceBNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), instanceBNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, instanceB1.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceB1[0]->GetType().c_str());
    EXPECT_STREQ("InstanceB1", instanceB1[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 InstanceB2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceBNodes[1]->GetType().c_str());
    EXPECT_STREQ("InstanceB2", instanceBNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_GroupedByClassAndByLabel_InstanceLabelOverride, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_GroupedByClassAndByLabel_InstanceLabelOverride)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some B instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("Property", ECValue("InstanceB1")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("Property", ECValue("InstanceB1")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("Property", ECValue("InstanceB2")); });

    // insert A instance
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, true, false, "", Utf8PrintfString("%s,%s", classB->GetFullName(), classA->GetName().c_str()), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 1 A instance node
    DataContainer<NavNodeCPtr> instanceANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, instanceANodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceANodes[0]->GetType().c_str());
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, instanceANodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 B nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> instanceBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get())).get(); }
        );
    ASSERT_EQ(2, instanceBNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, instanceBNodes[0]->GetType().c_str());

    // make sure we have 2 InstanceB1 instance nodes
    DataContainer<NavNodeCPtr> instanceB1Nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), instanceBNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), instanceBNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, instanceB1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceB1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("InstanceB1", instanceB1Nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 InstanceB2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceBNodes[1]->GetType().c_str());
    EXPECT_STREQ("InstanceB2", instanceBNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_DoNotSort_ReturnsUnsortedNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_DoNotSort_ReturnsUnsortedNodes)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    instanceNodesOfSpecificClassesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Instance2", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Instance1", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_ArePolymorphic, R"*(
    <ECEntityClass typeName="Base" />
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_ArePolymorphic)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    // insert some base and derived instances
    IECInstancePtr baseClassInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass);
    IECInstancePtr derivedClassInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", baseClass->GetFullName(), true);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_AreNotPolymorphic, R"*(
    <ECEntityClass typeName="Base" />
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_AreNotPolymorphic)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    // insert some base and derived instances
    IECInstancePtr baseClassInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass);
    IECInstancePtr derivedClassInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", baseClass->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_InstanceFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_InstanceFilter)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("Property", ECValue(10)); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("Property", ECValue(5)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "this.Property<=5", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* Note: In this test we initialize data providers for the first 2 specs when determining
* root node's children. The first spec returns 0 nodes and the second one returns 1 node.
* Overall the root node should have 2 children - 2'nd and 3'rd.
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_ReturnsCorrectChildNodesWhenOneOfThemIsHiddenAndCombinedHierarchyLevelIsNotInitialized, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_ReturnsCorrectChildNodesWhenOneOfThemIsHiddenAndCombinedHierarchyLevelIsNotInitialized)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"root\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false,
        true, false, false, false, "", classA->GetFullName(), false));
    childRule->AddSpecification(*CreateCustomNodeSpecification("child2"));
    childRule->AddSpecification(*CreateCustomNodeSpecification("child3"));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("child2", childNodes[0]->GetType().c_str());
    EXPECT_STREQ("child3", childNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Note: In this test we initialize data providers for the first spec when determining
* root node's children. The first spec returns its child nodes and the second one returns
* 1 node. Overall the root node should have 2 children - 2'nd and 3'rd.
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_ReturnsCorrectChildNodesWhenOneOfThemReturnsItsChildrenAndCombinedHierarchyLevelIsNotInitialized, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_ReturnsCorrectChildNodesWhenOneOfThemReturnsItsChildrenAndCombinedHierarchyLevelIsNotInitialized)
    {
    ECClassCP classA = GetClass("A");

    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.Type = \"root\"", 1, false, TargetTree_Both);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true,
        false, false, false, false, "", classA->GetFullName(), false));
    childRule1->AddSpecification(*CreateCustomNodeSpecification("child3"));
    rules->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.IsInstanceNode", 1, false, TargetTree_Both);
    childRule2->AddSpecification(*CreateCustomNodeSpecification("child2"));
    rules->AddPresentationRule(*childRule2);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("child2", childNodes[0]->GetType().c_str());
    EXPECT_STREQ("child3", childNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_AlwaysReturnsChildren, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_AlwaysReturnsChildren)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert instances with relationship
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, true, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Forward, GetSchema()->GetName(), relationshipAHasB->GetFullName(), classB->GetFullName());
    childRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 B node
    ASSERT_EQ(1, classANodes.GetSize());

    NavNodeCPtr classANode = classANodes[0];
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get())).get(); }
        );

    // make sure we have 1 A node
    ASSERT_EQ(1, classBNodes.GetSize());
    NavNodeCPtr node = classBNodes[0];
    EXPECT_EQ(ChildrenHint::Always, NavNodeExtendedData(*node).GetChildrenHint());
    EXPECT_TRUE(node->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_HideNodesInHierarchy, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_HideNodesInHierarchy)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert instances with relationship
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, true, false, false, false, false, false, 0, "", RequiredRelationDirection_Forward, GetSchema()->GetName(), relationshipAHasB->GetFullName(), classB->GetFullName());
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    ChildNodeRule* customNodeRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    customNodeRule->AddSpecification(*customNodeSpecification);
    relatedInstanceNodesSpecification->AddNestedRule(*customNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

     // make sure we have 1 A node
    ASSERT_EQ(1, classANodes.GetSize());

    NavNodeCPtr classANode = classANodes[0];
    DataContainer<NavNodeCPtr> customNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get())).get(); }
        );

    // make sure we have 1 custom node
    ASSERT_EQ(1, customNodes.GetSize());

    NavNodeCPtr instanceNode = customNodes[0];
    ASSERT_STREQ("test", instanceNode->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert instances with relationship
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, true, false, false, false, false, 0, "", RequiredRelationDirection_Forward, GetSchema()->GetName(), relationshipAHasB->GetFullName(), classB->GetFullName());
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 A node
    ASSERT_EQ(1, classANodes.GetSize());

    NavNodeCPtr classANode = classANodes[0];
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get())).get(); }
        );

    // make sure we have 0 B nodes
    ASSERT_EQ(0, classBNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert instances with relationship
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, true, false, false, false, false, 0, "", RequiredRelationDirection_Forward, GetSchema()->GetName(), relationshipAHasB->GetFullName(), classB->GetFullName());
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    ChildNodeRule* customNodeRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    customNodeRule->AddSpecification(*customNodeSpecification);
    relatedInstanceNodesSpecification->AddNestedRule(*customNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 A node
    ASSERT_EQ(1, classANodes.GetSize());
    NavNodeCPtr classANode = classANodes[0];
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get())).get(); }
        );

    // make sure we have 1 B node
    ASSERT_EQ(1, classBNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_GroupedByClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert instances with relationships
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0, "", RequiredRelationDirection_Forward, GetSchema()->GetName(), relationshipAHasB->GetFullName(), classB->GetFullName());
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 A node
    ASSERT_EQ(1, classANodes.GetSize());
    NavNodeCPtr classANode = classANodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get())).get(); }
        );

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_ExcludedClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E">
        <BaseClass>D</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClasses_ExcludedClasses)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECClassCP classE = GetClass("E");

    //expected returned instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    //expected filtered out instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classA->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classA->GetName().c_str() })},
        bvector<MultiSchemaClass*> {
            new MultiSchemaClass(classB->GetSchema().GetName().c_str(), false, bvector<Utf8String> { classB->GetName().c_str() }),
            new MultiSchemaClass(classD->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classD->GetName().c_str() })
        }));
    rules->AddPresentationRule(*rootRule);

    // verify
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); });

    ASSERT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], { a });
    VerifyNodeInstances(*rootNodes[1], { c });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *d);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false,
        "", { new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)) }));
    rules->AddPresentationRule(*childRule);

    // verify
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(3, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    EXPECT_STREQ("B", classGroupingNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[1]->GetType().c_str());
    EXPECT_STREQ("C", classGroupingNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[2]->GetType().c_str());
    EXPECT_STREQ("D", classGroupingNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childNodes0 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes0.GetSize());
    VerifyNodeInstance(*childNodes0[0], *b);

    DataContainer<NavNodeCPtr> childNodes1 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get())).get(); }
        );
    ASSERT_EQ(1, childNodes1.GetSize());
    VerifyNodeInstance(*childNodes1[0], *c);

    DataContainer<NavNodeCPtr> childNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[2].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[2].get())).get(); }
        );
    ASSERT_EQ(1, childNodes2.GetSize());
    VerifyNodeInstance(*childNodes2[0], *d);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childrenRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, true, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", rel->GetFullName(), classA->GetFullName()));
    rules->AddPresentationRule(*childrenRule);

    // request & validate
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(3, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b);
    VerifyNodeInstance(*rootNodes[1], *c);
    VerifyNodeInstance(*rootNodes[2], *d);

    DataContainer<NavNodeCPtr> classGroupedChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classGroupedChildNodes.GetSize());
    EXPECT_STREQ("C", classGroupedChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupedChildNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupedChildNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childInstanceNodes.GetSize());
    VerifyNodeInstance(*childInstanceNodes[0], *c);

    DataContainer<NavNodeCPtr> classGroupedGrandChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childInstanceNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childInstanceNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classGroupedGrandChildNodes.GetSize());
    EXPECT_STREQ("D", classGroupedGrandChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> grandChildInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupedGrandChildNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupedGrandChildNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, grandChildInstanceNodes.GetSize());
    VerifyNodeInstance(*grandChildInstanceNodes[0], *d);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(3, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b);
    VerifyNodeInstance(*rootNodes[1], *c);
    VerifyNodeInstance(*rootNodes[2], *d);

    DataContainer<NavNodeCPtr> classGroupedChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classGroupedChildNodes.GetSize());
    EXPECT_STREQ("C", classGroupedChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupedChildNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupedChildNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childInstanceNodes.GetSize());
    VerifyNodeInstance(*childInstanceNodes[0], *c);

    DataContainer<NavNodeCPtr> classGroupedGrandChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childInstanceNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childInstanceNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classGroupedGrandChildNodes.GetSize());
    EXPECT_STREQ("D", classGroupedGrandChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> grandChildInstanceNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupedGrandChildNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupedGrandChildNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, grandChildInstanceNodes.GetSize());
    VerifyNodeInstance(*grandChildInstanceNodes[0], *d);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_GroupedByLabel_DoesntGroup1Instance, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByLabel_DoesntGroup1Instance)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert instances with relationships
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, true, false, 0, "", RequiredRelationDirection_Backward, GetSchema()->GetName(), relationshipAHasB->GetFullName(), classA->GetFullName());
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 B node
    ASSERT_EQ(1, classBNodes.GetSize());
    ASSERT_STREQ("InstanceB", classBNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr classBNode = classBNodes[0];
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get())).get(); }
        );

    // make sure we have 1 A node
    ASSERT_EQ(1, classANodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classANodes[0]->GetType().c_str());
    ASSERT_STREQ("InstanceA", classANodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is owned by" polymorphic="False">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByLabel_Groups3InstancesWith1GroupingNode)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_Has_C")->GetRelationshipClassCP();

    // insert instances with relationships
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);

    IECInstancePtr classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);
    classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, true, false, 0, "", RequiredRelationDirection_Both, GetSchema()->GetName(), Utf8PrintfString("%s,%s", relationshipAHasB->GetFullName(), relationshipBHasC->GetName().c_str()), Utf8PrintfString("%s,%s", classA->GetFullName(), classC->GetName().c_str()));
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 B node
    ASSERT_EQ(1, classBNodes.GetSize());
    ASSERT_STREQ(GetDefaultDisplayLabel(*classBInstance).c_str(), classBNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr classBNode = classBNodes[0];
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 A node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());

    // make sure we have 2 C nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[1]->GetType().c_str());
    DataContainer<NavNodeCPtr> classCNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get())).get(); }
        );
    ASSERT_EQ(2, classCNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_DoNotSort_ReturnsUnsortedNodes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is owned by" polymorphic="False">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_DoNotSort_ReturnsUnsortedNodes)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_Has_C")->GetRelationshipClassCP();

    // insert some instances with relationships
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);
    classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, GetSchema()->GetName(), Utf8PrintfString("%s,%s", relationshipAHasB->GetFullName(), relationshipBHasC->GetName().c_str()), Utf8PrintfString("%s,%s", classA->GetFullName(), classC->GetName().c_str()));
    relatedInstanceNodesSpecification->SetDoNotSort(true);
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 B node
    ASSERT_EQ(1, classBNodes.GetSize());
    ASSERT_STREQ(GetDefaultDisplayLabel(*classBInstance).c_str(), classBNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr classBNode = classBNodes[0];
    DataContainer<NavNodeCPtr> classCNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get())).get(); }
        );

    // make sure we have 2 C nodes
    ASSERT_EQ(2, classCNodes.GetSize());

    ASSERT_STREQ("InstanceC2", classCNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("InstanceC1", classCNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *element);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *e4);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(4, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c1);
    VerifyNodeInstance(*childNodes[1], *c2);
    VerifyNodeInstance(*childNodes[2], *c3);
    VerifyNodeInstance(*childNodes[3], *c4);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(4, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c1);
    VerifyNodeInstance(*childNodes[1], *c2);
    VerifyNodeInstance(*childNodes[2], *c3);
    VerifyNodeInstance(*childNodes[3], *c4);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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

    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(4, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c1);
    VerifyNodeInstance(*childNodes[1], *c2);
    VerifyNodeInstance(*childNodes[2], *c3);
    VerifyNodeInstance(*childNodes[3], *c4);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_SkipRelatedLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is owned by" polymorphic="False">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_SkipRelatedLevel)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_Has_C")->GetRelationshipClassCP();

    // insert instances with relationships
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);

    IECInstancePtr classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1, "", RequiredRelationDirection_Both, GetSchema()->GetName(), relationshipBHasC->GetFullName(), classC->GetFullName());
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

     // make sure we have 1 A node
    ASSERT_EQ(1, classANodes.GetSize());
    NavNodeCPtr classANode = classANodes[0];
    DataContainer<NavNodeCPtr> classCNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get())).get(); }
        );

    // make sure we have 1 C node
    ASSERT_EQ(1, classCNodes.GetSize());
    NavNodeCPtr classCNode = classCNodes[0];
    ASSERT_STREQ(GetDefaultDisplayLabel(*classCInstance).c_str(), classCNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#711486
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_SkipRelatedLevel_DoesntDuplicateNodesWhenSkippingMultipleDifferentInstancesWithTheSameEndpointInstance, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_Many_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="Many_A_Has_Many_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_SkipRelatedLevel_DoesntDuplicateNodesWhenSkippingMultipleDifferentInstancesWithTheSameEndpointInstance)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasManyB = GetClass("A_Has_Many_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipManyAHasManyB = GetClass("Many_A_Has_Many_B")->GetRelationshipClassCP();

    // insert instances with relationships
    IECInstancePtr classAInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(1));});
    IECInstancePtr classAInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr classBInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasManyB, *classAInstance1, *classBInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasManyB, *classAInstance1, *classBInstance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipManyAHasManyB, *classAInstance2, *classBInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipManyAHasManyB, *classAInstance2, *classBInstance2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Property = 1", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childrenRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"A\", \"%s\")", GetSchema()->GetName().c_str()), 1, false, TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1,
        "", RequiredRelationDirection_Both, GetSchema()->GetName(), relationshipManyAHasManyB->GetFullName(), classA->GetFullName()));
    rules->AddPresentationRule(*childrenRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

     // make sure we have 1 A root node
    ASSERT_EQ(1, rootNodes.GetSize());
    NavNodeCPtr classAInstance1Node = rootNodes[0];
    ECInstanceId classAInstance1Id;
    ECInstanceId::FromString(classAInstance1Id, classAInstance1->GetInstanceId().c_str());
    VerifyNodeInstance(*classAInstance1Node, *classAInstance1);

    // make sure we have 1 A child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classAInstance1Node.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classAInstance1Node.get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    NavNodeCPtr classAInstance2Node = childNodes[0];
    ECInstanceId classAInstance2Id;
    ECInstanceId::FromString(classAInstance2Id, classAInstance2->GetInstanceId().c_str());
    VerifyNodeInstance(*classAInstance2Node, *classAInstance2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodes_DoesntDuplicateNodesWhenSkippingDifferentInstancesWithTheSameEndpointInstance, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodes_DoesntDuplicateNodesWhenSkippingDifferentInstancesWithTheSameEndpointInstance)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAC = GetClass("A_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAC, *a, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAC, *a, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childrenRule = new ChildNodeRule(Utf8PrintfString("ThisNode.ClassName = \"%s\"", classA->GetName().c_str()), 1, false, TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(relationshipAC->GetFullName(), RequiredRelationDirection_Forward),
            new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Backward),
            }),
        }));
    rules->AddPresentationRule(*childrenRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 A root node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    // make sure we have 1 B child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *b);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodes_DoesntDuplicateNodesWhenSkippingDifferentInstancesWithTheSameEndpointInstance_GroupedByClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_ReturnsMultiRelationshipRecursivelyRelatedInstanceNodes_DoesntDuplicateNodesWhenSkippingDifferentInstancesWithTheSameEndpointInstance_GroupedByClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAC = GetClass("A_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAC, *a, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAC, *a, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childrenRule = new ChildNodeRule(Utf8PrintfString("ThisNode.ClassName = \"%s\"", classA->GetName().c_str()), 1, false, TargetTree_Both);
    childrenRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(relationshipAC->GetFullName(), RequiredRelationDirection_Forward),
            new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Backward),
            }),
        }));
    rules->AddPresentationRule(*childrenRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 A root node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    // make sure we have 1 B child grouping node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_EQ(classB, &childNodes[0]->GetKey()->AsECClassGroupingNodeKey()->GetECClass());

    // make sure we have 1 B grand child node
    DataContainer<NavNodeCPtr> grandChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, grandChildNodes.GetSize());
    VerifyNodeInstance(*grandChildNodes[0], *b);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_InstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_InstanceFilter)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert some instances with relationships
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB123"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);
    classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "this.Property~\"InstanceB1\"", RequiredRelationDirection_Both, GetSchema()->GetName(), relationshipAHasB->GetFullName(), classB->GetFullName());
    relatedInstanceNodesSpecification->SetDoNotSort(true);
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 B node
    ASSERT_EQ(1, classANodes.GetSize());
    ASSERT_STREQ(GetDefaultDisplayLabel(*classAInstance).c_str(), classANodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    NavNodeCPtr classANode = classANodes[0];
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANode.get())).get(); }
        );

    // make sure we have 1 B node
    ASSERT_EQ(1, classBNodes.GetSize());
    ASSERT_STREQ("InstanceB1", classBNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_InstanceFilter_AppliesOnlyToMatchingInstances, R"*(
    <ECEntityClass typeName="ClassA">
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="Name" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="None">
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
    ECRelationshipClassCP relationship = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert some instances with relationships
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Name", ECValue("One")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationship, *instanceA1, *instanceB1);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Name", ECValue("Two")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationship, *instanceA2, *instanceB2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule("ParentNode.ClassName = \"ClassA\"", 1000, false, TargetTree_Both);
    relatedNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "this.Name = \"One\" OR this.Name = \"Two\"", RequiredRelationDirection_Both, "", relationship->GetFullName(), classB->GetFullName()));
    rules->AddPresentationRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_GroupedByLabel_GroupsByClassAndByLabel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is owned by" polymorphic="False">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByLabel_GroupsByClassAndByLabel)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_Has_C")->GetRelationshipClassCP();

    // insert instances with relationships
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);
    IECInstancePtr classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);
    classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);
    classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"C\"", 1, "this.Property", ""));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, true, false, true, false, 0, "", RequiredRelationDirection_Both, GetSchema()->GetName(), Utf8PrintfString("%s,%s", relationshipAHasB->GetFullName(), relationshipBHasC->GetName().c_str()), Utf8PrintfString("%s,%s", classA->GetFullName(), classC->GetName().c_str()));
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 B node
    ASSERT_EQ(1, classBNodes.GetSize());
    NavNodeCPtr classBNode = classBNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get())).get(); }
        );

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 2 C nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> classCNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get())).get(); }
        );
    ASSERT_EQ(2, classCNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, classCNodes[0]->GetType().c_str());

    // make sure we have 2 InstanceC1 instance nodes
    DataContainer<NavNodeCPtr> classCInstance1Nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classCNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classCNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, classCInstance1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classCInstance1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("InstanceC1", classCInstance1Nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 InstanceC2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classCNodes[1]->GetType().c_str());
    EXPECT_STREQ("InstanceC2", classCNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 A instance node
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classANodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classANodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*classAInstance).c_str(), classANodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_RelatedInstanceNodes_GroupedByLabel_GroupsByClassAndByLabel_InstanceLabelOverride, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is owned by" polymorphic="False">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_RelatedInstanceNodes_GroupedByLabel_GroupsByClassAndByLabel_InstanceLabelOverride)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_Has_C")->GetRelationshipClassCP();

    // insert instances with relationships
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);
    IECInstancePtr classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);
    classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC1"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);
    classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceC2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *classBInstance, *classCInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, true, false, true, false, 0, "", RequiredRelationDirection_Both, GetSchema()->GetName(), Utf8PrintfString("%s,%s", relationshipAHasB->GetFullName(), relationshipBHasC->GetName().c_str()), Utf8PrintfString("%s,%s", classA->GetFullName(), classC->GetName().c_str()));
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    instanceNodesOfSpecificClassesSpecification->AddNestedRule(*relatedNodeRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 B node
    ASSERT_EQ(1, classBNodes.GetSize());
    NavNodeCPtr classBNode = classBNodes[0];
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBNode.get())).get(); }
        );

    // make sure we have 2 class grouping nodes
    ASSERT_EQ(2, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());

    // make sure we have 1 A instance node
    DataContainer<NavNodeCPtr> instanceANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, instanceANodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceANodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*classAInstance).c_str(), instanceANodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 C nodes (one grouping node and one instance node)
    DataContainer<NavNodeCPtr> classCNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classGroupingNodes[1].get())).get(); }
        );
    ASSERT_EQ(2, classCNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, classCNodes[0]->GetType().c_str());

    // make sure we have 2 InstanceC1 instance nodes
    DataContainer<NavNodeCPtr> instanceC1Nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classCNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classCNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, instanceC1Nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceC1Nodes[0]->GetType().c_str());
    EXPECT_STREQ("InstanceC1", instanceC1Nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 InstanceC2 instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classCNodes[1]->GetType().c_str());
    EXPECT_STREQ("InstanceC2", classCNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    ASSERT_STREQ("test label", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }


/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", modelClass->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Model\"", 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "",
        modelContainsElementsRel->GetFullName(), Utf8PrintfString("%s;E:%s", elementClass->GetFullName(), geometricElementClass->GetFullName())));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *model);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(3, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *baseElement);
    VerifyNodeInstance(*childNodes[1], *customElement);
    VerifyNodeInstance(*childNodes[2], *physicalElement);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", modelClass->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"Model\"", 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0, "", RequiredRelationDirection_Forward, "",
        modelContainsElementsRel->GetFullName(), Utf8PrintfString("%s;PE:%s", elementClass->GetFullName(), geometricElementClass->GetFullName())));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *model);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *baseElement);
    VerifyNodeInstance(*childNodes[1], *customElement);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstances_HideIfNoChildren_ReturnsNodesIfHasChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_HideIfNoChildren_ReturnsNodesIfHasChildren)
    {
    // insert instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, true, false, false);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s.A", GetSchema()->GetName().c_str()), GetSchema()->GetName(), "A"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "test", "test", "test", "test");
    childRule->AddSpecification(*customNodeSpecification);
    searchResultInstanceSpecification->AddNestedRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 instance node
    ASSERT_EQ(1, classANodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instance).c_str(), classANodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 custom node
    DataContainer<NavNodeCPtr> customNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classANodes[0].get())).get(); }
        );
    ASSERT_EQ(1, customNodes.GetSize());
    EXPECT_STREQ("test", customNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstances_HideIfNoChildren_ReturnsEmptyListIfNoChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_HideIfNoChildren_ReturnsEmptyListIfNoChildren)
    {
    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, true, false, false);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s.A", GetSchema()->GetName().c_str()), GetSchema()->GetName(), "A"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstances_GroupedByClass, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByClass)
    {
    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, true, false);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s.A", GetSchema()->GetName().c_str()), GetSchema()->GetName(), "A"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> classGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, classGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classGroupingNodes[0]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstances_GroupedByLabel_DoesntGroup1Instance, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByLabel_DoesntGroup1Instance)
    {
    // insert instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, true);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s.A", GetSchema()->GetName().c_str()), GetSchema()->GetName(), "A"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 instance node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstances_GroupedByLabel_Groups3InstancesWith1GroupingNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByLabel_Groups3InstancesWith1GroupingNode)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, true);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s.B", GetSchema()->GetName().c_str()), GetSchema()->GetName(), "B"));
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s.A", GetSchema()->GetName().c_str()), GetSchema()->GetName(), "A"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 A instance node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("InstanceA", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 B instance nodes with label grouping node
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[1]->GetType().c_str());
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get())).get(); }
        );
    ASSERT_EQ(2, classBNodes.GetSize());
    ASSERT_STREQ("InstanceB", classBNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("InstanceB", classBNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstances_GroupedByLabel_Groups4InstancesWith2GroupingNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_GroupedByLabel_Groups4InstancesWith2GroupingNodes)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, true);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s.B", GetSchema()->GetName().c_str()), GetSchema()->GetName(), "B"));
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s.A", GetSchema()->GetName().c_str()), GetSchema()->GetName(), "A"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 label grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 2 A instance nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[0]->GetType().c_str());
    DataContainer<NavNodeCPtr> classANodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_STREQ("InstanceA", classANodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("InstanceA", classANodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 B instance nodes
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, nodes[1]->GetType().c_str());
    DataContainer<NavNodeCPtr> classBNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get())).get(); }
        );
    ASSERT_EQ(2, classBNodes.GetSize());
    ASSERT_STREQ("InstanceB", classBNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("InstanceB", classBNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstances_DoNotSort_ReturnsUnsortedNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SearchResultInstances_DoNotSort_ReturnsUnsortedNodes)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    SearchResultInstanceNodesSpecificationP searchResultInstanceSpecification = new SearchResultInstanceNodesSpecification(1, false, false, false, false, false);
    searchResultInstanceSpecification->SetDoNotSort(true);
    searchResultInstanceSpecification->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s.A", GetSchema()->GetName().c_str()), GetSchema()->GetName(), "A"));
    rule->AddSpecification(*searchResultInstanceSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("InstanceA2", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("InstanceA1", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ImageIdOverride)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    ImageIdOverrideP imageIdOverride = new ImageIdOverride("ThisNode.Type=\"customType\"", 1, "\"overridedImageId\"");
    rules->AddPresentationRule(*imageIdOverride);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node with overriden ImageId
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedImageId", nodes[0]->GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesRelatedInstanceInImageIdOverrideCondition, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, UsesRelatedInstanceInImageIdOverrideCondition)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *classAInstance, *classBInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ImageIdOverrideP imageIdOverride = new ImageIdOverride("ThisNode.ClassName = \"A\" ANDALSO classBAlias.Property = \"InstanceB\"", 1, "\"overridedImageId\"");
    rules->AddPresentationRule(*imageIdOverride);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    instanceNodesOfSpecificClassesSpecification->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relationshipAHasB->GetFullName(), classB->GetFullName(), "classBAlias"));
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node with overriden ImageId
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedImageId", nodes[0]->GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LabelOverride)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.Type=\"customType\"", 1, "\"overridedLabel\"", "\"overridedDescription\"");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node with overrided Label & Description
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedLabel", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("overridedDescription", nodes[0]->GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LabelOverrideWithGroupedInstancesCountOnClassGroupingNode, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LabelOverrideWithGroupedInstancesCountOnClassGroupingNode)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", Utf8PrintfString("%s,%s", classB->GetFullName(), classA->GetName().c_str()), true);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsClassGroupingNode", 1, "\"Count: \" & ThisNode.GroupedInstancesCount", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // verify there're two grouping nodes with correct labels
    ASSERT_EQ(2, nodes.GetSize());
    ASSERT_STREQ("Count: 2", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 3", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNode)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    ECInstanceInserter inserter(s_project->GetECDb(), *classA, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(10));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "Property");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    rules->AddPresentationRule(*groupingRule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsPropertyGroupingNode", 1, "\"Count: \" & ThisNode.GroupedInstancesCount", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // verify there're three grouping nodes with correct labels
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Count: 1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 2", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 3", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNodeSortedByPropertyValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, LabelOverrideWithGroupedInstancesCountOnPropertyGroupingNodeSortedByPropertyValue)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    ECInstanceInserter inserter(s_project->GetECDb(), *classA, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(10));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true);
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "Property");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingSpec->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    rules->AddPresentationRule(*groupingRule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.IsPropertyGroupingNode", 1, "\"Count: \" & ThisNode.GroupedInstancesCount", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // verify there're three grouping nodes with correct labels
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Count: 2", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 3", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Count: 1", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, StyleOverride)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "customType", "label", "description", "imageId");

    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    StyleOverrideP styleOverride = new StyleOverride("ThisNode.Type=\"customType\"", 1, "\"overridedForeColor\"", "\"overridedBackColor\"", "\"overridedFontStyle\"");
    rules->AddPresentationRule(*styleOverride);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node with overrided BackColor, ForeColor & FontStyle
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("overridedBackColor", nodes[0]->GetBackColor().c_str());
    ASSERT_STREQ("overridedForeColor", nodes[0]->GetForeColor().c_str());
    ASSERT_STREQ("overridedFontStyle", nodes[0]->GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CheckBoxRule_UsesDefaultValueIfPropertyIsNull, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
        <ECProperty propertyName="UnsetProperty" typeName="bool" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CheckBoxRule_UsesDefaultValueIfPropertyIsNull)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"InstanceB\"", 1, false, "UnsetProperty", false, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 A node
    ASSERT_STREQ("InstanceA", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 B node with CheckBoxRule
    ASSERT_STREQ("InstanceB", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(nodes[1]->IsCheckboxVisible());
    ASSERT_TRUE(nodes[1]->IsCheckboxEnabled());
    ASSERT_FALSE(nodes[1]->IsChecked());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CheckBoxRule_WithoutProperty)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    CustomNodeSpecificationP customNodeSpecification = new CustomNodeSpecification(1, false, "type", "customLabel", "description", "imageId");
    rule->AddSpecification(*customNodeSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"customLabel\"", 1, false, "", false, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ("customLabel", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(nodes[0]->IsCheckboxVisible());
    ASSERT_TRUE(nodes[0]->IsCheckboxEnabled());
    ASSERT_FALSE(nodes[0]->IsChecked());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CheckBoxRule_UsesInversedPropertyName, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Identifier" typeName="string" />
        <ECProperty propertyName="Checkbox" typeName="bool" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Identifier" typeName="string" />
        <ECProperty propertyName="Checkbox" typeName="bool" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CheckBoxRule_UsesInversedPropertyName)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){
        instance.SetValue("Checkbox", ECValue(false));
        instance.SetValue("Identifier", ECValue("InstanceB"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){
        instance.SetValue("Checkbox", ECValue(false));
        instance.SetValue("Identifier", ECValue("InstanceA"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Identifier"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Identifier"));
    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"InstanceB\"", 1, false, "Checkbox", true, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 A node
    ASSERT_STREQ("InstanceA", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 B node with CheckBoxRule
    ASSERT_STREQ("InstanceB", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(nodes[1]->IsCheckboxVisible());
    ASSERT_TRUE(nodes[1]->IsCheckboxEnabled());
    ASSERT_TRUE(nodes[1]->IsChecked());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CheckBoxRule_DoesNotUseInversedPropertyName, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Identifier" typeName="string" />
        <ECProperty propertyName="Checkbox" typeName="bool" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Identifier" typeName="string" />
        <ECProperty propertyName="Checkbox" typeName="bool" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CheckBoxRule_DoesNotUseInversedPropertyName)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){
        instance.SetValue("Checkbox", ECValue(false));
        instance.SetValue("Identifier", ECValue("InstanceB"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){
        instance.SetValue("Checkbox", ECValue(false));
        instance.SetValue("Identifier", ECValue("InstanceA"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Identifier"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Identifier"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("ThisNode.Label=\"InstanceB\"", 1, false, "Checkbox", false, false, "");
    rules->AddPresentationRule(*checkBoxRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );;

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());

    // make sure we have 1 A node
    ASSERT_STREQ("InstanceA", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_FALSE(nodes[0]->IsCheckboxVisible());

    // make sure we have 1 B node with CheckBoxRule
    ASSERT_STREQ("InstanceB", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(nodes[1]->IsCheckboxVisible());
    ASSERT_TRUE(nodes[1]->IsCheckboxEnabled());
    ASSERT_FALSE(nodes[1]->IsChecked());
    }
/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortingAscending_LabelOverride, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingAscending_LabelOverride)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance3"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\"", 1, "this.Property", ""));

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, GetSchema()->GetName(), "A", "Property", true, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 3 nodes sorted ascending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Instance1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Instance2", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Instance3", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortingAscending, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingAscending)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance3"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, GetSchema()->GetName(), "A", "Property", true, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 3 nodes sorted ascending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Instance1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Instance2", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Instance3", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortingDescending, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingDescending)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("instance2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance3"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, GetSchema()->GetName(), "A", "Property", false, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 node sorted descending
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Instance3", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("instance2", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Instance1", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Hierarchy_SortingRule_DoNotSort, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Hierarchy_SortingRule_DoNotSort)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance3"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance1"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, GetSchema()->GetName(), "A", "MissingProperty", false, true, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 3 unsorted nodes
    ASSERT_EQ(3, nodes.GetSize());
    ASSERT_STREQ("Instance2", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Instance3", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_STREQ("Instance1", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_DisableDerivedClassesSorting, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_DisableDerivedClassesSorting)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntPropA", ECValue(3)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("IntPropA", ECValue(3)); instance.SetValue("IntPropB", ECValue(2)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("IntPropA", ECValue(1)); instance.SetValue("IntPropB", ECValue(1)); });
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("IntPropA", ECValue(2)); instance.SetValue("IntPropB", ECValue(3)); });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, classA->GetSchema().GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    SortingRuleP sortingRule1 = new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "IntPropA", true, false, true);
    rules->AddPresentationRule(*sortingRule1);
    SortingRuleP sortingRule2 = new SortingRule("", 2, classB->GetSchema().GetName(), classB->GetName(), "", false, true, false);
    rules->AddPresentationRule(*sortingRule2);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    ASSERT_EQ(6, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *a2);
    VerifyNodeInstance(*nodes[1], *a3);
    VerifyNodeInstance(*nodes[2], *a1);
    VerifyNodeInstance(*nodes[3], *b1);
    VerifyNodeInstance(*nodes[4], *b2);
    VerifyNodeInstance(*nodes[5], *b3);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_DisableSorting, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntProp1" typeName="int"/>
        <ECProperty propertyName="IntProp2" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_DisableSorting)
    {
    ECClassCP classA = GetClass("A");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(3)); instance.SetValue("IntProp2", ECValue(7)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(1)); instance.SetValue("IntProp2", ECValue(1)); });
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(2)); instance.SetValue("IntProp2", ECValue(5)); });
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(2)); instance.SetValue("IntProp2", ECValue(2)); });
    IECInstancePtr a5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(2)); instance.SetValue("IntProp2", ECValue(1)); });
    IECInstancePtr a6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(3)); instance.SetValue("IntProp2", ECValue(3)); });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, classA->GetSchema().GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);
    SortingRuleP sortingRule1 = new SortingRule("", 3, classA->GetSchema().GetName(), classA->GetName(), "IntProp1", true, false, false);
    rules->AddPresentationRule(*sortingRule1);
    SortingRuleP sortingRule2 = new SortingRule("", 2, classA->GetSchema().GetName(), classA->GetName(), "", false, true, false);
    rules->AddPresentationRule(*sortingRule2);
    SortingRuleP sortingRule3 = new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "IntProp2", true, false, false);
    rules->AddPresentationRule(*sortingRule3);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    ASSERT_EQ(6, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *a2);
    VerifyNodeInstance(*nodes[1], *a3);
    VerifyNodeInstance(*nodes[2], *a4);
    VerifyNodeInstance(*nodes[3], *a5);
    VerifyNodeInstance(*nodes[4], *a1);
    VerifyNodeInstance(*nodes[5], *a6);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortingAscendingPolymorphically, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingAscendingPolymorphically)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass, [](IECInstanceR instance) { instance.SetValue("Property", ECValue(2)); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass, [](IECInstanceR instance) { instance.SetValue("Property", ECValue(4)); });
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass, [](IECInstanceR instance) { instance.SetValue("Property", ECValue(1)); });
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass, [](IECInstanceR instance) { instance.SetValue("Property", ECValue(3)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, GetSchema()->GetName(), "Base", "Property", true, false, true);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 4 sorted ascending nodes
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortingByTwoProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="DoubleProperty" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingByTwoProperties)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("DoubleProperty", ECValue(1.0));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("DoubleProperty", ECValue(2.0));
        });
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("DoubleProperty", ECValue(4.0));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, GetSchema()->GetName(), "A", "IntProperty", true, false, false);
    rules->AddPresentationRule(*sortingRule);
    sortingRule = new SortingRule("", 1, GetSchema()->GetName(), "A", "DoubleProperty", false, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 3 nodes sorted ascending by IntProperty & descending by DoubleProperty
    ASSERT_EQ(3, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *instance3);
    VerifyNodeInstance(*nodes[1], *instance2);
    VerifyNodeInstance(*nodes[2], *instance1);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortingByEnumProperty, R"*(
    <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="True" description="" displayLabel="TestEnum">
        <ECEnumerator value="1" displayLabel="Z" />
        <ECEnumerator value="2" displayLabel="M" />
        <ECEnumerator value="3" displayLabel="A" />
    </ECEnumeration>
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="TestEnum" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortingRule_SortingByEnumProperty)
    {
    // insert some instances
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(1));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(3));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    SortingRuleP sortingRule = new SortingRule("", 1, GetSchema()->GetName(), "A", "Property", true, false, false);
    rules->AddPresentationRule(*sortingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes sorted correctly by enum display value
    ASSERT_EQ(2, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *instance2);
    VerifyNodeInstance(*nodes[1], *instance1);
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#176463
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", elementClass->GetFullName(), true));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, elementClass->GetFullName(), "MinX"));

    //make sure we have all nodes in correct order
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(instancesCount, rootNodes.GetSize());
    for (size_t i = 0; i < instancesCount; ++i)
        EXPECT_STREQ(std::to_string(i).c_str(), rootNodes[i]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsByBaseClass, R"*(
    <ECEntityClass typeName="Base" />
    <ECEntityClass typeName="Derived1">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Derived2">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByBaseClass)
    {
    // insert some instances
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("Derived1"));
    IECInstancePtr instanceG = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("Derived2"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "Base", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", false, GetSchema()->GetName(), "Base");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Base", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 2 Base child nodes
    DataContainer<NavNodeCPtr> classEChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(2, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classEChildNodes[0]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceF).c_str(), classEChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classEChildNodes[1]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceG).c_str(), classEChildNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsByDerivedClassSpecifiedInClassGroupSpecification, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByDerivedClassSpecifiedInClassGroupSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRuleP rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, classB->GetSchema().GetName(), classB->GetName()));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 3 nodes: 1 class grouping node and 2 instance nodes
    ASSERT_EQ(3, nodes.GetSize());
    VerifyClassGroupingNode(*nodes[0], "", { instanceB }, classB, true);
    VerifyNodeInstance(*nodes[1], *instanceA);
    VerifyNodeInstance(*nodes[2], *instanceC);

    // make sure the class grouping node has correct children
    DataContainer<NavNodeCPtr> bNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, bNodes.GetSize());
    VerifyNodeInstance(*bNodes[0], *instanceB);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsByBaseClassSpecifiedInClassGroupSpecification, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByBaseClassSpecifiedInClassGroupSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRuleP rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 3 nodes: 1 class grouping node and 2 instance nodes
    ASSERT_EQ(3, nodes.GetSize());
    VerifyClassGroupingNode(*nodes[0], "", { instanceB }, classA, true);
    VerifyNodeInstance(*nodes[1], *instanceA);
    VerifyNodeInstance(*nodes[2], *instanceC);

    // make sure the class grouping node has correct children
    DataContainer<NavNodeCPtr> bNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, bNodes.GetSize());
    VerifyNodeInstance(*bNodes[0], *instanceB);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsDerivedInstancesByBaseClassWhenThereAreInstancesOfDifferentDerivedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsDerivedInstancesByBaseClassWhenThereAreInstancesOfDifferentDerivedClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRuleP rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, "", ""));
    rules->AddPresentationRule(*groupingRule);

    // validate hierarchy
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, true, { instanceB }),
            {
            CreateInstanceNodeValidator({ instanceB }),
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_DoesNotCreateGroupForSingleItem, R"*(
    <ECEntityClass typeName="Base" />
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_DoesNotCreateGroupForSingleItem)
    {
    // insert instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("Derived"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "Base", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", false, GetSchema()->GetName(), "Base");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 Derived node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceF).c_str(), nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_CreatesGroupForSingleItem, R"*(
    <ECEntityClass typeName="Base" />
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_CreatesGroupForSingleItem)
    {
    // insert instance
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("Derived"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "Base", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", true, GetSchema()->GetName(), "Base");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Base", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 Derived node
    DataContainer<NavNodeCPtr> classFNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classFNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classFNodes[0]->GetType().c_str());
    ASSERT_STREQ(GetDefaultDisplayLabel(*instanceF).c_str(), classFNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_CreatesMultipleClassGroupsForSameInstancesWhenTheirClassHasMultipleBaseClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.1.0">
                <AppliesToEntityClass>A</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_CreatesMultipleClassGroupsForSameInstancesWhenTheirClassHasMultipleBaseClasses)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    auto instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    auto groupingRuleA = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRuleA->AddGroup(*new ClassGroup("", true, "", ""));
    rules->AddPresentationRule(*groupingRuleA);

    auto groupingRuleB = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    groupingRuleB->AddGroup(*new ClassGroup("", true, "", ""));
    rules->AddPresentationRule(*groupingRuleB);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 class grouping nodes - A and B
    ASSERT_EQ(2, nodes.GetSize());
    VerifyClassGroupingNode(*nodes[0], "", { instanceC }, classA, true);
    VerifyClassGroupingNode(*nodes[1], "", { instanceC }, classB, true);

    // make sure the class grouping nodes have correct children
    DataContainer<NavNodeCPtr> aNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, aNodes.GetSize());
    VerifyNodeInstance(*aNodes[0], *instanceC);

    DataContainer<NavNodeCPtr> bNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[1].get())).get(); }
        );
    ASSERT_EQ(1, bNodes.GetSize());
    VerifyNodeInstance(*bNodes[0], *instanceC);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_DoesNotCreateGroupForSingleItem)
    {
    // insert instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", false, "Property", "");
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 A node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *instance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_CreatesGroupForSingleItem, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_CreatesGroupForSingleItem)
    {
    // insert instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "Property");
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 A property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyGroupingNode(*nodes[0], "", { instance }, { ECValue() });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_SetImageIdForGroupingNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_SetImageIdForGroupingNode)
    {
    // insert instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "changedImageId", true, "Property", "");
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 instance property grouping node with changed ImageId
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyGroupingNode(*nodes[0], "", { instance }, { ECValue() });
    EXPECT_STREQ("changedImageId", nodes[0]->GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByNullProperty, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByNullProperty)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    // insert 2 instances
    IECInstancePtr baseInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass);
    IECInstancePtr derivedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", baseClass->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "Base", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Property", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyGroupingNode(*nodes[0], "", { baseInstance, derivedInstance }, { ECValue() });
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    EXPECT_EQ(2, children.GetSize());
    VerifyNodeInstance(*children[0], *baseInstance);
    VerifyNodeInstance(*children[1], *derivedInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsNullAndEmptyStringProperties, R"*(
    <ECEntityClass typeName="MyClass">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsNullAndEmptyStringProperties)
    {
    ECClassCP ecClass = GetClass("MyClass");

    // insert 2 instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(""));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", ecClass->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, ecClass->GetSchema().GetName(), ecClass->GetName(), "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "Prop", "");
    groupingSpec->SetCreateGroupForUnspecifiedValues(true);
    groupingRule->AddGroup(*groupingSpec);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyGroupingNode(*nodes[0], "", { instance1, instance2 }, { ECValue(), ECValue("") });
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    EXPECT_EQ(2, children.GetSize());
    VerifyNodeInstance(*children[0], *instance1);
    VerifyNodeInstance(*children[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsPolymorphically, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropertyB" typeName="int" />
        <ECNavigationProperty propertyName="RelatedA" relationshipName="AHasB" direction="Backward" />
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="AHasB" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A Has B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="A Has B" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsPolymorphically)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP AHasB = GetClass("AHasB")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr classAInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropertyA", ECValue("InstanceA"));});
    IECInstancePtr classBInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr classCInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *AHasB, *classAInstance, *classBInstance);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *AHasB, *classAInstance, *classCInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "PropertyA"));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "B", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "RelatedA", ""));
    rules->AddPresentationRule(*groupingRule);

    GroupingRuleP groupingRule2 = new GroupingRule("", 1, false, GetSchema()->GetName(), "C", "", "", "");
    groupingRule2->AddGroup(*new PropertyGroup("", "", true, "PropertyB", ""));
    rules->AddPresentationRule(*groupingRule2);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyGroupingNode(*nodes[0], "", { classBInstance, classCInstance }, { ECValue(RulesEngineTestHelpers::GetInstanceKey(*classAInstance).GetId()) });
    ASSERT_STREQ("InstanceA", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> bChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(2, bChildren.GetSize());
    VerifyPropertyGroupingNode(*bChildren[0], "", { classCInstance }, { ECValue() });
    VerifyNodeInstance(*bChildren[1], *classBInstance);

    // make sure the property grouping node has 1 child
    DataContainer<NavNodeCPtr> cChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bChildren[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bChildren[0].get())).get(); }
        );
    ASSERT_EQ(1, cChildren.GetSize());
    VerifyNodeInstance(*cChildren[0], *classCInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", ecClass->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, ecClass->GetSchema().GetName(), ecClass->GetName(), "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "MyProp", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyGroupingNode(*nodes[0], "", { instance1, instance2 }, { ECValue("My Value") });
    ASSERT_STREQ("My Value", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure the node has 2 children
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(2, children.GetSize());
    VerifyNodeInstance(*children[0], *instance1);
    VerifyNodeInstance(*children[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByIntegerEnumAsGroupingValue, R"*(
    <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="True" description="" displayLabel="TestEnum">
        <ECEnumerator value="1" displayLabel="Z" />
        <ECEnumerator value="2" displayLabel="M" />
        <ECEnumerator value="3" displayLabel="A" />
    </ECEnumeration>
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="TestEnum" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByIntegerEnumAsGroupingValue)
    {
    ECClassCP classA = GetClass("A");

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(3));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(1));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "Property", "");
    propertyGroup->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    propertyGroup->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 property grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    VerifyPropertyGroupingNode(*nodes[0], "", { instance1 }, { ECValue(3) });
    ASSERT_STREQ("A", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[1], "", { instance2 }, { ECValue(1) });
    ASSERT_STREQ("Z", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByStringEnumAsDisplayLabel, R"*(
    <ECEnumeration typeName="TestEnum" backingTypeName="string" isStrict="True" description="" displayLabel="TestEnum">
        <ECEnumerator value="One" displayLabel="3" />
        <ECEnumerator value="Two" displayLabel="2" />
        <ECEnumerator value="Three" displayLabel="1" />
    </ECEnumeration>
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="TestEnum" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByStringEnumAsDisplayLabel)
    {
    ECClassCP classA = GetClass("A");

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Three"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("One"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Property", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 property grouping nodes
    ASSERT_EQ(2, nodes.GetSize());

    VerifyPropertyGroupingNode(*nodes[0], "", { instance1 }, { ECValue("Three") });
    ASSERT_STREQ("1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[1], "", { instance2 }, { ECValue("One") });
    ASSERT_STREQ("3", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByNullForeignKey, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropertyB" typeName="int" />
        <ECNavigationProperty propertyName="RelatedA" relationshipName="AHasB" direction="Backward" />
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="AHasB" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A Has B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="A Has B" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByNullForeignKey)
    {
    ECClassCP classB = GetClass("B");

    // insert an instance with null foreign key to class A
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "B", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "RelatedA", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyGroupingNode(*nodes[0], "", { instance }, { ECValue() });
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByRelationshipProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <ECProperty propertyName="Priority" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByRelationshipProperty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_Has_B")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instanceA, *instanceB, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, GetSchema()->GetName(), rel->GetFullName(), classB->GetFullName()));
    rules->AddPresentationRule(*childNodeRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A_Has_B", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Priority", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // expect 1 A node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceA);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyPropertyGroupingNode(*childNodes[0], "", { instanceB }, { ECValue(5) });
    ASSERT_STREQ("5", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByRelationshipNavigationProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="A has B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="A belongs to B" polymorphic="False">
            <Class class="B" />
        </Target>
        <ECNavigationProperty propertyName="InstanceC" relationshipName="ABRelationship_Has_C" direction="Backward" />
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ABRelationship_Has_C" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="C belongs to AB relationship" polymorphic="False">
            <Class class="C" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="AB relationship has C" polymorphic="False">
            <Class class="A_Has_B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByRelationshipNavigationProperty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP rel_ab = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_abc = GetClass("ABRelationship_Has_C")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(1));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue(2));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue(3));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_ab, *instanceA, *instanceB, [&](IECInstanceR instance)
        {
        instance.SetValue("InstanceC", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceC).GetId(), rel_abc));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, GetSchema()->GetName(), rel_ab->GetFullName(), classB->GetFullName()));
    rules->AddPresentationRule(*childNodeRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A_Has_B", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "InstanceC", ""));
    rules->AddPresentationRule(*groupingRule);

    LabelOverrideP labelOverride = new LabelOverride("ThisNode.ClassName=\"C\"", 1, "\"Label \" & this.Property", "");
    rules->AddPresentationRule(*labelOverride);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // expect 1 ClassS node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceA);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );

    // expect 1 child property grouping node with label of instanceC
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyPropertyGroupingNode(*childNodes[0], "", { instanceB }, { ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceC).GetId()) });
    EXPECT_STREQ("Label 3", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // request for grandchildren
    DataContainer<NavNodeCPtr> grandchildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
        );

    // expect 1 grandchild instance node
    ASSERT_EQ(1, grandchildNodes.GetSize());
    VerifyNodeInstance(*grandchildNodes[0], *instanceB);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsBySkippedRelationshipProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <ECProperty propertyName="Priority" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is owned by" polymorphic="False">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsBySkippedRelationshipProperty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP rel1 = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = GetClass("B_Has_C")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel1, *instanceA, *instanceB, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel2, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 1,
        "", RequiredRelationDirection_Forward, GetSchema()->GetName(), rel2->GetFullName(), classC->GetFullName()));
    rules->AddPresentationRule(*childNodeRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A_Has_B", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Priority", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // expect 1 A node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceA);

    // request for children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );

    // expect 1 child property grouping node
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyPropertyGroupingNode(*childNodes[0], "", { instanceC }, { ECValue(5) });
    ASSERT_STREQ("5", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsMultipleClassesByTheSameRelationshipProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
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
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <ECProperty propertyName="Priority" typeName="int" />
        <Source multiplicity="(0..1)" roleLabel="A References B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B References A" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsMultipleClassesByTheSameRelationshipProperty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP rel = GetClass("A_Has_B")->GetRelationshipClassCP();

    // set up the hierarchy
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instanceA, *instanceC, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instanceA, *instanceD, [](IECInstanceR instance){instance.SetValue("Priority", ECValue(5));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootNodeRule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, GetSchema()->GetName(), rel->GetFullName(), Utf8PrintfString("%s,%s", classC->GetFullName(), classD->GetName().c_str())));
    rules->AddPresentationRule(*childNodeRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A_Has_B", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Priority", ""));
    rules->AddPresentationRule(*groupingRule);

    // request for root nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // expect 1 A node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceA);

    // expect 1 child property grouping node
    DataContainer<NavNodeCPtr> childNodes1 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes1.GetSize());
    VerifyPropertyGroupingNode(*childNodes1[0], "", { instanceC, instanceD }, { ECValue(5) });
    ASSERT_STREQ("5", childNodes1[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // expect 2 child instance nodes under the grouping node
    DataContainer<NavNodeCPtr> childNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes1[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes1[0].get())).get(); }
        );
    ASSERT_EQ(2, childNodes2.GetSize());
    VerifyNodeInstance(*childNodes2[0], *instanceC);
    VerifyNodeInstance(*childNodes2[1], *instanceD);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsAndSortsByPropertyValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="GroupingProperty" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsAndSortsByPropertyValue)
    {
    // Setup instance data
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.1)); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-1.0)); });
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.101)); });
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.01)); });
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue()); });

    // Setup rules
    auto propertyGroup = new PropertyGroup("", "", true, "GroupingProperty");
    propertyGroup->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    propertyGroup->SetSortingValue(PropertyGroupingValue::PropertyValue);

    auto groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*propertyGroup);

    auto rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rootNodeRule->AddCustomizationRule(*groupingRule);

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rootNodeRule);

    m_locater->AddRuleSet(*rules);

    // Act
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // Assert
    ASSERT_EQ(5, nodes.GetSize());

    VerifyPropertyGroupingNode(*nodes[0], "", { instance5 }, { ECValue() });
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[1], "", { instance2 }, { ECValue(-1.0) });
    EXPECT_STREQ("-1.00", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[2], "", { instance3 }, { ECValue(-0.101) });
    EXPECT_STREQ("-0.10", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[3], "", { instance1 }, { ECValue(-0.1) });
    EXPECT_STREQ("-0.10", nodes[3]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[4], "", { instance4 }, { ECValue(-0.01) });
    EXPECT_STREQ("-0.01", nodes[4]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByPropertyValueAndSortsByDisplayLabel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="GroupingProperty" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByPropertyValueAndSortsByDisplayLabel)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.1)); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-1.0)); });
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.101)); });
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.01)); });
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue()); });

    auto propertyGroup = new PropertyGroup("", "", true, "GroupingProperty");
    propertyGroup->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    propertyGroup->SetSortingValue(PropertyGroupingValue::DisplayLabel);

    auto groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*propertyGroup);

    auto rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rootNodeRule->AddCustomizationRule(*groupingRule);

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rootNodeRule);

    m_locater->AddRuleSet(*rules);

    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    ASSERT_EQ(5, nodes.GetSize());

    VerifyPropertyGroupingNode(*nodes[0], "", { instance4 }, { ECValue(-0.01) });
    EXPECT_STREQ("-0.01", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[1], "", { instance3 }, { ECValue(-0.101) });
    EXPECT_STREQ("-0.10", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[2], "", { instance1 }, { ECValue(-0.1) });
    EXPECT_STREQ("-0.10", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[3], "", { instance2 }, { ECValue(-1.0) });
    EXPECT_STREQ("-1.00", nodes[3]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[4], "", { instance5 }, { ECValue() });
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[4]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsAndSortsByDisplayLabel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="GroupingProperty" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsAndSortsByDisplayLabel)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.1)); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-1.0)); });
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.101)); });
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.01)); });
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue()); });

    auto propertyGroup = new PropertyGroup("", "", true, "GroupingProperty");
    propertyGroup->SetPropertyGroupingValue(PropertyGroupingValue::DisplayLabel);
    propertyGroup->SetSortingValue(PropertyGroupingValue::DisplayLabel);

    auto groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*propertyGroup);

    auto rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rootNodeRule->AddCustomizationRule(*groupingRule);

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rootNodeRule);

    m_locater->AddRuleSet(*rules);

    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    ASSERT_EQ(4, nodes.GetSize());

    VerifyPropertyGroupingNode(*nodes[0], "", { instance4 }, { ECValue(-0.01) });
    EXPECT_STREQ("-0.01", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[1], "", { instance1, instance3 }, { ECValue(-0.1), ECValue(-0.101) });
    EXPECT_STREQ("-0.10", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[2], "", { instance2 }, { ECValue(-1.0) });
    EXPECT_STREQ("-1.00", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[3], "", { instance5 }, { ECValue() });
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByDisplayLabelAndSortsByPropertyValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="GroupingProperty" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_GroupsByDisplayLabelAndSortsByPropertyValue)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.1)); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-1.0)); });
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.101)); });
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue(-0.01)); });
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [] (IECInstanceR instance) { instance.SetValue("GroupingProperty", ECValue()); });

    auto propertyGroup = new PropertyGroup("", "", true, "GroupingProperty");
    propertyGroup->SetPropertyGroupingValue(PropertyGroupingValue::DisplayLabel);
    propertyGroup->SetSortingValue(PropertyGroupingValue::PropertyValue);

    auto groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*propertyGroup);

    auto rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rootNodeRule->AddCustomizationRule(*groupingRule);

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rootNodeRule);

    m_locater->AddRuleSet(*rules);

    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    ASSERT_EQ(4, nodes.GetSize());

    VerifyPropertyGroupingNode(*nodes[0], "", { instance5 }, { ECValue() });
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[1], "", { instance2 }, { ECValue(-1.0) });
    EXPECT_STREQ("-1.00", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[2], "", { instance1, instance3 }, { ECValue(-0.1), ECValue(-0.101) });
    EXPECT_STREQ("-0.10", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*nodes[3], "", { instance4 }, { ECValue(-0.01) });
    EXPECT_STREQ("-0.01", nodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ReturnsNoResultsWhenGivenInvalidPropertyName, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_PropertyGroup_ReturnsNoResultsWhenGivenInvalidPropertyName)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    auto groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Test"));

    auto rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rootNodeRule->AddCustomizationRule(*groupingRule);

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rootNodeRule);

    m_locater->AddRuleSet(*rules);

    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_InstanceLabelOverride_SameLabelInstanceGroup, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_InstanceLabelOverride_SameLabelInstanceGroup)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    SameLabelInstanceGroupP sameLabelInstanceGroup = new SameLabelInstanceGroup("");
    groupingRule->AddGroup(*sameLabelInstanceGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 A instance node
    ASSERT_EQ(1, nodes.GetSize());
    ASSERT_STREQ(NAVNODE_TYPE_ECInstancesNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("InstanceA", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], {a1, a2});

    // request children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *b1);
    VerifyNodeInstance(*childNodes[1], *b2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], {a1, a2});

    // request children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *b1);
    VerifyNodeInstance(*childNodes[1], *b2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(6, rootNodes.GetSize());

    size_t i = 0;
    EXPECT_STREQ("c", rootNodes[i]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[i++], {c});

    EXPECT_STREQ("b11", rootNodes[i]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[i++], {b11});

#ifndef wip_enable_display_label_postprocessor
    EXPECT_STREQ("m", rootNodes[i]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[i++], { b12, b21, d });
#endif

    EXPECT_STREQ("b13", rootNodes[i]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[i++], {b13});

    EXPECT_STREQ("b22", rootNodes[i]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[i++], {b22});

    EXPECT_STREQ("b23", rootNodes[i]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[i++], {b23});

#ifdef wip_enable_display_label_postprocessor
    EXPECT_STREQ("m", rootNodes[i]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[i++], { b12, b21, d });
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_DuringPostProcessing_FromDifferentHierarchyLevels_SortOrderIsCorrectWhenNodesComeBeforeOtherSpecs, R"*(
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
    <ECRelationshipClass typeName="A_A" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="false">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup_DuringPostProcessing_FromDifferentHierarchyLevels_SortOrderIsCorrectWhenNodesComeBeforeOtherSpecs)
    {
    ECClassCP baseClass = GetClass("Element");
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_A")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("m")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *a11);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("m")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *a21);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b1")); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b2")); });
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b3")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, baseClass->GetFullName(), "UserLabel"));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false,
        "this.UserLabel = NULL", classA->GetFullName(), false));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classB->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    auto childSpec = new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", rel->GetFullName(), classA->GetFullName());
    childRule->AddSpecification(*childSpec);
    rules->AddPresentationRule(*childRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(4, rootNodes.GetSize());
    EXPECT_STREQ("m", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[0], {a11, a21});
    EXPECT_STREQ("b1", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[1], {b1});
    EXPECT_STREQ("b2", rootNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[2], {b2});
    EXPECT_STREQ("b3", rootNodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[3], {b3});
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_DuringPostProcessing_FromDifferentHierarchyLevels_SortOrderIsCorrectWhenNodesComeAfterOtherSpecs, R"*(
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
    <ECRelationshipClass typeName="A_A" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="false">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup_DuringPostProcessing_FromDifferentHierarchyLevels_SortOrderIsCorrectWhenNodesComeAfterOtherSpecs)
    {
    ECClassCP baseClass = GetClass("Element");
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_A")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *a11);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("a")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *a21);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b1")); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b2")); });
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("b3")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, baseClass->GetFullName(), "UserLabel"));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classB->GetFullName(), false));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false,
        "this.UserLabel = NULL", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    auto childSpec = new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", rel->GetFullName(), classA->GetFullName());
    childRule->AddSpecification(*childSpec);
    rules->AddPresentationRule(*childRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(4, rootNodes.GetSize());
    EXPECT_STREQ("b1", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[0], {b1});
    EXPECT_STREQ("b2", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[1], {b2});
    EXPECT_STREQ("b3", rootNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[2], {b3});
    EXPECT_STREQ("a", rootNodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[3], {a11, a21});
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());

    EXPECT_STREQ("a", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[0], {a1, a2, a3});

    EXPECT_STREQ("b", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[1], {b1, b2, b3});
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], {a});

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("b", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*childNodes[0], {b1, b2});
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("T_ROOT", rootNodes[0]->GetType().c_str());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_DuringPostProcessing_DeterminesChildrenForGroupedNodesWithChildrenHint, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_SameLabelInstanceGroup_DuringPostProcessing_DeterminesChildrenForGroupedNodesWithChildrenHint)
    {
    ECClassCP ecClass = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("a"));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("a"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, ecClass->GetFullName(), "Prop"));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Always, false, false, false, false,
        "", ecClass->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, ecClass->GetSchema().GetName(), ecClass->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("a", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_TRUE(rootNodes[0]->DeterminedChildren());
    EXPECT_TRUE(rootNodes[0]->HasChildren());
    VerifyNodeInstances(*rootNodes[0], { a1, a2 });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_GroupsByProperty_WithRanges, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges)
    {
    // insert some instances
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"), [](IECInstanceR instance){instance.SetValue("Property", ECValue(7));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "Property", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyRangeGroupingNode(*nodes[0], "", { instance }); // TODO: range grouping nodes don't have grouped values? { ECValue(7) }
    EXPECT_STREQ(CommonStrings::RULESENGINE_OTHER, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, children.GetSize());
    VerifyNodeInstance(*children[0], *instance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_GroupsByProperty_WithRanges_FilteringByIntegersRange, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByIntegersRange)
    {
    ECClassCP classA = GetClass("A");

    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(4));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "Property", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyRangeGroupingNode(*nodes[0], "", { instance }); // TODO: range grouping nodes don't have grouped values? { ECValue(4) }
    EXPECT_STREQ("Range", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, children.GetSize());
    VerifyNodeInstance(*children[0], *instance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_GroupsByProperty_WithRanges_FilteringByDoublesRange, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByDoublesRange)
    {
    ECClassCP classA = GetClass("A");

    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(4.5));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "Property", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyRangeGroupingNode(*nodes[0], "", { instance }); // TODO: range grouping nodes don't have grouped values? { ECValue(4.5) }
    EXPECT_STREQ("Range", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, children.GetSize());
    VerifyNodeInstance(*children[0], *instance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_GroupsByProperty_WithRanges_FilteringByLongsRange, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="long" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByLongsRange)
    {
    ECClassCP classA = GetClass("A");

    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue((int64_t)4));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "Property", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyRangeGroupingNode(*nodes[0], "", { instance }); // TODO: range grouping nodes don't have grouped values? { ECValue((int64_t)4) }
    EXPECT_STREQ("Range", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, children.GetSize());
    VerifyNodeInstance(*children[0], *instance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_GroupsByProperty_WithRanges_FilteringByDateTimeRange, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="dateTime" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_FilteringByDateTimeRange)
    {
    ECClassCP classA = GetClass("A");

    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(DateTime(2017, 5, 30)));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "Property", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("Range", "", "2017-05-01", "2017-06-01");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, nodes.GetSize());
    VerifyPropertyRangeGroupingNode(*nodes[0], "", { instance }); // TODO: range grouping nodes don't have grouped values? { ECValue(DateTime(2017, 5, 30)) }
    EXPECT_STREQ("Range", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 ECInstance child node
    DataContainer<NavNodeCPtr> children = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, children.GetSize());
    VerifyNodeInstance(*children[0], *instance);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_GroupsByProperty_WithRanges_WithHideIfNoChildrenParent, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A References B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B References A" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_GroupsByProperty_WithRanges_WithHideIfNoChildrenParent)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(4));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false,
        "", classB->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Backward, "", relationshipAHasB->GetFullName(), classA->GetFullName()));
    rules->AddPresentationRule(*childRule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP propertyGroup = new PropertyGroup("", "", true, "Property", "");
    PropertyRangeGroupSpecificationP propertyRangeGroupSpecification = new PropertyRangeGroupSpecification("", "", "1", "5");
    propertyGroup->AddRange(*propertyRangeGroupSpecification);
    groupingRule->AddGroup(*propertyGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // expect 1 instanceB node
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceB);

    // request children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );

    // make sure we have 1 property grouping node
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyPropertyRangeGroupingNode(*childNodes[0], "", { instanceA }); // TODO: range grouping nodes don't have grouped values? { ECValue(4) }
    EXPECT_STREQ("1 - 5", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // request grandchildren
    DataContainer<NavNodeCPtr> grandChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
        );

    // make sure we have the instanceA
    ASSERT_EQ(1, grandChildNodes.GetSize());
    VerifyNodeInstance(*grandChildNodes[0], *instanceA);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsByBaseClassAndByInstancesClasses, R"*(
    <ECEntityClass typeName="Base" />
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsByBaseClassAndByInstancesClasses)
    {
    // insert some instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("Derived"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();

    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, true, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "Base", "", "", "");
    ClassGroupP classGroup = new ClassGroup("", true, GetSchema()->GetName(), "Base");
    groupingRule->AddGroup(*classGroup);
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 class grouping node
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, nodes[0]->GetType().c_str());
    ASSERT_STREQ("Base", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure we have 1 classE child nodes
    DataContainer<NavNodeCPtr> classEChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classEChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, classEChildNodes[0]->GetType().c_str());
    ASSERT_STREQ("Derived", classEChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Recursion prevention
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnECInstanceNodesOfTheSameSpecificationAlreadyExistingInHierarchyWithoutChildren, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A Has B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B Has A" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnECInstanceNodesOfTheSameSpecificationAlreadyExistingInHierarchyWithoutChildren)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationship = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert instances with relationship
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationship, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, GetSchema()->GetName(), relationship->GetFullName(), Utf8PrintfString("%s,%s", classA->GetFullName(), classB->GetName().c_str()));
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*relatedNodeRule);

    // make sure we have 1 A root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // A instance node should have 1 B instance child node
    DataContainer<NavNodeCPtr> classAChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classAChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classAChildNodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceB).c_str(), classAChildNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // B instance node has 1 A instance node. There's such a node already in the hierarchy, but it's based on
    // different (root node rule) specification, so we allow it
    DataContainer<NavNodeCPtr> classBChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classAChildNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classAChildNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classBChildNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classBChildNodes[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // This time A instance node has children to show that this relationship is recurring.
    DataContainer<NavNodeCPtr> classAChildNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBChildNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classBChildNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, classAChildNodes2.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, classAChildNodes2[0]->GetType().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceB).c_str(), classAChildNodes2[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // This time B instance node has no children because there's such a node up in the
    // hierarchy and it's also based on the same specification.
    DataContainer<NavNodeCPtr> classBChildNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classAChildNodes2[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), classAChildNodes2[0].get())).get(); }
    );
    ASSERT_EQ(0, classBChildNodes2.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#624346
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupingWorksCorrectlyWithRelatedInstancesSpecificationWhenParentRelatedInstanceNodeLevelIsHidden, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A Has B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B Has A" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupingWorksCorrectlyWithRelatedInstancesSpecificationWhenParentRelatedInstanceNodeLevelIsHidden)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    /* Create the following hierarchy:
            + A
            +--+ Property grouping node
            |  +--- B
       Then, hide the first level (A). The expected result:
            + Property grouping node
            +--- B
    */
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* relatedNodeRule = new ChildNodeRule();
    RelatedInstanceNodesSpecificationP relatedInstanceNodesSpecification = new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Both, GetSchema()->GetName(), relationshipAHasB->GetFullName(), classB->GetFullName());
    relatedNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*relatedNodeRule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "B", "", "", "");
    PropertyGroup* propertyGroupSpec = new PropertyGroup("", "", true, "Property");
    groupingRule->AddGroup(*propertyGroupSpec);
    rules->AddPresentationRule(*groupingRule);

    // make sure we have 1 property grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instanceB }, { ECValue() });

    // the node should have 1 B child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *instanceB);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CustomizesNodesWhenCustomizationRulesDefinedInSupplementalRuleset, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CustomizesNodesWhenCustomizationRulesDefinedInSupplementalRuleset)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule sets
    PresentationRuleSetPtr primaryRules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*primaryRules);

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    primaryRules->AddPresentationRule(*rule);

    PresentationRuleSetPtr supplementalRules = PresentationRuleSet::CreateInstance(primaryRules->GetRuleSetId());
    supplementalRules->SetSupplementationPurpose("Customization");
    m_locater->AddRuleSet(*supplementalRules);

    LabelOverride* customizationRule = new LabelOverride("", 1, "\"Test\"", "");
    supplementalRules->AddPresentationRule(*customizationRule);

    // make sure we have 1 node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), primaryRules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), primaryRules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instance);
    EXPECT_STREQ("Test", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PolymorphicallyCustomizesChildNodesWhenCustomizationTargetIsExcludedNonPolymorphically, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, PolymorphicallyCustomizesChildNodesWhenCustomizationTargetIsExcludedNonPolymorphically)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(2, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classA->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classA->GetName().c_str() })},
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classA->GetSchema().GetName().c_str(), false, bvector<Utf8String> { classA->GetName().c_str() })}));
    rootRule->AddCustomizationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverrideStringValueSpecification("NewLabel") }));
    rules->AddPresentationRule(*rootRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(1, nodes.GetSize());

    VerifyNodeInstance(*nodes[0], *b);
    EXPECT_STREQ("NewLabel", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PolymorphicallyCustomizesChildNodesWhenSomeChildrenClassesExcluded, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, PolymorphicallyCustomizesChildNodesWhenSomeChildrenClassesExcluded)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(2, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classA->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classA->GetName().c_str() })},
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classB->GetSchema().GetName().c_str(), false, bvector<Utf8String> { classB->GetName().c_str() })}));
    rootRule->AddCustomizationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverrideStringValueSpecification("NewLabel") }));
    rules->AddPresentationRule(*rootRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, nodes.GetSize());

    VerifyNodeInstance(*nodes[0], *a);
    EXPECT_STREQ("NewLabel", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyNodeInstance(*nodes[1], *c);
    EXPECT_STREQ("NewLabel", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PolymorphicallyCustomizesChildNodesWhenCustomizationTargetClassExcluded, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, PolymorphicallyCustomizesChildNodesWhenCustomizationTargetClassExcluded)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto specification = new InstanceNodesOfSpecificClassesSpecification(2, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classA->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classA->GetName().c_str() })},
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classB->GetSchema().GetName().c_str(), false, bvector<Utf8String> { classB->GetName().c_str() })});
    specification->SetDoNotSort(true);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*specification);
    rootRule->AddCustomizationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), { new InstanceLabelOverrideStringValueSpecification("NewLabel") }));
    rules->AddPresentationRule(*rootRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, nodes.GetSize());

    VerifyNodeInstance(*nodes[0], *a);
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyNodeInstance(*nodes[1], *c);
    EXPECT_STREQ("NewLabel", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }


/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PolymorphicallyCustomizesNodeWhenCustomizationTargetParentIsExcluded, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, PolymorphicallyCustomizesNodeWhenCustomizationTargetParentIsExcluded)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto specification = new InstanceNodesOfSpecificClassesSpecification(2, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classA->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classA->GetName().c_str() })},
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classB->GetSchema().GetName().c_str(), false, bvector<Utf8String> { classB->GetName().c_str() })});
    specification->SetDoNotSort(true);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*specification);
    rootRule->AddCustomizationRule(*new InstanceLabelOverride(1, false, classC->GetFullName(), { new InstanceLabelOverrideStringValueSpecification("NewLabel") }));
    rules->AddPresentationRule(*rootRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]() { return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); });
    ASSERT_EQ(2, nodes.GetSize());

    VerifyNodeInstance(*nodes[0], *a);
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyNodeInstance(*nodes[1], *c);
    EXPECT_STREQ("NewLabel", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupingChildrenByRelatedInstanceProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Identifier" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Identifier" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupingChildrenByRelatedInstanceProperty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_Has_C")->GetRelationshipClassCP();

    /* Create the following hierarchy:
            + A
            +--+ C property grouping node
            |  +--- B + related instance C
    */
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){
        instance.SetValue("Description", ECValue("test"));
        instance.SetValue("Identifier", ECValue("InstanceA"));
        });
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Identifier", ECValue("test"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\"", 1, "this.Identifier", ""));

    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP relatedInstanceNodesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "instanceC.Identifier = parent.Description", classB->GetFullName(), false);
    relatedInstanceNodesSpecification->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relationshipBHasC->GetFullName(), classC->GetFullName(), "instanceC"));
    childNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*childNodeRule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "C", "", "", "");
    PropertyGroup* propertyGroupSpec = new PropertyGroup("", "", true, "Description");
    groupingRule->AddGroup(*propertyGroupSpec);
    rules->AddPresentationRule(*groupingRule);

    // make sure we have 1 instanceA
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("InstanceA", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // the node should have 1 property grouping node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyPropertyGroupingNode(*childNodes[0], "", { instanceB }, { ECValue() });

    // the child node should have 1 B child node
    DataContainer<NavNodeCPtr> grandChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, grandChildNodes.GetSize());
    VerifyNodeInstance(*grandChildNodes[0], *instanceB);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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

    // make sure we have 1 A instance
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { a }, { ECValue("test") });
    EXPECT_STREQ("test", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // the child node should have 1 B child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *a);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupingChildrenByRelatedInstanceProperty_InstanceLabelOverride, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Identifier" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Identifier" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupingChildrenByRelatedInstanceProperty_InstanceLabelOverride)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_Has_C")->GetRelationshipClassCP();

    /* Create the following hierarchy:
            + A
            +--+ C property grouping node
            |  +--- B + related C
    */
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){
        instance.SetValue("Description", ECValue("test"));
        instance.SetValue("Identifier", ECValue("InstanceA"));
        });
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Identifier", ECValue("test"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Identifier"));
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP instanceNodesOfSpecificClassesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*instanceNodesOfSpecificClassesSpecification);
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childNodeRule = new ChildNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP relatedInstanceNodesSpecification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "instanceC.Identifier = parent.Description", classB->GetFullName(), false);
    relatedInstanceNodesSpecification->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relationshipBHasC->GetFullName(), classC->GetFullName(), "instanceC"));
    childNodeRule->AddSpecification(*relatedInstanceNodesSpecification);
    rules->AddPresentationRule(*childNodeRule);

    GroupingRule* groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "C", "", "", "");
    PropertyGroup* propertyGroupSpec = new PropertyGroup("", "", true, "Description");
    groupingRule->AddGroup(*propertyGroupSpec);
    rules->AddPresentationRule(*groupingRule);

    // make sure we have 1 instanceA
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceA);

    // the node should have 1 property grouping node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyPropertyGroupingNode(*childNodes[0], "", { instanceB }, { ECValue() });

    // the child node should have 1 B child node
    DataContainer<NavNodeCPtr> grandChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, grandChildNodes.GetSize());
    VerifyNodeInstance(*grandChildNodes[0], *instanceB);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b1);
    VerifyNodeInstance(*rootNodes[1], *b2);

    // each node should have 1 ClassA child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *a);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsChildNodesWhenTheresOnlyOneLabelGroupingNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, ReturnsChildNodesWhenTheresOnlyOneLabelGroupingNode)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("TestValue"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("TestValue"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // make sure we have 2 nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("TestValue", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str());
    EXPECT_STREQ("TestValue", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RulesDrivenECPresentationManagerNavigationTests, ReturnsChildrenUsingAllSpecifications)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*CreateCustomNodeSpecification("a"));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type=\"a\"", 1, false, TargetTree_Both);
    childRule->AddSpecification(*CreateCustomNodeSpecification("b"));
    childRule->AddSpecification(*CreateCustomNodeSpecification("c"));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("a", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 2 child nodes
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childNodes.GetSize());
    EXPECT_STREQ("b", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("c", childNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AutoExpandSetsShouldAutoExpandFlagForRootNodes)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    // set auto expand property to true (default false)
    RootNodeRule* rootRule = new RootNodeRule("1=1", 10, false, TargetTree_Both, true);
    CustomNodeSpecificationP spec = new CustomNodeSpecification(1, true, "test", "test", "test", "test");
    rootRule->AddSpecification(*spec);
    rules->AddPresentationRule(*rootRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_TRUE(rootNodes[0]->ShouldAutoExpand());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FiltersNodesByParentNodes_MatchingFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByParentNodes_MatchingFilter)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test"));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"A\", \"%s\")", GetSchema()->GetName().c_str()), 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Property = parent.Property", classB->GetFullName(), false));
    rules->AddPresentationRule(*childRule);

    ChildNodeRule* childRule2 = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"B\", \"%s\")", GetSchema()->GetName().c_str()), 1, false, TargetTree_Both);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Property = parent.parent.Property", classC->GetFullName(), false));
    rules->AddPresentationRule(*childRule2);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceB).c_str(), childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 child node as well
    DataContainer<NavNodeCPtr> childNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes2.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceC).c_str(), childNodes2[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FiltersNodesByParentNodes_NonMatchingParentFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByParentNodes_NonMatchingParentFilter)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"A\", \"%s\")", GetSchema()->GetName().c_str()), 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Property = parent.Property", classB->GetFullName(), false));
    rules->AddPresentationRule(*childRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(0, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FiltersNodesByParentNodes_NonMatchingGrandparentFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByParentNodes_NonMatchingGrandparentFilter)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"A\", \"%s\")", GetSchema()->GetName().c_str()), 1, false, TargetTree_Both);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Property = parent.Property", classB->GetFullName(), false));
    rules->AddPresentationRule(*childRule);

    ChildNodeRule* childRule2 = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"B\", \"%s\")", GetSchema()->GetName().c_str()), 1, false, TargetTree_Both);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "this.Property = parent.parent.Property", classC->GetFullName(), false));
    rules->AddPresentationRule(*childRule2);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 1 child node
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceB).c_str(), childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // make sure it has 0 children
    DataContainer<NavNodeCPtr> childNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
        );
    ASSERT_EQ(0, childNodes2.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_FiltersGroupedNodesByGrandParentNodes_MatchingFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Value" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Value" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_FiltersGroupedNodesByGrandParentNodes_MatchingFilter)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Value", ECValue(1));});
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Value", ECValue(1));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    grandChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false,
        "this.Value = parent.parent.Value", classC->GetFullName(), false));
    rules->AddPresentationRule(*grandChildRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> aNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, aNodes.GetSize());
    VerifyNodeInstance(*aNodes[0], *a);

    // make sure it has 1 'B' grouping node
    DataContainer<NavNodeCPtr> bGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), aNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), aNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, bGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, bGroupingNodes[0]->GetKey()->GetType().c_str());

    // make sure it has 1 'B' instance node
    DataContainer<NavNodeCPtr> bNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, bNodes.GetSize());
    VerifyNodeInstance(*bNodes[0], *b);

    // make sure it has 1 'C' grouping node
    DataContainer<NavNodeCPtr> cGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, cGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, cGroupingNodes[0]->GetKey()->GetType().c_str());

    // make sure it has 1 'C' instance node
    DataContainer<NavNodeCPtr> cNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), cGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), cGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, cNodes.GetSize());
    VerifyNodeInstance(*cNodes[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_FiltersGroupedNodesByGrandParentNodes_MatchingFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Value" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Value" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_FiltersGroupedNodesByGrandParentNodes_MatchingFilter)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Value", ECValue(1)); });
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Value", ECValue(1)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.ClassName=\"%s\"", classA->GetName().c_str()), 1, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward))
        }));
    rules->AddPresentationRule(*childRule);

    ChildNodeRule* grandChildRule = new ChildNodeRule(Utf8PrintfString("ParentNode.ClassName=\"%s\"", classB->GetName().c_str()), 1, false, TargetTree_Both);
    grandChildRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "this.Value = parent.parent.Value",
        {
        new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward))
        }));
    rules->AddPresentationRule(*grandChildRule);

    // make sure we have 1 root node
    DataContainer<NavNodeCPtr> aNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, aNodes.GetSize());
    VerifyNodeInstance(*aNodes[0], *a);

    // make sure it has 1 'B' grouping node
    DataContainer<NavNodeCPtr> bGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), aNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), aNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, bGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, bGroupingNodes[0]->GetKey()->GetType().c_str());

    // make sure it has 1 'B' instance node
    DataContainer<NavNodeCPtr> bNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, bNodes.GetSize());
    VerifyNodeInstance(*bNodes[0], *b);

    // make sure it has 1 'C' grouping node
    DataContainer<NavNodeCPtr> cGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, cGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, cGroupingNodes[0]->GetKey()->GetType().c_str());

    // make sure it has 1 'C' instance node
    DataContainer<NavNodeCPtr> cNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), cGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), cGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, cNodes.GetSize());
    VerifyNodeInstance(*cNodes[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FiltersNodesByHiddenGrandParentNodesWhenParentNodesAreHidden, R"*(
    <ECEntityClass typeName="Subject" />
    <ECEntityClass typeName="Model" />
    <ECEntityClass typeName="Category" />
    <ECEntityClass typeName="Element">
        <ECNavigationProperty propertyName="Model" relationshipName="M_E" direction="Backward" />
        <ECNavigationProperty propertyName="Category" relationshipName="C_E" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="S_M" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Subject"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Model" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="M_E" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_E" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Category"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, FiltersNodesByHiddenGrandParentNodesWhenParentNodesAreHidden)
    {
    ECClassCP classS = GetClass("Subject");
    ECClassCP classM = GetClass("Model");
    ECClassCP classC = GetClass("Category");
    ECClassCP classE = GetClass("Element");
    ECRelationshipClassCP relSM = GetClass("S_M")->GetRelationshipClassCP();
    ECRelationshipClassCP relME = GetClass("M_E")->GetRelationshipClassCP();
    ECRelationshipClassCP relCE = GetClass("C_E")->GetRelationshipClassCP();

    IECInstancePtr s = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classS);
    IECInstancePtr m1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classM);
    IECInstancePtr m2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classM);
    IECInstancePtr m3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classM);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr e3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relSM, *s, *m1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relSM, *s, *m2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relSM, *s, *m3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relME, *m1, *e1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relME, *m2, *e2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relME, *m3, *e3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCE, *c, *e1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCE, *c, *e2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCE, *c, *e3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown,
        false, false, false, false, "", classS->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule("ParentNode.ClassName = \"Subject\"", 1, false, TargetTree_Both);
    childRule1->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "", {
        new RepeatableRelationshipPathSpecification({new RepeatableRelationshipStepSpecification(relSM->GetFullName(), RequiredRelationDirection_Forward)}),
        }));
    rules->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule("ParentNode.ClassName = \"Model\"", 1, false, TargetTree_Both);
    childRule2->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", {
        new RepeatableRelationshipPathSpecification({
            new RepeatableRelationshipStepSpecification(relME->GetFullName(), RequiredRelationDirection_Forward),
            new RepeatableRelationshipStepSpecification(relCE->GetFullName(), RequiredRelationDirection_Backward),
            }),
        }));
    rules->AddPresentationRule(*childRule2);

    GroupingRule* categoriesGrouping = new GroupingRule("ParentNode.ClassName = \"Model\"", 1000, false, classC->GetSchema().GetName(), classC->GetName(), "", "", "");
    categoriesGrouping->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*categoriesGrouping);

    ChildNodeRule* childRule3 = new ChildNodeRule("ParentNode.ClassName = \"Category\"", 1, false, TargetTree_Both);
    childRule3->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "this.Model.Id = parent.parent.ECInstanceId", {
        new RepeatableRelationshipPathSpecification({new RepeatableRelationshipStepSpecification(relCE->GetFullName(), RequiredRelationDirection_Forward)}),
        }));
    rules->AddPresentationRule(*childRule3);

    // make sure we have 1 S root node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *s);

    // make sure it has 1 C child
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c);

    // make sure it has 2 E children
    DataContainer<NavNodeCPtr> grandChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
        );
    ASSERT_EQ(3, grandChildNodes.GetSize());
    VerifyNodeInstance(*grandChildNodes[0], *e1);
    VerifyNodeInstance(*grandChildNodes[1], *e2);
    VerifyNodeInstance(*grandChildNodes[2], *e3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByDoubleProperty_NoDigitsAfterDecimalPointAppendTwoZeroes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_NoDigitsAfterDecimalPointAppendTwoZeroes)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2 }, { ECValue(2.) });
    EXPECT_STREQ("2.00", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByDoubleProperty_OneDigitAfterDecimalPointAppendOneZero, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_OneDigitAfterDecimalPointAppendOneZero)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.5));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.5));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2 }, { ECValue(2.5) });
    EXPECT_STREQ("2.50", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointSameResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointSameResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(0.00546));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(0.00798));
        });
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(0.00899));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2, instance3 }, { ECValue(0.00546), ECValue(0.00798), ECValue(0.00899) });
    EXPECT_STREQ("0.01", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 3 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(3, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    VerifyNodeInstance(*childrenNodes[2], *instance3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointDifferentResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsSecondDigitAfterDecimalPointDifferentResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.505f));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.504f));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());

    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance2 }, { ECValue(2.504f) });
    EXPECT_STREQ("2.50", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*rootNodes[1], "", { instance1 }, { ECValue(2.505f) });
    EXPECT_STREQ("2.51", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*firstNodeChildrenNodes[0], *instance2);

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get())).get(); }
        );
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*secondNodeChildrenNodes[0], *instance1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointSameResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointSameResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.59999999));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.59999999));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2 }, { ECValue(2.59999999) });
    EXPECT_STREQ("2.60", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointDifferentResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="double" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByDoubleProperty_MorePreciseNumbersRoundsFirstDigitAfterDecimalPointDifferentResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.595));
        });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue(2.594));
        });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByDouble = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByDouble);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());

    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance2 }, { ECValue(2.594) });
    EXPECT_STREQ("2.59", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*rootNodes[1], "", { instance1 }, { ECValue(2.595) });
    EXPECT_STREQ("2.60", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*firstNodeChildrenNodes[0], *instance2);

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get())).get(); }
        );
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*secondNodeChildrenNodes[0], *instance1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByPointProperty_EqualPoints, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPoints)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2 }, { ECValue(DPoint3d::From(1.12, 1.12, 1.12)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByPointProperty_AlmostEqualPointsSameResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_AlmostEqualPointsSameResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.121, 1.121, 1.121))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.122, 1.122, 1.122))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2 }, { ECValue(DPoint3d::From(1.121, 1.121, 1.121)), ECValue(DPoint3d::From(1.122, 1.122, 1.122)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByPointProperty_DifferentPointsDifferentResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_DifferentPointsDifferentResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.11, 1.11, 1.11))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());

    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1 }, { ECValue(DPoint3d::From(1.11, 1.11, 1.11)) });
    EXPECT_STREQ("X: 1.11 Y: 1.11 Z: 1.11", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*rootNodes[1], "", { instance2 }, { ECValue(DPoint3d::From(1.12, 1.12, 1.12)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*firstNodeChildrenNodes[0], *instance1);

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get())).get(); }
        );
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*secondNodeChildrenNodes[0], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByPointProperty_EqualPointsDifferentXCoordinatesDifferentResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPointsDifferentXCoordinatesDifferentResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.11, 1.12, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());

    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1 }, { ECValue(DPoint3d::From(1.11, 1.12, 1.12)) });
    EXPECT_STREQ("X: 1.11 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*rootNodes[1], "", { instance2 }, { ECValue(DPoint3d::From(1.12, 1.12, 1.12)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*firstNodeChildrenNodes[0], *instance1);

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get())).get(); }
        );
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*secondNodeChildrenNodes[0], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByPointProperty_EqualPointsDifferentYCoordinatesDifferentResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPointsDifferentYCoordinatesDifferentResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.11, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());

    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1 }, { ECValue(DPoint3d::From(1.12, 1.11, 1.12)) });
    EXPECT_STREQ("X: 1.12 Y: 1.11 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*rootNodes[1], "", { instance2 }, { ECValue(DPoint3d::From(1.12, 1.12, 1.12)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*firstNodeChildrenNodes[0], *instance1);

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get())).get(); }
        );
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*secondNodeChildrenNodes[0], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByPointProperty_EqualPointsDifferentZCoordinatesDifferentResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_EqualPointsDifferentZCoordinatesDifferentResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.12, 1.11))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 2 grouping nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());

    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1 }, { ECValue(DPoint3d::From(1.12, 1.12, 1.11)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.11", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyPropertyGroupingNode(*rootNodes[1], "", { instance2 }, { ECValue(DPoint3d::From(1.12, 1.12, 1.12)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure first grouping node has 1 child node
    DataContainer<NavNodeCPtr> firstNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, firstNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*firstNodeChildrenNodes[0], *instance1);

    //make sure second grouping node has 1 child node
    DataContainer<NavNodeCPtr> secondNodeChildrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get())).get(); }
        );
    ASSERT_EQ(1, secondNodeChildrenNodes.GetSize());
    VerifyNodeInstance(*secondNodeChildrenNodes[0], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByPointProperty_YZCoordinatesEqualXCoordinatesAlmostEqualSameResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_YZCoordinatesEqualXCoordinatesAlmostEqualSameResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.119, 1.12, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.124, 1.12, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2 }, { ECValue(DPoint3d::From(1.119, 1.12, 1.12)), ECValue(DPoint3d::From(1.124, 1.12, 1.12)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByPointProperty_XZCoordinatesEqualYCoordinatesAlmostEqualSameResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_XZCoordinatesEqualYCoordinatesAlmostEqualSameResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.119, 1.12))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.124, 1.12))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2 }, { ECValue(DPoint3d::From(1.12, 1.119, 1.12)), ECValue(DPoint3d::From(1.12, 1.124, 1.12)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GroupsNodesByPointProperty_XYCoordinatesEqualZCoordinatesAlmostEqualSameResult, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GroupsNodesByPointProperty_XYCoordinatesEqualZCoordinatesAlmostEqualSameResult)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.12, 1.119))); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue(DPoint3d::From(1.12, 1.12, 1.124))); });

    //create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    GroupingRuleP groupingRule = new GroupingRule("", 0, "", GetSchema()->GetName(), "A", "", "", "");
    PropertyGroupP groupByPoint = new PropertyGroup("", "", true, "Property", "");
    groupingRule->AddGroup(*groupByPoint);
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rule);

    //make sure we have 1 grouping node
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2 }, { ECValue(DPoint3d::From(1.12, 1.12, 1.119)), ECValue(DPoint3d::From(1.12, 1.12, 1.124)) });
    EXPECT_STREQ("X: 1.12 Y: 1.12 Z: 1.12", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    //make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    VerifyPropertyGroupingNode(*rootNodes[0], "", { instance1, instance2 }, { ECValue(DateTime(2019, 11, 28)) });
    EXPECT_STREQ("2019-11-28T00:00:00.000Z", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("dateTime", rootNodes[0]->GetLabelDefinition().GetTypeName().c_str());

    // make sure it has 2 children nodes
    DataContainer<NavNodeCPtr> childrenNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childrenNodes.GetSize());
    VerifyNodeInstance(*childrenNodes[0], *instance1);
    VerifyNodeInstance(*childrenNodes[1], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsFilteredNodesFromNotExpandedHierarchy, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsFilteredNodesFromNotExpandedHierarchy)
    {
    // insert some instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, true, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for filtered nodes paths
    bvector<NodesPathElement> nodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "")));

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.size());
    EXPECT_EQ(1, nodes[0].GetChildren().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsFilteredNodesMatchingPercentSymbol, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsFilteredNodesMatchingPercentSymbol)
    {
    ECClassCP classA = GetClass("A");

    // insert some instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("%")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "Property"));

    // request for filtered nodes paths
    bvector<NodesPathElement> nodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "%")));

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.size());
    ASSERT_EQ(1, nodes[0].GetFilteringData().GetOccurances());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsFilteredNodesMatchingUnderscoreSymbol, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsFilteredNodesMatchingUnderscoreSymbol)
    {
    ECClassCP classA = GetClass("A");

    // insert some instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("_")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "Property"));

    // request for filtered nodes paths
    bvector<NodesPathElement> nodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "_")));

    // make sure we have 1 node
    ASSERT_EQ(1, nodes.size());
    ASSERT_EQ(1, nodes[0].GetFilteringData().GetOccurances());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotReturnFilteredNodesNotMatchingPercentSymbol, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DoesNotReturnFilteredNodesNotMatchingPercentSymbol)
    {
    // insert some instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for filtered nodes paths
    bvector<NodesPathElement> nodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "%")));

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotReturnFilteredNodesNotMatchingCaretSymbol, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DoesNotReturnFilteredNodesNotMatchingCaretSymbol)
    {
    // insert some instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName()));
    rules->AddPresentationRule(*rule);

    // request for filtered nodes paths
    bvector<NodesPathElement> nodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "^")));

    // make sure we have 0 nodes
    ASSERT_EQ(0, nodes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsFilteredNodesUnderSameParent, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsFilteredNodesUnderSameParent)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert instances with relationship
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("InstanceB")); });
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("InstanceB")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecificationP classASpec = new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false);
    rootRule->AddSpecification(*classASpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Always, false, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", relationshipAHasB->GetFullName(), classB->GetFullName()));
    classASpec->AddNestedRule(*childRule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Property") }));

    // request for filtered nodes paths
    bvector<NodesPathElement> nodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "InstanceB")));

    // make sure we have 1 root node
    ASSERT_EQ(1, nodes.size());
    // make sure we have 2 child nodes
    EXPECT_EQ(2, nodes[0].GetChildren().size());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#381967
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsOnlyVisibleFilteredNodes, R"*(
    <ECEntityClass typeName="TestA" />
    <ECEntityClass typeName="TestB" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsOnlyVisibleFilteredNodes)
    {
    ECClassCP classA = GetClass("TestA");
    ECClassCP classB = GetClass("TestB");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, true, false, false, "", classA->GetFullName(), false));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), {new InstanceLabelOverrideClassLabelValueSpecification()}));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), {new InstanceLabelOverrideClassLabelValueSpecification()}));

    // request for filtered nodes paths
    bvector<NodesPathElement> nodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "Test")));

    // make sure we have 1 match
    ASSERT_EQ(1, nodes.size());
    VerifyNodeInstance(*nodes[0].GetNode(), *b);
    EXPECT_FALSE(nodes[0].GetNode()->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#469692
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsOnlyVisibleFilteredNodes_WhenParentsHidden, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsOnlyVisibleFilteredNodes_WhenParentsHidden)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), {new InstanceLabelOverrideClassLabelValueSpecification()}));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), {new InstanceLabelOverrideClassLabelValueSpecification()}));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classC->GetFullName(), {new InstanceLabelOverrideClassLabelValueSpecification()}));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, true, false, false, "", classA->GetFullName(), false));
    auto groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rootRule->AddCustomizationRule(*groupingRule);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRuleP childRule1 = new ChildNodeRule(Utf8PrintfString("ParentNode.InstanceId = %s", a1->GetInstanceId().c_str()), 1, false);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*childRule1);

    ChildNodeRuleP childRule2 = new ChildNodeRule(Utf8PrintfString("ParentNode.InstanceId = %s", a2->GetInstanceId().c_str()), 1, false);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*childRule2);

    // request for filtered nodes paths
    bvector<NodesPathElement> nodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "B")));

    // make sure we have 1 match, "TestC" under merged "TestA"
    ASSERT_EQ(1, nodes.size());
    VerifyNodeInstances(*nodes[0].GetNode(), {a1, a2});
    EXPECT_TRUE(nodes[0].GetNode()->HasChildren());
    ASSERT_EQ(1, nodes[0].GetChildren().size());
    VerifyNodeInstance(*nodes[0].GetChildren()[0].GetNode(), *b);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FindsNodesInPartiallyInitializedHierarchy, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="long" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, FindsNodesInPartiallyInitializedHierarchy)
    {
    ECClassCP classA = GetClass("A");

    for (size_t i = 0; i < 1001; ++i)
        RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [i](IECInstanceR instance){instance.SetValue("Prop", ECValue((uint64_t)i));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop") }));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    // partially initialize the root hierarchy level
    DataContainer<NavNodeCPtr> nodes = GetValidatedResponse(m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), PageOptions(0, 5))));
    ASSERT_EQ(5, nodes.GetSize());

    // request for filtered nodes paths from the uninitialized part
    bvector<NodesPathElement> filteredNodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "1000")));

    // make sure we have 1 match
    ASSERT_EQ(1, filteredNodes.size());
    ASSERT_STREQ("1000", filteredNodes[0].GetNode()->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_FALSE(filteredNodes[0].GetNode()->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FilteringPartiallyInitializedHierarchyDoesntDuplicateNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="long" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, FilteringPartiallyInitializedHierarchyDoesntDuplicateNodes)
    {
    ECClassCP classA = GetClass("A");

    for (size_t i = 0; i < 1001; ++i)
        RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [i](IECInstanceR instance) {instance.SetValue("Prop", ECValue((uint64_t)i)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop") }));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    // partially initialize the root hierarchy level
    DataContainer<NavNodeCPtr> nodes = GetValidatedResponse(m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), PageOptions(0, 5))));
    ASSERT_EQ(5, nodes.GetSize());

    // request for filtered nodes paths from the uninitialized part
    bvector<NodesPathElement> filteredNodes = GetValidatedResponse(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "1000")));

    // ensure total nodes count (from cache) is as expected
    EXPECT_EQ(1001, GetValidatedResponse(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()))));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_OverridesInstanceLabelAsFirstNotEmptyParameter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
        <ECProperty propertyName="EmptyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_OverridesInstanceLabelAsFirstNotEmptyParameter)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Instance1"));});
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "EmptyProperty,Property"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 2 nodes
    ASSERT_EQ(2, nodes.GetSize());
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Instance2", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());

    instanceNode = nodes[1];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("Instance1", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_FindsCorrectPropertyWithLowerCasePropertyNameAndOverridesLabel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_FindsCorrectPropertyWithLowerCasePropertyNameAndOverridesLabel)
    {
    ECClassCP classA = GetClass("A");

    // insert some instance
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "property"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );

    // make sure we have 1 nodes
    ASSERT_EQ(1, nodes.GetSize());
    NavNodeCPtr instanceNode = nodes[0];
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, instanceNode->GetType().c_str());
    ASSERT_STREQ("InstanceA", instanceNode->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_CreatesLabelUsingRelatedPropertyValue, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_CreatesLabelUsingRelatedPropertyValue)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("C1"));});
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("C2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(),
        {
        new InstanceLabelOverridePropertyValueSpecification("Prop", RelationshipPathSpecification(
            {
            new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward),
            new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward),
            })),
        }));

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ("C1", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_AppliesToSpecificRuleWhenNested, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_AppliesToSpecificRuleWhenNested)
    {
    ECClassCP classA = GetClass("A");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Prop", ECValue("test"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule1 = new RootNodeRule();
    rule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(2, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rule1->AddCustomizationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop") }));
    rules->AddPresentationRule(*rule1);

    RootNodeRule* rule2 = new RootNodeRule();
    rule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule2);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, nodes.GetSize());

    VerifyNodeInstance(*nodes[0], *a);
    EXPECT_STREQ("test", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    VerifyNodeInstance(*nodes[1], *a);
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_RespectsPriority, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop1" typeName="string" />
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_RespectsPriority)
    {
    ECClassCP classA = GetClass("A");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Prop1", ECValue("1"));
        instance.SetValue("Prop2", ECValue("2"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop1") }));
    rules->AddPresentationRule(*new InstanceLabelOverride(2, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop2") }));

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ("2", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_RespectsOnlyIfNotHandledAttribute, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop1" typeName="string" />
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_RespectsOnlyIfNotHandledAttribute)
    {
    ECClassCP classA = GetClass("A");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Prop2", ECValue("2"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop1") }));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop2") }));

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_GetsRelatedInstanceLabel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_GetsRelatedInstanceLabel)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("B"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop") }));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverrideRelatedInstanceLabelSpecification(RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        })) }));

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ("B", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_GetsRelatedInstanceLabel_PreventsInfiniteRecursion, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceLabelOverride_GetsRelatedInstanceLabel_PreventsInfiniteRecursion)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue("B")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), { new InstanceLabelOverrideRelatedInstanceLabelSpecification(RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Backward),
        })) }));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverrideRelatedInstanceLabelSpecification(RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        })) }));

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(6, rootNodes.GetSize());

    EXPECT_STREQ("Custom Element 1 UserLabel", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Element 2 CodeValue", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Element [0-3]", rootNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ("Custom Geometric Element 1 CodeValue", rootNodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Geometric Element 2 UserLabel [0-5]", rootNodes[4]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Geometric Element [0-6]", rootNodes[5]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> modelNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, modelNodes.GetSize());

    DataContainer<NavNodeCPtr> elementNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), modelNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), modelNodes[0].get())).get(); }
        );
    ASSERT_EQ(3, elementNodes.GetSize());
    EXPECT_STREQ("CodeValue", elementNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Custom Element-0-4", elementNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("UserLabel-0-3", elementNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA1->GetFullName(), "CodeValue"));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, true, false, false, false, "", classA1->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule();
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("ClassA1_CodeValue", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("ClassC_CodeValue", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("ClassA1_CodeValue", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ("ClassC_UserLabel", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "CodeValue"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    allInstanceNodesSpecification->SetDoNotSort(true);
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("ClassC_CodeValue", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("ClassA1_CodeValue", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "CodeValue"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "UserLabel"));

    RootNodeRule* rule = new RootNodeRule();
    AllInstanceNodesSpecificationP allInstanceNodesSpecification = new AllInstanceNodesSpecification(1, false, false, false, false, false, GetSchema()->GetName());
    rule->AddSpecification(*allInstanceNodesSpecification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ("1_Instance_C", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("2_Instance_A", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CreatesValidHierarchyWhenHidingMultipleHierarchyLevelsWithMultipleSpecificationsInARow)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("1", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());

    // get child nodes
    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(4, childNodes.GetSize());
    EXPECT_STREQ("1.1.1.1", childNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("1.1.1.2", childNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("1.1.2", childNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("1.2", childNodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    }

#ifdef wip_enable_display_label_postprocessor
/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideNodesInHierarchy_SortsSameLevelChildrenFromDifferentParentsAppropriately, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="BA" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, HideNodesInHierarchy_SortsSameLevelChildrenFromDifferentParentsAppropriately)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("1"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a1, *b11);
    IECInstancePtr b12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("3"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a1, *b12);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("2"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a2, *b21);
    IECInstancePtr b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("UserLabel", ECValue("4"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a2, *b22);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "UserLabel"));

    RootNodeRule* rootRule = new RootNodeRule();
    auto rootSpec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false);
    rootSpec->SetHideExpression("TRUE");
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)})
        }));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(4, rootNodes.GetSize());
    EXPECT_STREQ("1", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[0], { b11 });
    EXPECT_STREQ("2", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[1], { b21 });
    EXPECT_STREQ("3", rootNodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[2], { b12 });
    EXPECT_STREQ("4", rootNodes[3]->GetLabelDefinition().GetDisplayValue().c_str());
    VerifyNodeInstances(*rootNodes[3], { b22 });
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
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
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AppliesNodeExtendedDataUsingVirtualParentNode, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AppliesNodeExtendedDataUsingVirtualParentNode)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* nodesA = new RootNodeRule();
    nodesA->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*nodesA);

    ChildNodeRule* nodesB = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    nodesB->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*nodesB);

    ChildNodeRule* nodesC = new ChildNodeRule("ParentNode.ClassName = \"B\"", 1, false);
    nodesC->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classC->GetFullName(), false));
    rules->AddPresentationRule(*nodesC);

    ExtendedDataRule* ex = new ExtendedDataRule();
    ex->AddItem("ParentClassName", "ParentNode.ClassName");
    nodesC->AddCustomizationRule(*ex);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *c);

    RapidJsonAccessor extendedData = childNodes[0]->GetUsersExtendedData();
    ASSERT_TRUE(extendedData.GetJson().IsObject());
    ASSERT_EQ(1, extendedData.GetJson().MemberCount());
    ASSERT_TRUE(extendedData.GetJson().HasMember("ParentClassName"));
    EXPECT_STREQ(classB->GetName().c_str(), extendedData.GetJson()["ParentClassName"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* Based on VSTS#150682
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *subject1);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *partition);
    }

/*---------------------------------------------------------------------------------**//**
* Based on VSTS#150682
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(0, rootNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* Based on VSTS#150682
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *subject1);

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(0, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HideExpressionWithChildrenArtifacts_HidesBasedOnConditionalArtifacts, R"*(
    <ECEntityClass typeName="Model" />
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="IntProperty" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, HideExpressionWithChildrenArtifacts_HidesBasedOnConditionalArtifacts)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP relModelContainsElements = GetClass("ModelContainsElements")->GetRelationshipClassCP();

    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relModelContainsElements, *model1, *element1);

    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relModelContainsElements, *model2, *element2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* modelsRule = new RootNodeRule();
    auto modelsSpec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", modelClass->GetFullName(), true);
    modelsSpec->SetHideExpression("NOT ThisNode.ChildrenArtifacts.AnyMatches(x => x.IsSpecialChild)");
    modelsRule->AddSpecification(*modelsSpec);
    rules->AddPresentationRule(*modelsRule);

    ChildNodeRule* elementsRule = new ChildNodeRule("ParentNode.ClassName = \"Model\"", 1000, false, TargetTree_Both);
    elementsRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(relModelContainsElements->GetFullName(), RequiredRelationDirection_Forward),
            }),
        }));
    rules->AddPresentationRule(*elementsRule);

    bmap<Utf8String, Utf8String> artifactDefinitions;
    artifactDefinitions.Insert("IsSpecialChild", "TRUE");
    elementsRule->AddCustomizationRule(*new NodeArtifactsRule("this.IntProperty = 1", artifactDefinitions));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions) { return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&]() { return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
    );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *model1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    NavNodeCPtr rootNode = rootNodes[0];
    EXPECT_FALSE(rootNode->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(3, rootNodes.GetSize());

    NavNodeCPtr geometricGroupingNode = rootNodes[0];
    NavNodeCPtr physicalGroupingNode = rootNodes[1];
    NavNodeCPtr spatialGroupingNode = rootNodes[2];
    EXPECT_TRUE(nullptr != geometricGroupingNode->GetKey()->AsECClassGroupingNodeKey());
    EXPECT_TRUE(nullptr != physicalGroupingNode->GetKey()->AsECClassGroupingNodeKey());
    EXPECT_TRUE(nullptr != spatialGroupingNode->GetKey()->AsECClassGroupingNodeKey());

    DataContainer<NavNodeCPtr> geometricNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), geometricGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), geometricGroupingNode.get())).get(); }
        );
    ASSERT_EQ(2, geometricNodes.GetSize());
    VerifyNodeInstance(*geometricNodes[0], *geometric1);
    VerifyNodeInstance(*geometricNodes[1], *geometric2);

    DataContainer<NavNodeCPtr> physicalNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalGroupingNode.get())).get(); }
        );
    ASSERT_EQ(2, physicalNodes.GetSize());
    VerifyNodeInstance(*physicalNodes[0], *physical1);
    VerifyNodeInstance(*physicalNodes[1], *physical2);

    DataContainer<NavNodeCPtr> spatialNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), spatialGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), spatialGroupingNode.get())).get(); }
        );
    ASSERT_EQ(2, spatialNodes.GetSize());
    VerifyNodeInstance(*spatialNodes[0], *spatial1);
    VerifyNodeInstance(*spatialNodes[1], *spatial2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(3, rootNodes.GetSize());

    NavNodeCPtr drawingElementsGroupingNode = rootNodes[0];
    NavNodeCPtr physicalElements1GroupingNode = rootNodes[1];
    NavNodeCPtr physicalElements2GroupingNode = rootNodes[2];
    ASSERT_TRUE(nullptr != drawingElementsGroupingNode->GetKey()->AsECClassGroupingNodeKey());
    ASSERT_TRUE(nullptr != physicalElements1GroupingNode->GetKey()->AsECClassGroupingNodeKey());
    ASSERT_TRUE(nullptr != physicalElements2GroupingNode->GetKey()->AsECClassGroupingNodeKey());

    DataContainer<NavNodeCPtr> drawingElementNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), drawingElementsGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), drawingElementsGroupingNode.get())).get(); }
        );
    ASSERT_EQ(1, drawingElementNodes.GetSize());
    VerifyNodeInstance(*drawingElementNodes[0], *drawingElement1);

    DataContainer<NavNodeCPtr> physicalElement1Nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalElements1GroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalElements1GroupingNode.get())).get(); }
        );
    ASSERT_EQ(1, physicalElement1Nodes.GetSize());
    VerifyNodeInstance(*physicalElement1Nodes[0], *physicalElement1);

    DataContainer<NavNodeCPtr> physicalElement2Nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalElements2GroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalElements2GroupingNode.get())).get(); }
        );
    ASSERT_EQ(1, physicalElement2Nodes.GetSize());
    VerifyNodeInstance(*physicalElement2Nodes[0], *physicalElement2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(3, rootNodes.GetSize());

    NavNodeCPtr drawingClassGroupingNode = rootNodes[0];
    ASSERT_TRUE(nullptr != drawingClassGroupingNode->GetKey()->AsECClassGroupingNodeKey());

    NavNodeCPtr physicalClassGroupingNode = rootNodes[1];
    ASSERT_TRUE(nullptr != physicalClassGroupingNode->GetKey()->AsECClassGroupingNodeKey());

    NavNodeCPtr elementNode = rootNodes[2];
    VerifyNodeInstance(*elementNode, *element);

    DataContainer<NavNodeCPtr> drawingClassGroupChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), drawingClassGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), drawingClassGroupingNode.get())).get(); }
        );
    ASSERT_EQ(1, drawingClassGroupChildren.GetSize());
    VerifyNodeInstance(*drawingClassGroupChildren[0], *drawingElement);

    DataContainer<NavNodeCPtr> physicalClassGroupChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalClassGroupingNode.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalClassGroupingNode.get())).get(); }
        );
    ASSERT_EQ(3, physicalClassGroupChildren.GetSize());
    ASSERT_TRUE(nullptr != physicalClassGroupChildren[0]->GetKey()->AsECClassGroupingNodeKey());
    VerifyNodeInstance(*physicalClassGroupChildren[1], *physicalElement);
    VerifyNodeInstance(*physicalClassGroupChildren[2], *physicalElement2);

    DataContainer<NavNodeCPtr> physical1ClassGroupChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalClassGroupChildren[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), physicalClassGroupChildren[0].get())).get(); }
        );
    ASSERT_EQ(1, physical1ClassGroupChildren.GetSize());
    VerifyNodeInstance(*physical1ClassGroupChildren[0], *physicalElement1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_DoesNotDuplicateClassGroupingNode, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_DoesNotDuplicateClassGroupingNode)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false,
        "", classA->GetFullName(), true));

    GroupingRuleP baseClassGroup = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    baseClassGroup->AddGroup(*new ClassGroup("", true, "", ""));
    rootRule->AddCustomizationRule(*baseClassGroup);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_TRUE(nullptr != rootNodes[0]->GetKey()->AsECClassGroupingNodeKey());
    EXPECT_EQ(classA, &rootNodes[0]->GetKey()->AsECClassGroupingNodeKey()->GetECClass());
    EXPECT_TRUE(nullptr != rootNodes[1]->GetKey()->AsECClassGroupingNodeKey());
    EXPECT_EQ(classB, &rootNodes[1]->GetKey()->AsECClassGroupingNodeKey()->GetECClass());

    DataContainer<NavNodeCPtr> aChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, aChildren.GetSize());
    VerifyNodeInstance(*aChildren[0], *a);

    DataContainer<NavNodeCPtr> bChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[1].get())).get(); }
        );
    ASSERT_EQ(2, bChildren.GetSize());
    EXPECT_TRUE(nullptr != bChildren[0]->GetKey()->AsECClassGroupingNodeKey());
    EXPECT_EQ(classC, &bChildren[0]->GetKey()->AsECClassGroupingNodeKey()->GetECClass());
    VerifyNodeInstance(*bChildren[1], *b);

    DataContainer<NavNodeCPtr> cChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bChildren[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), bChildren[0].get())).get(); }
        );
    ASSERT_EQ(1, cChildren.GetSize());
    VerifyNodeInstance(*cChildren[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsRelatedInstancesByClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Grouping_ClassGroup_GroupsRelatedInstancesByClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP rel = GetClass("A_B")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1000, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    GroupingRuleP baseClassGroup = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    baseClassGroup->AddGroup(*new ClassGroup("", true, "", ""));
    rules->AddPresentationRule(*baseClassGroup);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    DataContainer<NavNodeCPtr> groupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, groupingNodes.GetSize());
    ASSERT_TRUE(groupingNodes[0]->GetKey()->AsECClassGroupingNodeKey() != nullptr);
    EXPECT_EQ(classB, &groupingNodes[0]->GetKey()->AsECClassGroupingNodeKey()->GetECClass());

    DataContainer<NavNodeCPtr> bNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), groupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), groupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, bNodes.GetSize());
    VerifyNodeInstance(*bNodes[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_Grouping_ClassGroup_GroupsRelatedInstancesByClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DEPRECATED_Grouping_ClassGroup_GroupsRelatedInstancesByClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP rel = GetClass("A_B")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), true));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1000, false, TargetTree_Both);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", rel->GetFullName(), ""));
    rules->AddPresentationRule(*childRule);

    GroupingRuleP baseClassGroup = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    baseClassGroup->AddGroup(*new ClassGroup("", true, "", ""));
    rules->AddPresentationRule(*baseClassGroup);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    DataContainer<NavNodeCPtr> groupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, groupingNodes.GetSize());
    ASSERT_TRUE(groupingNodes[0]->GetKey()->AsECClassGroupingNodeKey() != nullptr);
    EXPECT_EQ(classB, &groupingNodes[0]->GetKey()->AsECClassGroupingNodeKey()->GetECClass());

    DataContainer<NavNodeCPtr> bNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), groupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), groupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, bNodes.GetSize());
    VerifyNodeInstance(*bNodes[0], *c);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", element->GetFullName(), true));

    InstanceLabelOverride* labelOverrideRule = new InstanceLabelOverride(1, false, element->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("DateTimeProp") });
    rules->AddPresentationRule(*labelOverrideRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    NavNodeCPtr node = rootNodes[0];
    EXPECT_STREQ("2019-11-28T00:00:00.000Z", node->GetLabelDefinition().GetRawValue()->AsSimpleValue()->GetValue().GetString());
    EXPECT_STREQ("dateTime", node->GetLabelDefinition().GetTypeName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", element->GetFullName(), true));

    LabelOverride* labelOverride = new LabelOverride("", 1, "this.IntProperty", "");
    rules->AddPresentationRule(*labelOverride);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    NavNodeCPtr node = rootNodes[0];
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, node->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_DoesNotDuplicateManyToXRelatedNodesWithMergedParentNode, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed" description="Associates Elements with a Model">
        <Source multiplicity="(0..*)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RelatedInstanceNodes_DoesNotDuplicateManyToXRelatedNodesWithMergedParentNode)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a1, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a2, *b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false,
        "", classA->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1000, false, TargetTree_Both);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1000, ChildrenHint::Always, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)
            })
        })
    );

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    EXPECT_EQ(1, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
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
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());

    DataContainer<NavNodeCPtr> childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    EXPECT_EQ(1, childNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#269433
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortsCombinedHierarchyLevelsByDisplayLabel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SortsCombinedHierarchyLevelsByDisplayLabel)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("1"));});
    IECInstancePtr b12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("3"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a1, *b11);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a1, *b12);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a2, *b2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false,
        "", classA->GetFullName(), true));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1000, false);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)),
        }));

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), "Label"));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(3, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b11);
    VerifyNodeInstance(*rootNodes[1], *b2);
    VerifyNodeInstance(*rootNodes[2], *b12);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SuppressesInfiniteHierarchyByReturningNodeWithoutChildrenAfterDetectingSimilarAncestor, R"*(
    <ECEntityClass typeName="A" />
    <ECRelationshipClass typeName="A_A" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="aa" polymorphic="false">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="aa" polymorphic="false">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, SuppressesInfiniteHierarchyByReturningNodeWithoutChildrenAfterDetectingSimilarAncestor)
    {
    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP relAA = GetClass("A_A")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAA, *a, *a);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule();
    rules->AddPresentationRule(*childRule);
    auto childSpecification = new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(relAA->GetFullName(), RequiredRelationDirection_Forward)),
        });
    childRule->AddSpecification(*childSpecification);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
    );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    // should get A child node, because it's built using another spec
    auto childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
    );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *a);
    EXPECT_TRUE(childNodes[0]->HasChildren());

    // should get yet another A node, but without children this time - it has a similar ancestor built from the same spec
    auto finalChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
    );
    ASSERT_EQ(1, finalChildNodes.GetSize());
    VerifyNodeInstance(*finalChildNodes[0], *a);
    EXPECT_FALSE(finalChildNodes[0]->HasChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsSimilarNode11TimesIfSimilarAncestorsCheckIsDisabled, R"*(
    <ECEntityClass typeName="A" />
    <ECRelationshipClass typeName="A_To_A" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="is in" polymorphic="false">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="categorizes" polymorphic="false">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsSimilarNode11TimesIfSimilarAncestorsCheckIsDisabled)
    {
    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP rel = GetClass("A_To_A")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *a);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1000, false);
    rules->AddPresentationRule(*childRule);
    auto childSpecification = new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)),
        });
    childSpecification->SetSuppressSimilarAncestorsCheck(true);
    childRule->AddSpecification(*childSpecification);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);
    EXPECT_TRUE(rootNodes[0]->HasChildren());

    NavNodeCPtr parent = rootNodes[0];
    int count = 11;
    while (count--)
        {
        auto childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), parent.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), parent.get())).get(); }
        );
        ASSERT_EQ(1, childNodes.GetSize());
        VerifyNodeInstance(*childNodes[0], *a);
        EXPECT_EQ((count > 0), childNodes[0]->HasChildren());
        parent = childNodes[0];
        }

    auto finalChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), parent.get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), parent.get())).get(); }
        );
    ASSERT_EQ(0, finalChildNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllowsMultipleGroupingNodesBasedOnSameSpecificationIfGroupedNodesAreDifferent, R"*(
    <ECEntityClass typeName="A" />
    <ECRelationshipClass typeName="A_To_A" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="is in" polymorphic="false">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="categorizes" polymorphic="false">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, AllowsMultipleGroupingNodesBasedOnSameSpecificationIfGroupedNodesAreDifferent)
    {
    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP rel = GetClass("A_To_A")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a2, *a3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        Utf8PrintfString("this.ECInstanceId = %s", a1->GetInstanceId().c_str()), classA->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1000, false);
    rules->AddPresentationRule(*childRule);
    auto childSpecification = new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)),
        });
    childRule->AddSpecification(*childSpecification);

    // request for nodes

    auto rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a1);

    auto childGroupingNodes1 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childGroupingNodes1.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, childGroupingNodes1[0]->GetType().c_str());

    auto childNodes1 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childGroupingNodes1[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childGroupingNodes1[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes1.GetSize());
    VerifyNodeInstance(*childNodes1[0], *a2);

    auto childGroupingNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes1[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes1[0].get())).get(); }
        );
    ASSERT_EQ(1, childGroupingNodes2.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, childGroupingNodes2[0]->GetType().c_str());

    auto childNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childGroupingNodes2[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childGroupingNodes2[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes2.GetSize());
    VerifyNodeInstance(*childNodes2[0], *a3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PreventsMultipleGroupingNodesBasedOnSameSpecificationIfGroupedNodesAreTheSame, R"*(
    <ECEntityClass typeName="A" />
    <ECRelationshipClass typeName="A_To_A" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="false">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, PreventsMultipleGroupingNodesBasedOnSameSpecificationIfGroupedNodesAreTheSame)
    {
    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP rel = GetClass("A_To_A")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a2, *a2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        Utf8PrintfString("this.ECInstanceId = %s", a1->GetInstanceId().c_str()), classA->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1000, false);
    rules->AddPresentationRule(*childRule);
    auto childSpecification = new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)),
        });
    childRule->AddSpecification(*childSpecification);

    // request for nodes

    auto rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a1);

    auto childGroupingNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childGroupingNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, childGroupingNodes[0]->GetType().c_str());

    auto childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childGroupingNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childGroupingNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *a2);

    auto lastChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
      [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get()), pageOptions)).get(); },
      [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())).get(); }
    );
    ASSERT_EQ(1, lastChildNodes.GetSize());
    VerifyNodeInstance(*lastChildNodes[0], *a2);
    ASSERT_FALSE(lastChildNodes[0]->HasChildren());

    auto noChildNodes = RulesEngineTestHelpers::GetValidatedNodes(
      [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), lastChildNodes[0].get()), pageOptions)).get(); },
      [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), lastChildNodes[0].get())).get(); }
    );
    ASSERT_EQ(0, noChildNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetNodesWithMultipleVirtualHierarchyLevels, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GetNodesWithMultipleVirtualHierarchyLevels)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, nullptr, true);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, nullptr, true);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, nullptr, true);
    IECInstancePtr d1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, nullptr, true);
    IECInstancePtr d2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false));

    ChildNodeRule* classBRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    rules->AddPresentationRule(*classBRule);
    classBRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "", classB->GetFullName(), false));

    ChildNodeRule* classCRule = new ChildNodeRule("ParentNode.ClassName = \"B\"", 1, false);
    rules->AddPresentationRule(*classCRule);
    classCRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "", classC->GetFullName(), false));

    ChildNodeRule* classDRule = new ChildNodeRule("ParentNode.ClassName = \"C\"", 1, false);
    rules->AddPresentationRule(*classDRule);
    classDRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classD->GetFullName(), false));

    // request for nodes
    auto rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    auto childNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, childNodes.GetSize());
    VerifyNodeInstance(*childNodes[0], *d1);
    VerifyNodeInstance(*childNodes[1], *d2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFilter_AppliesInstanceFilterForFormattedProperty, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="LengthProperty" typeName="double" kindOfQuantity="Length" />
    </ECEntityClass>
    <KindOfQuantity typeName="Length" displayLabel="Length" persistenceUnit="M" relativeError="0" defaultPresentationUnit="M" presentationUnits="M;FT(fi8)"/>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFilter_AppliesInstanceFilterForFormattedProperty)
    {
    ECEntityClassCP classA = GetClass("ClassA")->GetEntityClassCP();

    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("LengthProperty", ECValue(10.0));});
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("LengthProperty", ECValue(9.95));});
    IECInstancePtr instanceA3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("LengthProperty", ECValue(10.0)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "GetFormattedValue(this.LengthProperty, \"UsSurvey\") = \"32' 9 3/4\"\"\"", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceA1);
    VerifyNodeInstance(*rootNodes[1], *instanceA3);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFilter_AppliesInstanceFilterForFormattedRelatedProperty, R"*(
    <ECEntityClass typeName="A">
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="LengthProperty" typeName="double" kindOfQuantity="Length" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="false">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <KindOfQuantity typeName="Length" displayLabel="Length" persistenceUnit="M" relativeError="0" defaultPresentationUnit="M" presentationUnits="M(U)"/>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFilter_AppliesInstanceFilterForFormattedRelatedProperty)
    {
    ECEntityClassCP classA = GetClass("A")->GetEntityClassCP();
    ECEntityClassCP classB = GetClass("B")->GetEntityClassCP();
    ECRelationshipClassCP relAB = GetClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("LengthProperty", ECValue(9.95));});
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("LengthProperty", ECValue(10.0));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA1, *instanceB1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA2, *instanceB2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    RootNodeRule* rule = new RootNodeRule();
    auto* specification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "GetFormattedValue(related.LengthProperty) = \"10.0 m\"", classA->GetFullName(), false);
    rule->AddSpecification(*specification);
    specification->AddRelatedInstance(*new RelatedInstanceSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classB->GetFullName()), "related"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceA2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotDuplicateNodesWhenMultipleRelatedInstancesExists, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, DoesNotDuplicateNodesWhenMultipleRelatedInstancesExists)
    {
    ECEntityClassCP classA = GetClass("A")->GetEntityClassCP();
    ECEntityClassCP classB = GetClass("B")->GetEntityClassCP();
    ECRelationshipClassCP relAB = GetClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceB3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA1, *instanceB1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA1, *instanceB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA1, *instanceB3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    RootNodeRule* rule = new RootNodeRule();
    auto specification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    rule->AddSpecification(*specification);
    specification->AddRelatedInstance(*new RelatedInstanceSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classB->GetFullName()), "related"));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceA1);
    VerifyNodeInstance(*rootNodes[1], *instanceA2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FiltersNodesWithHasRelatedInstanceLambdaVersionFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="ChildrenClassId" typeName="long" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E">
        <BaseClass>D</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, FiltersNodesWithHasRelatedInstanceLambdaVersionFilter)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECClassCP classE = GetClass("E");

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr ab = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [&](IECInstanceR instance){instance.SetValue("ChildrenClassId", ECValue(classB->GetId().GetValue()));});
    IECInstancePtr ac = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [&](IECInstanceR instance){instance.SetValue("ChildrenClassId", ECValue(classC->GetId().GetValue()));});
    IECInstancePtr ad = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [&](IECInstanceR instance){instance.SetValue("ChildrenClassId", ECValue(classD->GetId().GetValue()));});
    IECInstancePtr ae = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [&](IECInstanceR instance){instance.SetValue("ChildrenClassId", ECValue(classE->GetId().GetValue()));});
    IECInstancePtr a0 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    UNUSED_VARIABLE(ad);
    UNUSED_VARIABLE(a0);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        Utf8PrintfString("this.HasRelatedInstance(\"%s\", x => x.IsOfClass(this.ChildrenClassId))", classC->GetFullName()), classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    ASSERT_EQ(3, nodes.GetSize());
    VerifyNodeInstance(*nodes[0], *ab);
    VerifyNodeInstance(*nodes[1], *ac);
    VerifyNodeInstance(*nodes[2], *ad);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceNodesOfSpecificClasses_InstanceFilterWithVariablesAndLambda, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceNodesOfSpecificClasses_InstanceFilterWithVariablesAndLambda)
    {
    // set up data set
    ECClassCP classA = GetClass("A");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "GetVariableIntValues(\"ids\").AnyMatches(x => x = this.ECInstanceId)", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request root nodes without variables
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
        );
    EXPECT_EQ(0, rootNodes.GetSize());

    // request root nodes with variables (a2 id)
    RulesetVariables variables;
    variables.SetIntValues("ids", { (int64_t)BeInt64Id::FromString(a2->GetInstanceId().c_str()).GetValueUnchecked() });

    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), variables), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), variables)).get(); }
        );
    EXPECT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], { a2 });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesCurrectHierarchyLevelConsistingFromOnlySameLabelGroupedNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CreatesCurrectHierarchyLevelConsistingFromOnlySameLabelGroupedNode)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    bvector<bpair<IECInstancePtr, IECInstancePtr>> instances;
    for (int i = 0; i < 5; ++i)
        {
        Utf8String label = Utf8PrintfString("Instance-%d", i);
        IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [&](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue(label.c_str()));});
        IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [&](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue(label.c_str()));});
        instances.push_back(make_bpair(instance1, instance2));
        }

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    RootNodeRule* rule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecification* spec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), false);
    // add hide expression to avoid caching total count
    spec->SetHideExpression("FALSE");
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    GroupingRule* groupingRule = new GroupingRule("", 1000, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "UserLabel"));

    // request root nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions) { return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get(); },
        [&]() { return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get(); }
    );
    ASSERT_EQ(5, nodes.GetSize());
    for (int i = 0; i < 5; ++i)
        {
        auto mergedInstances = instances.at(i);
        VerifyNodeInstances(*nodes[i], { mergedInstances.first, mergedInstances.second });
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreateChildGroupingNodeWithoutChildrenWhenItHasSimilarAncestorCreatedWithCustomFunctionInInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, CreateChildGroupingNodeWithoutChildrenWhenItHasSimilarAncestorCreatedWithCustomFunctionInInstanceFilter)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    Utf8PrintfString commonInstanceFilterWithCustomFunction("this.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str());

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, commonInstanceFilterWithCustomFunction, classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule1 = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classB->GetFullName(), false));
    rules->AddPresentationRule(*childRule1);

    ChildNodeRule* childRule2 = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, commonInstanceFilterWithCustomFunction, classA->GetFullName(), false));
    rules->AddPresentationRule(*childRule2);

    GroupingRule* groupingRuleA = new GroupingRule("", 1000, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRuleA->AddGroup(*new ClassGroup("", true, "", ""));
    rules->AddPresentationRule(*groupingRuleA);

    // verify the hierarchy
    DataContainer<NavNodeCPtr> groupingNodesA = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()), pageOptions)).get();},
        [&](){return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get();}
    );
    ASSERT_EQ(1, groupingNodesA.GetSize());
    VerifyClassGroupingNode(*groupingNodesA[0], "", { a }, classA, true);

    DataContainer<NavNodeCPtr> instanceNodesA = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), groupingNodesA[0].get()), pageOptions)).get();},
        [&](){return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), groupingNodesA[0].get())).get();}
    );
    ASSERT_EQ(1, instanceNodesA.GetSize());
    VerifyNodeInstance(*instanceNodesA[0], *a);

    DataContainer<NavNodeCPtr> instanceNodesB = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), instanceNodesA[0].get()), pageOptions)).get();},
        [&](){return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), instanceNodesA[0].get())).get();}
    );
    ASSERT_EQ(1, instanceNodesB.GetSize());
    VerifyNodeInstance(*instanceNodesB[0], *b);

    DataContainer<NavNodeCPtr> groupingNodesA2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), instanceNodesB[0].get()), pageOptions)).get(); },
        [&](){return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), instanceNodesB[0].get())).get(); }
    );
    ASSERT_EQ(1, groupingNodesA2.GetSize());
    VerifyClassGroupingNode(*groupingNodesA2[0], "", { a }, classA, true);

    DataContainer<NavNodeCPtr> noInstanceNodesA = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), groupingNodesA2[0].get()), pageOptions)).get(); },
        [&](){return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), groupingNodesA2[0].get())).get(); }
    );
    ASSERT_EQ(0, noInstanceNodesA.GetSize());

    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_ExcludedClassesWithLabelOveride, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstancesOfSpecificClasses_ExcludedClassesWithLabelOveride)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");

    //expected returned instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    //expected filtered out instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(2, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*> {
        new MultiSchemaClass(classA->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classA->GetName().c_str() })},
        bvector<MultiSchemaClass*> {
            new MultiSchemaClass(classC->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classC->GetName().c_str() })}));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classD->GetFullName(), { new InstanceLabelOverrideStringValueSpecification("NewLabel") }));
    rules->AddPresentationRule(*rootRule);

    // verify
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get();
        });

    ASSERT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstances(*rootNodes[0], { a });
    VerifyNodeInstances(*rootNodes[1], { b });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(OrdersNodeWithNotSpecifiedLabelLast, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, OrdersNodeWithNotSpecifiedLabelLast)
    {
    ECClassCP classA = GetClass("A");

    //expected returned instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("ElementName", ECValue("B")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), true));

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "ElementName"));
    rules->AddPresentationRule(*rootRule);

    // verify
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get();
        });
    ASSERT_EQ(2, rootNodes.GetSize());
    // node with Unspecified label last
    VerifyNodeInstances(*rootNodes[0], { b });
    VerifyNodeInstances(*rootNodes[1], { a });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(OrdersMultipleNodesWithNotSpecifiedLabelsLast, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, OrdersMultipleNodesWithNotSpecifiedLabelsLast)
    {
    ECClassCP classA = GetClass("A");

    //expected returned instances
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("ElementName", ECValue("A3")); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("ElementName", ECValue("A1")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), true));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "ElementName"));
    rules->AddPresentationRule(*rootRule);

    // verify
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())).get();
        });
    ASSERT_EQ(4, rootNodes.GetSize());
    // node with Unspecified label last
    VerifyNodeInstances(*rootNodes[0], { a1 });
    VerifyNodeInstances(*rootNodes[1], { a3 });
    VerifyNodeInstances(*rootNodes[2], { a4 });
    VerifyNodeInstances(*rootNodes[3], { a2 });
    }
