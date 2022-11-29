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
        "GetVariableIntValue(\"instance_id\") = this.ECInstanceId", classA->GetFullName(), false));

    // 0 root nodes with no ruleset variables
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr),
        {
        });

    // returns node for `instance1`
    RulesetVariables rulesetVariables1{
        RulesetVariableEntry("instance_id", BeInt64Id::FromString(instance1->GetInstanceId().c_str())),
        };
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables1, nullptr),
        {
        CreateInstanceNodeValidator({ instance1 }),
        });

    // returns node for `instance2`
    RulesetVariables rulesetVariables2{
        RulesetVariableEntry("instance_id", BeInt64Id::FromString(instance2->GetInstanceId().c_str())),
        };
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables2, nullptr),
        {
        CreateInstanceNodeValidator({ instance2 }),
        });
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
    rootRule->AddSpecification(*CreateCustomNodeSpecification("custom"));

    // doesn't return nodes from disabled spec
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr),
        {
        CreateCustomNodeValidator("custom"),
        });

    // returns nodes from enabled spec
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_instances", true) }), nullptr),
        {
        CreateInstanceNodeValidator({ instance }),
        CreateCustomNodeValidator("custom"),
        });
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

    // returns nodes when variable is not set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr),
        {
        CreateInstanceNodeValidator({ instance }),
        });

    // returns nodes when `hide_instances = false`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_instances", false) }), nullptr),
        {
        CreateInstanceNodeValidator({ instance }),
        });

    // doesn't return nodes when `hide_instances = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_instances", true) }), nullptr),
        {
        });
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
    rootRule->AddSpecification(*CreateCustomNodeSpecification("custom"));

    // returns nodes from both specs when variable is not set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr),
        {
        CreateInstanceNodeValidator({ instance }),
        CreateCustomNodeValidator("custom"),
        });

    // returns nodes from both specs when `hide_instances = false`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_instances", false) }), nullptr),
        {
        CreateInstanceNodeValidator({ instance }),
        CreateCustomNodeValidator("custom"),
        });

    // returns node from only one spec when `hide_instances = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_instances", true) }), nullptr),
        {
        CreateCustomNodeValidator("custom"),
        });
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

    // returns nodes from A spec when no variables are set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr),
        {
        CreateInstanceNodeValidator({ a }),
        });

    // returns no nodes when `hide_a = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_a", true) }), nullptr),
        {
        });

    // returns nodes from B spec when `hide_a = true` and `show_b = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_a", true), RulesetVariableEntry("show_b", true) }), nullptr),
        {
        CreateInstanceNodeValidator({ b }),
        });
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

    // returns nodes from B spec when no variables are set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr),
        {
        CreateInstanceNodeValidator({ b }),
        });

    // returns no nodes when `hide_b = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_b", true) }), nullptr),
        {
        });

    // returns nodes from A spec when `hide_b = true` and `show_a = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_b", true), RulesetVariableEntry("show_a", true) }), nullptr),
        {
        CreateInstanceNodeValidator({ a }),
        });
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

    // returns no children when no variables set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr),
        {
        CreateInstanceNodeValidator({ a }),
        });

    // returns children when `show_children = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_children", true) }), nullptr),
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            }),
        });
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
    childRule->AddSpecification(*CreateCustomNodeSpecification("custom"));

    // returns only custom node child when no variables set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr),
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateCustomNodeValidator("custom"),
            }),
        });

    // returns all children when `show_children = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_children", true) }), nullptr),
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            CreateCustomNodeValidator("custom"),
            }),
        });
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

    // doesn't return nodes when no variables set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr),
        {
        });

    // returns A nodes when `ViewType = "A"`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("ViewType", "A") }), nullptr),
        {
        CreateInstanceNodeValidator({ a }),
        });

    // returns B nodes when `ViewType = "B"`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("ViewType", "B") }), nullptr),
        {
        CreateInstanceNodeValidator({ b }),
        });
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

    // returns children when `show_child = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_child", true) }), nullptr),
        {
        ExpectedHierarchyDef(CreateCustomNodeValidator("root"),
            {
            CreateCustomNodeValidator("child")
            }),
        });

    // doesn't return children when `show_child = false`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_child", false) }), nullptr),
        {
        ExpectedHierarchyDef(CreateCustomNodeValidator("root"),
            {
            }),
        });
    }
