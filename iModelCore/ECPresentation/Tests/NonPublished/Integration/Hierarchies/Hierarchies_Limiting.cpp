/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "HierarchyIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TResult>
static void ExpectHierarchyLevelTooLargeException(folly::Future<TResult>&& future, AsyncHierarchyRequestParams const& params)
    {
    ASSERT_TRUE(future.wait().hasException());
    try {
        future.getTry().throwIfFailed();
        }
    catch (ResultSetTooLargeError const& ex)
        {
        EXPECT_EQ(params.GetLimit(), ex.GetExceededSize());
        }
    catch (...)
        {
        FAIL() << "Invalid exception - expected `ResultSetTooLargeError`";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExpectHierarchyLevelTooLargeException(ECPresentationManagerR manager, AsyncHierarchyRequestParams const& params)
    {
    ExpectHierarchyLevelTooLargeException(manager.GetNodes(params), params);
    ExpectHierarchyLevelTooLargeException(manager.GetNodesCount(params), params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenThereAreTooManyInstancesOfTheSameClassInHierarchyLevel, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenThereAreTooManyInstancesOfTheSameClassInHierarchyLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    auto a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    auto a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit
    params.SetLimit(2);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(1);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenThereAreTooManyInstancesOfDifferentClassesInHierarchyLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenThereAreTooManyInstancesOfDifferentClassesInHierarchyLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    auto a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    auto b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*>
            {
            new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() }),
            new MultiSchemaClass(classB->GetSchema().GetName(), true, bvector<Utf8String>{ classB->GetName() })
            },
        {}));
    rules->AddPresentationRule(*rootRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            CreateInstanceNodeValidator({ a }),
            CreateInstanceNodeValidator({ b }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit
    params.SetLimit(2);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(1);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenThereAreTooManyInstancesInMultiSpecificationHierarchyLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenThereAreTooManyInstancesInMultiSpecificationHierarchyLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    auto a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    auto b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classB->GetSchema().GetName(), true, bvector<Utf8String>{ classB->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            CreateInstanceNodeValidator({ a }),
            CreateInstanceNodeValidator({ b }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit
    params.SetLimit(2);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(1);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenClassGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenClassGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    auto a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    auto b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    auto b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName(), classB->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a }),
                {
                CreateInstanceNodeValidator({ a }),
                }),
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b1, b2 }),
                {
                CreateInstanceNodeValidator({ b1 }),
                CreateInstanceNodeValidator({ b2 }),
                }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit
    params.SetLimit(3);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(2);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenLabelGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenLabelGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    auto a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("1"));});
    auto a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("1"));});
    auto a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("2"));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop") }));

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { a1, a2 }),
                {
                CreateInstanceNodeValidator({ a1 }),
                CreateInstanceNodeValidator({ a2 }),
                }),
            CreateInstanceNodeValidator({ a3 }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit
    params.SetLimit(3);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(2);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenSameLabelGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenSameLabelGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    auto a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("1"));});
    auto a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("1"));});
    auto a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("2"));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("Prop") }));

    auto groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    rules->AddPresentationRule(*groupingRule);

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            CreateInstanceNodeValidator({ a1, a2 }),
            CreateInstanceNodeValidator({ a3 }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit
    params.SetLimit(3);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(2);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenPropertyGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenPropertyGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    auto a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("1"));});
    auto a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("1"));});
    auto a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("2"));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "");
    groupingRule->AddGroup(*new PropertyGroup("", true, "Prop"));
    rules->AddPresentationRule(*groupingRule);

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a1, a2 }, { ECValue("1") }),
                {
                CreateInstanceNodeValidator({ a1 }),
                CreateInstanceNodeValidator({ a2 }),
                }),
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a3 }, { ECValue("2") }),
                {
                CreateInstanceNodeValidator({ a3 }),
                }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit
    params.SetLimit(3);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(2);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenBaseClassGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenBaseClassGroupingHierarchyLevelHasTooManyInstancesInHierarchyLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    auto a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    auto b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    auto b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto groupingRule = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "");
    groupingRule->AddGroup(*new ClassGroup());
    rules->AddPresentationRule(*groupingRule);

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, true, { b1, b2 }),
                {
                CreateInstanceNodeValidator({ b1 }),
                CreateInstanceNodeValidator({ b2 }),
                }),
            CreateInstanceNodeValidator({ a }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit
    params.SetLimit(3);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(2);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_DoesntThrowWhenThereAreTooManyInstancesInNonFilterableHierarchyLevel, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_DoesntThrowWhenThereAreTooManyInstancesInNonFilterableHierarchyLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    auto a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    auto a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rootRule = new RootNodeRule();
    auto spec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {});
    spec->SetHideExpression("false");
    rootRule->AddSpecification(*spec);
    rules->AddPresentationRule(*rootRule);

    // verify
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    params.SetLimit(1);
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a1 }),
        CreateInstanceNodeValidator({ a2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenThereAreTooManyInstancesInHierarchyLevelAndParentLevelIsHidden, R"*(
    <ECEntityClass typeName="A" />
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenThereAreTooManyInstancesInHierarchyLevelAndParentLevelIsHidden)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    auto a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    auto b11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b11);
    auto b12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b12);

    auto a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    auto b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b21);
    auto b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b22);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    auto childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            CreateInstanceNodeValidator({ b11 }),
            CreateInstanceNodeValidator({ b12 }),
            CreateInstanceNodeValidator({ b21 }),
            CreateInstanceNodeValidator({ b22 }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit
    params.SetLimit(4);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(3);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenThereAreTooManyInstancesInSameLabelMergedHierarchyLevel, R"*(
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenThereAreTooManyInstancesInSameLabelMergedHierarchyLevel)
    {
    // dataset
    ECClassCP classB = GetClass("B");

    auto b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("x"));});
    auto b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("2"));});
    auto b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("x"));});
    auto b4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("4"));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(),
        {
        new InstanceLabelOverridePropertyValueSpecification("Prop"),
        }));

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classB->GetSchema().GetName(), true, bvector<Utf8String>{ classB->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    auto groupingRule = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            CreateInstanceNodeValidator({ b2 }),
            CreateInstanceNodeValidator({ b4 }),
            CreateInstanceNodeValidator({ b1, b3 }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit (note: we count instances, not nodes)
    params.SetLimit(4);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(2);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Limiting_ThrowsWhenThereAreTooManyInstancesInSameLabelMergedHierarchyLevelWithHiddenIntermediateLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, Limiting_ThrowsWhenThereAreTooManyInstancesInSameLabelMergedHierarchyLevelWithHiddenIntermediateLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    auto a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    auto a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    auto b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("x")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    auto b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("2")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b2);

    auto b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("x")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b3);
    auto b4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("4")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b4);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(),
        {
        new InstanceLabelOverridePropertyValueSpecification("Prop"),
        }));

    auto groupingRule = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage::PostProcess));
    rules->AddPresentationRule(*groupingRule);

    auto rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    auto childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        }));
    rules->AddPresentationRule(*childRule);

    // set up full hierarchy validator
    auto validate = [&](AsyncHierarchyRequestParams const& p)
        {
        ValidateHierarchy(p,
            {
            CreateInstanceNodeValidator({ b2 }),
            CreateInstanceNodeValidator({ b4 }),
            CreateInstanceNodeValidator({ b1, b3 }),
            });
        };

    // validate the hierarchy with no limit
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    validate(params);

    // validate the hierarchy with large limit (note: we count instances, not nodes)
    params.SetLimit(4);
    validate(params);

    // verify request throws when limit is too small
    params.SetLimit(2);
    ExpectHierarchyLevelTooLargeException(*m_manager, params);
    }
