﻿/*---------------------------------------------------------------------------------------------
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableIntValue(\"instance_id\") = this.ECInstanceId", classA->GetFullName(), false));

    // 0 root nodes with no ruleset variables
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        });

    // returns node for `instance1`
    RulesetVariables rulesetVariables1{
        RulesetVariableEntry("instance_id", BeInt64Id::FromString(instance1->GetInstanceId().c_str())),
        };
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables1),
        {
        CreateInstanceNodeValidator({ instance1 }),
        });

    // returns node for `instance2`
    RulesetVariables rulesetVariables2{
        RulesetVariableEntry("instance_id", BeInt64Id::FromString(instance2->GetInstanceId().c_str())),
        };
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), rulesetVariables2),
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableBoolValue(\"show_instances\")", classA->GetFullName(), false));
    rootRule->AddSpecification(*CreateCustomNodeSpecification("custom"));

    // doesn't return nodes from disabled spec
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        CreateCustomNodeValidator("custom"),
        });

    // returns nodes from enabled spec
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_instances", true) })),
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT GetVariableBoolValue(\"hide_instances\")", classA->GetFullName(), false));

    // returns nodes when variable is not set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        CreateInstanceNodeValidator({ instance }),
        });

    // returns nodes when `hide_instances = false`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_instances", false) })),
        {
        CreateInstanceNodeValidator({ instance }),
        });

    // doesn't return nodes when `hide_instances = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_instances", true) })),
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT GetVariableBoolValue(\"hide_instances\")", classA->GetFullName(), false));
    rootRule->AddSpecification(*CreateCustomNodeSpecification("custom"));

    // returns nodes from both specs when variable is not set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        CreateInstanceNodeValidator({ instance }),
        CreateCustomNodeValidator("custom"),
        });

    // returns nodes from both specs when `hide_instances = false`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_instances", false) })),
        {
        CreateInstanceNodeValidator({ instance }),
        CreateCustomNodeValidator("custom"),
        });

    // returns node from only one spec when `hide_instances = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_instances", true) })),
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT GetVariableBoolValue(\"hide_a\")", classA->GetFullName(), false));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableBoolValue(\"show_b\")", classB->GetFullName(), false));

    // returns nodes from A spec when no variables are set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        CreateInstanceNodeValidator({ a }),
        });

    // returns no nodes when `hide_a = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_a", true) })),
        {
        });

    // returns nodes from B spec when `hide_a = true` and `show_b = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_a", true), RulesetVariableEntry("show_b", true) })),
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "GetVariableBoolValue(\"show_a\")", classA->GetFullName(), false));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT GetVariableBoolValue(\"hide_b\")", classB->GetFullName(), false));

    // returns nodes from B spec when no variables are set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        CreateInstanceNodeValidator({ b }),
        });

    // returns no nodes when `hide_b = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_b", true) })),
        {
        });

    // returns nodes from A spec when `hide_b = true` and `show_a = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_b", true), RulesetVariableEntry("show_a", true) })),
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
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
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        CreateInstanceNodeValidator({ a }),
        });

    // returns children when `show_children = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_children", true) })),
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
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
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateCustomNodeValidator("custom"),
            }),
        });

    // returns all children when `show_children = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_children", true) })),
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
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
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        });

    // returns A nodes when `ViewType = "A"`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("ViewType", "A") })),
        {
        CreateInstanceNodeValidator({ a }),
        });

    // returns B nodes when `ViewType = "B"`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("ViewType", "B") })),
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

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root"));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"root\" ANDALSO GetVariableBoolValue(\"show_child\")", 1000, false);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*CreateCustomNodeSpecification("child"));

    // returns children when `show_child = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_child", true) })),
        {
        ExpectedHierarchyDef(CreateCustomNodeValidator("root"),
            {
            CreateCustomNodeValidator("child")
            }),
        });

    // doesn't return children when `show_child = false`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("show_child", false) })),
        {
        ExpectedHierarchyDef(CreateCustomNodeValidator("root"),
            {
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectNodesWhenVariablesUsedInHideExpression)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root", [](auto& spec)
        {
        spec.SetHideExpression("GetVariableBoolValue(\"should_hide\")");
        }));

    // returns the node when no variables set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        CreateCustomNodeValidator("root")
        });

    // returns the node when `should_hide = false`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("should_hide", false) })),
        {
        CreateCustomNodeValidator("root")
        });

    // doesn't return the node when `should_hide = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("should_hide", true) })),
        {
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectNodesWhenVariablesUsedInChildHierarchyLevelsWithParentLevelHiddenIfNoChildren)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule("", 1000, false, true);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*CreateCustomNodeSpecification("root", [](auto& spec)
        {
        spec.SetHideIfNoChildren(true);
        }));

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"root\" ANDALSO NOT GetVariableBoolValue(\"hide_child\")", 1000, false);
    rules->AddPresentationRule(*childRule);
    childRule->AddSpecification(*CreateCustomNodeSpecification("child"));

    // returns both the root and the child node when no variables set
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables()),
        {
        ExpectedHierarchyDef(CreateCustomNodeValidator("root"),
            {
            CreateCustomNodeValidator("child")
            }),
        });

    // returns empty list when `hide_child = true`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_child", true) })),
        {
        });

    // return both the root and the child node when `hide_child = false`
    ValidateHierarchy(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables({ RulesetVariableEntry("hide_child", false) })),
        {
        ExpectedHierarchyDef(CreateCustomNodeValidator("root"),
            {
            CreateCustomNodeValidator("child")
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RulesetVariables_ReturnsCorrectNodesWhenMultipleChildSpecificationsHaveDifferentVariablesAndParentIsVirtual, R"*(
    <ECEntityClass typeName="A"/>
    <ECEntityClass typeName="B"/>
    <ECEntityClass typeName="C"/>
    <ECEntityClass typeName="D"/>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, RulesetVariables_ReturnsCorrectNodesWhenMultipleChildSpecificationsHaveDifferentVariablesAndParentIsVirtual)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");

    //expected returned instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rootRule = new RootNodeRule();
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "", classA->GetFullName(), true));

    auto childRule1 = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    rules->AddPresentationRule(*childRule1);
    childRule1->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "", classB->GetFullName(), true));

    auto childRule2 = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    rules->AddPresentationRule(*childRule2);
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT HasVariable(\"yyy\")", classC->GetFullName(), true));
    childRule2->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "NOT HasVariable(\"xxx\")", classD->GetFullName(), true));

    // verify
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a }),
            {
            CreateInstanceNodeValidator({ c }),
            CreateInstanceNodeValidator({ d }),
            }),
        });
    }