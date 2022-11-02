/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "HierarchyIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_SkipsSpecifiedNumberOfNodes_LabelOverride, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsSpecifiedNumberOfNodes_LabelOverride)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\"", 1, "this.Property", ""));

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(2))).get();
        });

    // expect 3 nodes: C, D, E
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("C", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("D", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("E", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_SkipsSpecifiedNumberOfNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsSpecifiedNumberOfNodes)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("Paging_SkipsSpecifiedNumberOfNodes");
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(2))).get();
        });

    // expect 3 nodes: C, D, E
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("C", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("D", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("E", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_SkippingMoreThanExists, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkippingMoreThanExists)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(5))).get();
        });

    // expect 0 nodes
    ASSERT_EQ(0, nodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_ReturnsSpecifiedNumberOfNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_ReturnsSpecifiedNumberOfNodes)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(0, 2))).get();
        });

    // expect 2 nodes: A, B
    ASSERT_EQ(2, nodes.GetSize());
    EXPECT_STREQ("A", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("B", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_ReturnsSpecifiedNumberOfNodesWhenUsingSameLabelPostProcessor, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Paging_ReturnsSpecifiedNumberOfNodesWhenUsingSameLabelPostProcessor)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("A")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("B")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("C")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("D")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("E")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "A", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(0, 3))).get();
        });

    // expect 3 nodes: A, B, C
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("A", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("B", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("C", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_PageSizeHigherThanTheNumberOfNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_PageSizeHigherThanTheNumberOfNodes)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("B"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(0, 5))).get();
        });

    // expect 2 nodes: A, B
    ASSERT_EQ(2, nodes.GetSize());
    EXPECT_STREQ("A", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("B", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_SkipsAndReturnsSpecifiedNumberOfNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsAndReturnsSpecifiedNumberOfNodes)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("C"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("D"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("E"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(1, 3))).get();
        });

    // expect 3 nodes: B, C, D
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("B", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("C", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("D", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_SkipsAndReturnsSpecifiedNumberOfNodesWhenHidingHierarchyLevels, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsAndReturnsSpecifiedNumberOfNodesWhenHidingHierarchyLevels)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("A")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("B")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("C")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("D")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("E")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    // note: we enable grouping by label which causes the first hierarchy level to contain virtual display label grouping nodes
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, true, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(1, 3))).get();
        });

    // expect 3 nodes: B, C, D
    ASSERT_EQ(3, nodes.GetSize());
    EXPECT_STREQ("B", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("C", nodes[1]->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("D", nodes[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_SkipsAndReturnsSpecifiedNumberOfNodesWhenSkippingMoreThanPageSize_WhenCountRequiresInitialization, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Paging_SkipsAndReturnsSpecifiedNumberOfNodesWhenSkippingMoreThanPageSize_WhenCountRequiresInitialization)
    {
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::DeleteInstances(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("A")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("B")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("C")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("D")); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("E")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    RootNodeRule* rule = new RootNodeRule();
    auto spec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false);
    spec->SetHideExpression("FALSE"); // set hide expression to disable getting nodes count through a query, just for testing reasons
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> nodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(3, 1))).get();
        });

    // expect 1 node: D
    ASSERT_EQ(1, nodes.GetSize());
    EXPECT_STREQ("D", nodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetsAllResultsWhenNumberOfNodesExceedsQueryBasedProviderPageSize, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, GetsAllResultsWhenNumberOfNodesExceedsQueryBasedProviderPageSize)
    {
    ECClassCP classA = GetClass("A");

    size_t numberOfNodes = QueryBasedNodesProvider::PartialProviderSize + 1;
    bvector<IECInstancePtr> instances;
    for (size_t i = 0; i < numberOfNodes; ++i)
        {
        instances.push_back(RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [&i](IECInstanceR instance)
            {
            instance.SetValue("Label", ECValue(std::to_string(i).c_str()));
            }));
        }

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "Label"));

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rootRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodesPage1 = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(0, 500))).get();
        });
    ASSERT_EQ(500, rootNodesPage1.GetSize());
    for (size_t i = 0; i < 500; ++i)
        VerifyNodeInstance(*rootNodesPage1[i], *instances[i]);

    DataContainer<NavNodeCPtr> rootNodesPage2 = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(500, 500))).get();
        });
    ASSERT_EQ(500, rootNodesPage2.GetSize());
    for (size_t i = 0; i < 500; ++i)
        VerifyNodeInstance(*rootNodesPage2[i], *instances[i + 500]);

    DataContainer<NavNodeCPtr> rootNodesPage3 = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(1000, 500))).get();
        });
    ASSERT_EQ(1, rootNodesPage3.GetSize());
    VerifyNodeInstance(*rootNodesPage3[0], *instances[1000]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsAllNodesWhenSkippingDuplicatesWithManyNodesInHierarchyLevel, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsAllNodesWhenSkippingDuplicatesWithManyNodesInHierarchyLevel)
    {
    ECEntityClassCP classA = GetClass("A")->GetEntityClassCP();
    ECEntityClassCP classB = GetClass("B")->GetEntityClassCP();
    ECRelationshipClassCP relAB = GetClass("A_To_B")->GetRelationshipClassCP();

    // insert an A instance which is related to 2 B instances
    IECInstancePtr instanceA0 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB01 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceB02 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA0, *instanceB01);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA0, *instanceB02);

    // insert 2000 A instances
    bvector<IECInstancePtr> aInstances;
    for (size_t i = 0; i < 2000; ++i)
        aInstances.push_back(RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    auto specification = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    specification->SetDoNotSort(true);
    specification->AddRelatedInstance(*new RelatedInstanceSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classB->GetFullName()), "related"));
    rule->AddSpecification(*specification);
    rules->AddPresentationRule(*rule);

    // request for nodes
    EXPECT_EQ(2001, GetValidatedResponse(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr))));

    DataContainer<NavNodeCPtr> rootNodes1 = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(0, 1000))).get();
        });
    EXPECT_EQ(1000, rootNodes1.GetSize());
    VerifyNodeInstance(*rootNodes1[0], *instanceA0);
    for (size_t i = 0; i < 999; ++i)
        VerifyNodeInstance(*rootNodes1[i + 1], *aInstances[i]);

    DataContainer<NavNodeCPtr> rootNodes2 = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(1000, 1000))).get();
        });
    EXPECT_EQ(1000, rootNodes2.GetSize());
    for (size_t i = 0; i < 1000; ++i)
        VerifyNodeInstance(*rootNodes2[i], *aInstances[i + 999]);

    DataContainer<NavNodeCPtr> rootNodes3 = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(2000, 1000))).get();
        });
    EXPECT_EQ(1, rootNodes3.GetSize());
    VerifyNodeInstance(*rootNodes3[0], *aInstances[1999]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_ReturnsNodesFromDifferentSpecifications, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D" />
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_C" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_D" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="D"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Paging_ReturnsNodesFromDifferentSpecifications)
    {
    ECEntityClassCP classA = GetClass("A")->GetEntityClassCP();
    ECEntityClassCP classB = GetClass("B")->GetEntityClassCP();
    ECEntityClassCP classC = GetClass("C")->GetEntityClassCP();
    ECEntityClassCP classD = GetClass("D")->GetEntityClassCP();
    ECRelationshipClassCP relAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relAC = GetClass("A_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relAD = GetClass("A_To_D")->GetRelationshipClassCP();

    // insert 3 A instances each related to 2 other instances
    IECInstancePtr instanceA0 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB01 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceB02 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceC11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr instanceC12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceD11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceD12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA0, *instanceB01);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA0, *instanceB02);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAC, *instanceA1, *instanceC11);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAC, *instanceA1, *instanceC12);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAD, *instanceA2, *instanceD11);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAD, *instanceA2, *instanceD12);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, true, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName = \"A\"", 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        { new RepeatableRelationshipPathSpecification({new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)}) }));
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        { new RepeatableRelationshipPathSpecification({new RepeatableRelationshipStepSpecification(relAC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)}) }));
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        { new RepeatableRelationshipPathSpecification({new RepeatableRelationshipStepSpecification(relAD->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)}) }));
    rules->AddPresentationRule(*childRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(1, 2))).get();
        });
    EXPECT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceB02);
    VerifyNodeInstance(*rootNodes[1], *instanceC11);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_ReturnsNodesFromDifferentSpecifications_WithSameLabelInstancePostProcessor, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D" />
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_C" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_D" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="is in" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="categorizes" polymorphic="false">
            <Class class="D"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Paging_ReturnsNodesFromDifferentSpecifications_WithSameLabelInstancePostProcessor)
    {
    ECEntityClassCP classA = GetClass("A")->GetEntityClassCP();
    ECEntityClassCP classB = GetClass("B")->GetEntityClassCP();
    ECEntityClassCP classC = GetClass("C")->GetEntityClassCP();
    ECEntityClassCP classD = GetClass("D")->GetEntityClassCP();
    ECRelationshipClassCP relAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relAC = GetClass("A_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relAD = GetClass("A_To_D")->GetRelationshipClassCP();

    // insert 3 A instances each related to 2 other instances
    IECInstancePtr instanceA0 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB01 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceB02 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceC11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr instanceC12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceD11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceD12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA0, *instanceB01);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA0, *instanceB02);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAC, *instanceA1, *instanceC11);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAC, *instanceA1, *instanceC12);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAD, *instanceA2, *instanceD11);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAD, *instanceA2, *instanceD12);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false));
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classC->GetFullName(), false));
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classD->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    auto groupingRule = new GroupingRule("", 1, false, classD->GetSchema().GetName(), classD->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(1, 2))).get();
        });
    EXPECT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instanceB02);
    VerifyNodeInstance(*rootNodes[1], *instanceC11);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Paging_ReturnsCorrectNodesSkippingFirstNodesWithManyNodesInHierarchyLevel, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Paging_ReturnsCorrectNodesSkippingFirstNodesWithManyNodesInHierarchyLevel)
    {
    ECEntityClassCP classA = GetClass("A")->GetEntityClassCP();

    // insert 2000 A instances
    bvector<IECInstancePtr> aInstances;
    for (size_t i = 0; i < 2000; ++i)
        aInstances.push_back(RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(1500, 1000))).get();
        });
    EXPECT_EQ(500, rootNodes.GetSize());
    for (size_t i = 0; i < 500; ++i)
        VerifyNodeInstance(*rootNodes[i], *aInstances[i + 1500]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsCorrectNodesCountWhenNumberOfNodesExceedsQueryBasedProviderPageSize, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, ReturnsCorrectNodesCountWhenNumberOfNodesExceedsQueryBasedProviderPageSize)
    {
    ECEntityClassCP classA = GetClass("A")->GetEntityClassCP();

    // insert double QueryBasedNodesProvider::PartialProviderSize number of nodes
    size_t numberOfNodes = QueryBasedNodesProvider::PartialProviderSize * 2;
    bvector<IECInstancePtr> aInstances;
    for (size_t i = 0; i < numberOfNodes; ++i)
        aInstances.push_back(RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rule = new RootNodeRule();
    rule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classA->GetFullName(), false));
    rules->AddPresentationRule(*rule);

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]()
        {
        return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), PageOptions(0, 20))).get();
        });
    EXPECT_EQ(20, rootNodes.GetSize());
    for (size_t i = 0; i < 20; ++i)
        VerifyNodeInstance(*rootNodes[i], *aInstances[i]);

    ASSERT_EQ(numberOfNodes, GetValidatedResponse(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr))));
    }
