/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "HierarchyIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.Prop = 2");
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 })
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ b2 })
            }),
        });

    // verify getting child nodes with instance filter
    params.SetInstanceFilter("this.Prop = 2");
    params.SetParentNode(hierarchy[0].node.get());
    ValidateHierarchy(params,
        {
        });
    params.SetParentNode(hierarchy[1].node.get());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    SearchResultInstanceNodesSpecification* rootSpec = new SearchResultInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false);
    rootSpec->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("select * from %s", classA->GetECSqlName().c_str()),
        classA->GetSchema().GetName(), classA->GetName()));
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.Prop = 2");
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersClassGroupingNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersClassGroupingNodes)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(0));});
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(3));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a1, a2 }),
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 })
            }),
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b }),
            {
            CreateInstanceNodeValidator({ b })
            }),
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classC, false, { c }),
            {
            CreateInstanceNodeValidator({ c })
            }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.Prop > 1");
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a2 }),
            {
            CreateInstanceNodeValidator({ a2 })
            }),
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classC, false, { c }),
            {
            CreateInstanceNodeValidator({ c })
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersLabelGroupingNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="LabelProp" typeName="string" />
        <ECProperty propertyName="FilterProp" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersLabelGroupingNodes)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr a12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr a13 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(2));});
    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr a22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(2));});
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("3")); instance.SetValue("FilterProp", ECValue(1));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("LabelProp") }));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { a11, a12, a13 }),
            {
            CreateInstanceNodeValidator({ a11 }),
            CreateInstanceNodeValidator({ a12 }),
            CreateInstanceNodeValidator({ a13 }),
            }),
        ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("2", { a21, a22 }),
            {
            CreateInstanceNodeValidator({ a21 }),
            CreateInstanceNodeValidator({ a22 }),
            }),
        CreateInstanceNodeValidator({ a3 }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.FilterProp = 1");
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { a11, a12 }),
            {
            CreateInstanceNodeValidator({ a11 }),
            CreateInstanceNodeValidator({ a12 }),
            }),
        CreateInstanceNodeValidator({ a21 }),
        CreateInstanceNodeValidator({ a3 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersBaseClassGroupingNodes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersBaseClassGroupingNodes)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, "", ""));
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classB->GetSchema().GetName(), true, bvector<Utf8String>{ classB->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, true, { b1, b2 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 })
            }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.Prop = 1");
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, true, { b1 }),
            {
            CreateInstanceNodeValidator({ b1 })
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersPropertyGroupingNodesWhenFilteringByGroupingProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersPropertyGroupingNodesWhenFilteringByGroupingProperty)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Prop"));
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a1 }, ValueList{ ECValue(1) }),
            {
            CreateInstanceNodeValidator({ a1 }),
            }),
        ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a2 }, ValueList{ ECValue(2) }),
            {
            CreateInstanceNodeValidator({ a2 }),
            }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.Prop = 1");
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a1 }, ValueList{ ECValue(1) }),
            {
            CreateInstanceNodeValidator({ a1 })
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersPropertyGroupingNodesWhenFilteringByNonGroupingProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="GroupingProp" typeName="int" />
        <ECProperty propertyName="FilteringProp" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersPropertyGroupingNodesWhenFilteringByNonGroupingProperty)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("GroupingProp", ECValue(111));
        instance.SetValue("FilteringProp", ECValue(777));
        });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("GroupingProp", ECValue(222));
        instance.SetValue("FilteringProp", ECValue(777));
        });
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("GroupingProp", ECValue(222));
        instance.SetValue("FilteringProp", ECValue(888));
        });

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "GroupingProp"));
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a1 }, ValueList{ ECValue(111) }),
            {
            CreateInstanceNodeValidator({ a1 }),
            }),
        ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a2, a3 }, ValueList{ ECValue(222) }),
            {
            CreateInstanceNodeValidator({ a2 }),
            CreateInstanceNodeValidator({ a3 }),
            }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.FilteringProp = 888");
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a3 }, ValueList{ ECValue(222) }),
            {
            CreateInstanceNodeValidator({ a3 })
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersSameLabelInstanceNodes, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="LabelProp" typeName="string" />
        <ECProperty propertyName="FilterProp" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersSameLabelInstanceNodes)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr a12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr a13 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(2));});
    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr a22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(2));});
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("3")); instance.SetValue("FilterProp", ECValue(1));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("LabelProp") }));

    auto groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a11, a12, a13 }),
        CreateInstanceNodeValidator({ a21, a22 }),
        CreateInstanceNodeValidator({ a3 }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.FilterProp = 1");
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a11, a12 }),
        CreateInstanceNodeValidator({ a21 }),
        CreateInstanceNodeValidator({ a3 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideNodesInHierarchyInParentLevel, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideNodesInHierarchyInParentLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(3));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b3);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b1 }),
        CreateInstanceNodeValidator({ b2 }),
        CreateInstanceNodeValidator({ b3 }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.Prop > 2");
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b3 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideNodesInHierarchyInParentLevelWhenMultipleSpecsAreUsedForChildren, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideNodesInHierarchyInParentLevelWhenMultipleSpecsAreUsedForChildren)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(10));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);

    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(15));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a3, *b3);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "this.Prop < 10",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "this.Prop = 10",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "this.Prop > 10",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b1 }),
        CreateInstanceNodeValidator({ b2 }),
        CreateInstanceNodeValidator({ b3 }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.Prop = 15");
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b3 }),
        });
    }

#ifdef WIP_PER_CLASS_INSTANCE_FILTERING
// this test creates a case where a hierarchy level is possibly combined from parent and child
// hierarchy levels - we need a way to create instance filters based on class to handle this
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideExpressionInParentLevel, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideExpressionInParentLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecification* rootSpec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {});
    rootSpec->SetHideExpression("TRUE");
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b1 }),
        CreateInstanceNodeValidator({ b2 }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.Prop = 1");
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b1 }),
        });
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideIfNoChildren, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideIfNoChildren)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);

    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, true, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            {
            CreateInstanceNodeValidator({ b2 }),
            }),
        });

    // verify with instance filter
    params.SetInstanceFilter("this.Prop = 1");
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });
    }
