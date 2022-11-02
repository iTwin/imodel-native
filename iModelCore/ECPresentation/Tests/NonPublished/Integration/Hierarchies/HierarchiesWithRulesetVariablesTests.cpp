/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "HierarchyIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectRootNodesFromOneSpecification, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectRootNodesFromOneSpecification)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableIntValue(\"instance_id\")=this.ECInstanceId", classA->GetFullName(), false));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodesWithDefaultVariables = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr)).get(); }
        );
    ASSERT_EQ(0, rootNodesWithDefaultVariables.GetSize());

    RulesetVariables rulesetVariables1{
        RulesetVariableEntry("instance_id", BeInt64Id::FromString(instance1->GetInstanceId().c_str())),
        };
    DataContainer<NavNodeCPtr> rootNodes1 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables1, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables1, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes1.GetSize());
    VerifyNodeInstance(*rootNodes1[0], *instance1);

    RulesetVariables rulesetVariables2{
        RulesetVariableEntry("instance_id", BeInt64Id::FromString(instance2->GetInstanceId().c_str())),
        };
    DataContainer<NavNodeCPtr> rootNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables2, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables2, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes2.GetSize());
    VerifyNodeInstance(*rootNodes2[0], *instance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectRootNodesFromMultipleSpecifications, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectRootNodesFromMultipleSpecifications)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableBoolValue(\"show_instances\")", classA->GetFullName(), false));

    rootRule->AddSpecification(*new CustomNodeSpecification(1, false, "TYPE_Custom", "Custom Node", "", ""));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodesWithDefaultVariables = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodesWithDefaultVariables.GetSize());
    EXPECT_STREQ("Custom Node", rootNodesWithDefaultVariables[0]->GetLabelDefinition().GetDisplayValue().c_str());

    RulesetVariables rulesetVariables{
        RulesetVariableEntry("show_instances", true),
        };
    DataContainer<NavNodeCPtr> rootNodesWithVariables = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(2, rootNodesWithVariables.GetSize());
    VerifyNodeInstance(*rootNodesWithVariables[0], *instance);
    EXPECT_STREQ("Custom Node", rootNodesWithVariables[1]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectRootNodesForDefaultVariablesFromOneSpecification, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectRootNodesForDefaultVariablesFromOneSpecification)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT GetVariableBoolValue(\"hide_instances\")", classA->GetFullName(), false));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instance);

    RulesetVariables rulesetVariables1{
        RulesetVariableEntry("hide_instances", false),
        };
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables1, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables1, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instance);

    RulesetVariables rulesetVariables2{
        RulesetVariableEntry("hide_instances", true),
        };
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables2, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables2, nullptr)).get(); }
        );
    ASSERT_EQ(0, rootNodes.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectRootNodesForDefaultVariablesFromMultipleSpecifications, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectRootNodesForDefaultVariablesFromMultipleSpecifications)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT GetVariableBoolValue(\"hide_instances\")", classA->GetFullName(), false));

    rootRule->AddSpecification(*new CustomNodeSpecification(1, false, "TYPE_Custom", "Custom Node", "", ""));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr)).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instance);
    EXPECT_STREQ("Custom Node", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    RulesetVariables rulesetVariables1{
        RulesetVariableEntry("hide_instances", false),
        };
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables1, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables1, nullptr)).get(); }
        );
    ASSERT_EQ(2, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *instance);
    EXPECT_STREQ("Custom Node", rootNodes[1]->GetLabelDefinition().GetDisplayValue().c_str());

    RulesetVariables rulesetVariables2{
        RulesetVariableEntry("hide_instances", true),
        };
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables2, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables2, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ("Custom Node", rootNodes[0]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectRootNodesFromMultipleSpecificationsWhenInitiallyFirstHasNodesAndSecondDoesNot, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectRootNodesFromMultipleSpecificationsWhenInitiallyFirstHasNodesAndSecondDoesNot)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT GetVariableBoolValue(\"hide_a\")", classA->GetFullName(), false));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableBoolValue(\"show_b\")", classB->GetFullName(), false));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    RulesetVariables rulesetVariables{
        RulesetVariableEntry("hide_a", true),
        };
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(0, rootNodes.GetSize());

    rulesetVariables.SetBoolValue("show_b", true);
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectRootNodesFromMultipleSpecificationsWhenInitiallyFirstDoesNotHaveNodesAndSecondDoes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectRootNodesFromMultipleSpecificationsWhenInitiallyFirstDoesNotHaveNodesAndSecondDoes)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableBoolValue(\"show_a\")", classA->GetFullName(), false));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT GetVariableBoolValue(\"hide_b\")", classB->GetFullName(), false));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b);

    RulesetVariables rulesetVariables{
        RulesetVariableEntry("hide_b", true),
        };
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(0, rootNodes.GetSize());

    rulesetVariables.SetBoolValue("show_a", true);
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectChildrenFromOneSpecification, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectChildrenFromOneSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1000, false);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "GetVariableBoolValue(\"show_children\")",
        {
        new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)),
        }));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    DataContainer<NavNodeCPtr> aChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(0, aChildren.GetSize());

    RulesetVariables rulesetVariables{
        RulesetVariableEntry("show_children", true),
        };
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    aChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(2, aChildren.GetSize());
    VerifyNodeInstance(*aChildren[0], *b1);
    VerifyNodeInstance(*aChildren[1], *b2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectChildrenFromMultipleSpecifications, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectChildrenFromMultipleSpecifications)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.ClassName=\"A\"", 1000, false);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "GetVariableBoolValue(\"show_children\")",
        {
        new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)),
        }));

    childRule->AddSpecification(*new CustomNodeSpecification(1, false, "TYPE_Custom", "Custom Node", "", ""));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    DataContainer<NavNodeCPtr> aChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(1, aChildren.GetSize());
    EXPECT_STREQ("Custom Node", aChildren[0]->GetLabelDefinition().GetDisplayValue().c_str());

    RulesetVariables rulesetVariables{
        RulesetVariableEntry("show_children", true),
        };
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    aChildren = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, rootNodes[0].get()), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, rootNodes[0].get())).get(); }
        );
    ASSERT_EQ(3, aChildren.GetSize());
    VerifyNodeInstance(*aChildren[0], *b1);
    VerifyNodeInstance(*aChildren[1], *b2);
    EXPECT_STREQ("Custom Node", aChildren[2]->GetLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectNodesFromSubConditions, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectNodesFromSubConditions)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);

    SubCondition* aCondition = new SubCondition("GetVariableStringValue(\"ViewType\") = \"A\"");
    SubCondition* bCondition = new SubCondition("GetVariableStringValue(\"ViewType\") = \"B\"");
    rootRule->AddSubCondition(*aCondition);
    rootRule->AddSubCondition(*bCondition);

    aCondition->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classA->GetFullName(), false));
    bCondition->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "", classB->GetFullName(), false));

    // request for nodes
    DataContainer<NavNodeCPtr> rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr)).get(); }
        );
    ASSERT_EQ(0, rootNodes.GetSize());

    RulesetVariables rulesetVariables{
        RulesetVariableEntry("ViewType", "A"),
        };
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *a);

    rulesetVariables.SetStringValue("ViewType", "B");
    rootNodes = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes.GetSize());
    VerifyNodeInstance(*rootNodes[0], *b);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectNodesAfterChangingVariablesInChildNodeRuleCondition)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, TargetTree_Both, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root"));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"root\" ANDALSO GetVariableBoolValue(\"show_child\")", 1000, false, TargetTree_Both);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*CreateCustomNodeSpecification("child"));

    // verify nodes
    RulesetVariables rulesetVariables{
        RulesetVariableEntry("show_child", true),
        };
    DataContainer<NavNodeCPtr> rootNodes1 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes1.GetSize());
    EXPECT_STREQ("root", rootNodes1[0]->GetType().c_str());
    EXPECT_TRUE(rootNodes1[0]->HasChildren());

    rulesetVariables.SetBoolValue("show_child", false);
    DataContainer<NavNodeCPtr> rootNodes2 = RulesEngineTestHelpers::GetValidatedNodes(
        [&](PageOptionsCR pageOptions){ return m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr), pageOptions)).get(); },
        [&](){ return m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables, nullptr)).get(); }
        );
    ASSERT_EQ(1, rootNodes2.GetSize());
    EXPECT_STREQ("root", rootNodes2[0]->GetType().c_str());
    EXPECT_FALSE(rootNodes2[0]->HasChildren());
    }
