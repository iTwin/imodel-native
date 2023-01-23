/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "HierarchyIntegrationTests.h"

typedef std::function<void(ContentDescriptor::Category const&)> FieldCategoryValidator;
typedef std::function<void(ContentDescriptor::Field const&)> FieldValidator;
typedef std::function<void(ContentDescriptor const&)> DescriptorValidator;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static FieldValidator CreatePropertiesFieldValidator(FieldCategoryValidator category, bvector<ECPropertyCP> const& props)
    {
    return [=](ContentDescriptor::Field const& field)
        {
        EXPECT_TRUE(field.IsPropertiesField());
        EXPECT_EQ(props.size(), field.AsPropertiesField()->GetProperties().size());
        for (size_t i = 0; i < props.size(); ++i)
            {
            EXPECT_EQ(props[i], &field.AsPropertiesField()->GetProperties()[i].GetProperty());
            }
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static FieldValidator CreatePropertiesFieldValidator(FieldCategoryValidator category, ECPropertyCR prop)
    {
    return CreatePropertiesFieldValidator(category, { &prop });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static FieldValidator CreatePropertiesFieldValidator(bvector<ECPropertyCP> const& props)
    {
    return CreatePropertiesFieldValidator(nullptr, props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static FieldValidator CreatePropertiesFieldValidator(ECPropertyCR prop)
    {
    return CreatePropertiesFieldValidator(nullptr, prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateDescriptor(ContentDescriptorCR actualDescriptor, bvector<FieldValidator> const& expectedFields)
    {
    EXPECT_EQ(expectedFields.size(), actualDescriptor.GetVisibleFields().size());
    for (size_t i = 0; i < actualDescriptor.GetVisibleFields().size(); ++i)
        {
        auto actualField = actualDescriptor.GetVisibleFields().at(i);
        auto const& fieldValidator = expectedFields[i];
        if (fieldValidator)
            fieldValidator(*actualField);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DescriptorValidator CreateDescriptorValidator(bvector<FieldValidator> const& expectedFields)
    {
    return [=](ContentDescriptor const& descriptor)
        {
        ValidateDescriptor(descriptor, expectedFields);
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateHierarchyLevelDescriptor(ECPresentationManagerR manager, AsyncHierarchyRequestParams const& params, DescriptorValidator const& expectedDescriptor)
    {
    auto parentNodeKey = params.GetParentNodeKey() ? params.GetParentNodeKey() : params.GetParentNode() ? params.GetParentNode()->GetKey().get() : nullptr;
    auto descriptorParams = AsyncHierarchyLevelDescriptorRequestParams::Create(HierarchyLevelDescriptorRequestParams(params, parentNodeKey), params);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(manager.GetNodesDescriptor(descriptorParams));
    ASSERT_TRUE(descriptor.IsValid());
    expectedDescriptor(*descriptor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExpectThrowingHierarchyLevelDescriptorRequest(ECPresentationManagerR manager, AsyncHierarchyRequestParams const& params)
    {
    auto parentNodeKey = params.GetParentNodeKey() ? params.GetParentNodeKey() : params.GetParentNode() ? params.GetParentNode()->GetKey().get() : nullptr;
    auto descriptorParams = AsyncHierarchyLevelDescriptorRequestParams::Create(HierarchyLevelDescriptorRequestParams(params, parentNodeKey), params);
    auto future = manager.GetNodesDescriptor(descriptorParams);
    ASSERT_TRUE(future.wait().hasException());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExpectThrowingHierarchyLevelRequest(ECPresentationManagerR manager, AsyncHierarchyRequestParams const& params)
    {
    auto future = manager.GetNodes(params);
    ASSERT_TRUE(future.wait().hasException());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static AsyncHierarchyRequestParams WithParentNode(AsyncHierarchyRequestParams const& in, NavNodeCP parentNode)
    {
    AsyncHierarchyRequestParams out(in);
    out.SetParentNode(parentNode);
    return out;
    }

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
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WhenFilterRequestsSelectClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WhenFilterRequestsSelectClass)
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
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        );

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2", *classA, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WhenFilterRequestsOneOfTheSelectClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WhenFilterRequestsOneOfTheSelectClasses)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(1));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(2));});
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(1));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName(), classB->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("PropA")),
        CreatePropertiesFieldValidator(*classB->GetPropertyP("PropB")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.PropB = 2", *classB, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WhenFilterRequestsDerivedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WhenFilterRequestsDerivedClass)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(1));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(2));});

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
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a }),
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classB->GetPropertyP("PropB")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.PropB = 2", *classB, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b2 }),
        });
    }

#ifdef FIXME_CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_WITH_RELATED_INSTANCES
// https://github.com/iTwin/imodel-native/issues/110
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WithRelatedInstanceSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WithRelatedInstanceSpecification)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(1));});
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(999));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(2));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(666));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);

    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(3));});
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(999));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a3, *b3);

    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(4));});
    IECInstancePtr b4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(666));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a4, *b4);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rootRule = new RootNodeRule();
    auto rootSpec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false,
        "b.PropB = 999",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() }),
        }, {});
    rootSpec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), "b", true));
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a3 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("PropA")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.PropA > 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a3 }),
        });
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WhenFilterUsesRelatedInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WhenFilterUsesRelatedInstances)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(2));});
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

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        );

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("b.PropB = 2", *classA, bvector<RelatedClassPath>
        {
        RelatedClassPath({ RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "r"), true, SelectClass<>(*classB, "b")) }),
        }));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WithExcludedClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithInstanceNodesOfSpecificClassesSpecification_WithExcludedClasses)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(1));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(2));});

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(1));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(2));});

    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(1));});
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("PropA", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() }),
        },
        {
        new MultiSchemaClass(classC->GetSchema().GetName(), true, bvector<Utf8String>{ classC->GetName() }),
        }));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator({ classA->GetPropertyP("PropA"), classB->GetPropertyP("PropA") }),
        CreatePropertiesFieldValidator(*classB->GetPropertyP("PropB")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.PropA = 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        CreateInstanceNodeValidator({ b2 }),
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
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ b1 })
                })),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ b2 })
                })),
        });

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classB->GetPropertyP("Prop")),
        }));

    // verify getting child nodes with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2"));
    ValidateHierarchy(WithParentNode(params, hierarchy[0].node.get()),
        {
        });
    ValidateHierarchy(WithParentNode(params, hierarchy[1].node.get()),
        {
        CreateInstanceNodeValidator({ b2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification_WhenFilterRequestsTargetClass, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification_WhenFilterRequestsTargetClass)
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
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ b1 })
                })),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ b2 })
                })),
        });

    // verify getting child nodes with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2", *classB, bvector<RelatedClassPath>()));
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
DEFINE_SCHEMA(InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification_WhenFilterRequestsOneOfTheTargetClasses, R"*(
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
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification_WhenFilterRequestsOneOfTheTargetClasses)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *c1);
    IECInstancePtr d1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *d1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *c2);
    IECInstancePtr d2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *d2);

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
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()),
            }),
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, classD->GetFullName()),
            })
        }));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ c1 }),
                CreateInstanceNodeValidator({ d1 })
                })),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ c2 }),
                CreateInstanceNodeValidator({ d2 })
                })),
        });

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classD->GetPropertyP("Prop")),
        }));

    // verify getting child nodes with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2", *classD, bvector<RelatedClassPath>()));
    params.SetParentNode(hierarchy[0].node.get());
    ValidateHierarchy(params,
        {
        });
    params.SetParentNode(hierarchy[1].node.get());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ d2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification_WhenFilterRequestsDerivedClass, R"*(
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
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification_WhenFilterRequestsDerivedClass)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *c1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *c2);

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
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ b1 }),
                CreateInstanceNodeValidator({ c1 })
                })),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ b2 }),
                CreateInstanceNodeValidator({ c2 })
                })),
        });

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classC->GetPropertyP("Prop")),
        }));

    // verify getting child nodes with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2", *classC, bvector<RelatedClassPath>()));
    params.SetParentNode(hierarchy[0].node.get());
    ValidateHierarchy(params,
        {
        });
    params.SetParentNode(hierarchy[1].node.get());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ c2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification_WhenFilterUsesRelatedInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
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
    <ECRelationshipClass typeName="BC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="false">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithRelatedInstanceNodesSpecification_WhenFilterUsesRelatedInstances)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);

    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a3, *b3);
    IECInstancePtr c31 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b3, *c31);
    IECInstancePtr c32 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b3, *c32);

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
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ b1 }),
                })),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ b2 }),
                })),
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a3 }),
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ b3 }),
                })),
        });

    // verify getting child nodes with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("c.Prop = 2", *classB, bvector<RelatedClassPath>
        {
        RelatedClassPath{ RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "r"), true, SelectClass<>(*classC, "c")) },
        }));
    params.SetParentNode(hierarchy[0].node.get());
    ValidateHierarchy(params,
        {
        });
    params.SetParentNode(hierarchy[1].node.get());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b2 }),
        });
    params.SetParentNode(hierarchy[2].node.get());
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b3 }),
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
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification_WhenFilterRequestsSelectClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification_WhenFilterRequestsSelectClass)
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
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        );

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2", *classA, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification_WhenFilterRequestsOneOfTheSelectClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification_WhenFilterRequestsOneOfTheSelectClasses)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(1));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    SearchResultInstanceNodesSpecification* rootSpec = new SearchResultInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false);
    rootSpec->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("select * from %s", classA->GetECSqlName().c_str()),
        classA->GetSchema().GetName(), classA->GetName()));
    rootSpec->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("select * from %s", classB->GetECSqlName().c_str()),
        classB->GetSchema().GetName(), classB->GetName()));
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a }),
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("PropA")),
        CreatePropertiesFieldValidator(*classB->GetPropertyP("PropB")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.PropB = 2", *classB, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification_WhenFilterRequestsDerivedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification_WhenFilterRequestsDerivedClass)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

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
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a }),
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classB->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2", *classB, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification_WhenFilterUsesRelatedInstances, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithSearchResultInstanceNodesSpecification_WhenFilterUsesRelatedInstances)
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
    SearchResultInstanceNodesSpecification* rootSpec = new SearchResultInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false);
    rootSpec->AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("select * from %s", classA->GetECSqlName().c_str()),
        classA->GetSchema().GetName(), classA->GetName()));
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        );

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("b.Prop = 2", *classA, bvector<RelatedClassPath>
        {
        RelatedClassPath{ RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "r"), true, SelectClass<>(*classB, "b")) },
        }));
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
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a1, a2 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a1 }),
                    CreateInstanceNodeValidator({ a2 })
                    })),
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ b })
                    })),
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classC, false, { c }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ c })
                    })),
            })
        );

    // validate hierarchy level filter descriptor (same for all hierarchy levels here)
    auto expectedDescriptor = CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator({ classA->GetPropertyP("Prop"), classB->GetPropertyP("Prop"), classC->GetPropertyP("Prop") }),
        });
    ValidateHierarchyLevelDescriptor(*m_manager, params, expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[1].node.get()), expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[2].node.get()), expectedDescriptor);

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop > 1"));
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
DEFINE_SCHEMA(InstanceFiltering_FiltersClassGroupingNodes_WhenFilterSpecifiesSelectClass, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersClassGroupingNodes_WhenFilterSpecifiesSelectClass)
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
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, false, { a1, a2 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a1 }),
                    CreateInstanceNodeValidator({ a2 })
                    })),
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classB, false, { b }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ b })
                    })),
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classC, false, { c }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ c })
                    })),
            })
        );

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop > 1", *classC, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
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
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { a11, a12, a13 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a11 }),
                    CreateInstanceNodeValidator({ a12 }),
                    CreateInstanceNodeValidator({ a13 }),
                    })),
            ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("2", { a21, a22 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a21 }),
                    CreateInstanceNodeValidator({ a22 }),
                    })),
            CreateInstanceNodeValidator({ a3 }),
            })
        );

    // validate hierarchy level filter descriptor (same for all hierarchy levels here)
    auto expectedDescriptor = CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("LabelProp")),
        CreatePropertiesFieldValidator(*classA->GetPropertyP("FilterProp")),
        });
    ValidateHierarchyLevelDescriptor(*m_manager, params, expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[1].node.get()), expectedDescriptor);

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.FilterProp = 1"));
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
DEFINE_SCHEMA(InstanceFiltering_FiltersLabelGroupingNodes_WhenFilterSpecifiesSelectClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="LabelProp" typeName="string" />
        <ECProperty propertyName="FilterProp" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="LabelProp" typeName="string" />
        <ECProperty propertyName="FilterProp" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersLabelGroupingNodes_WhenFilterSpecifiesSelectClass)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr a12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr a13 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(2));});

    IECInstancePtr b11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr b12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr b13 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(2));});

    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr a22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(2));});

    IECInstancePtr b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(2));});

    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("3")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("3")); instance.SetValue("FilterProp", ECValue(1));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("LabelProp") }));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("LabelProp") }));

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, true, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName(), classB->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { a11, a12, a13, b11, b12, b13 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a11 }),
                    CreateInstanceNodeValidator({ a12 }),
                    CreateInstanceNodeValidator({ a13 }),
                    CreateInstanceNodeValidator({ b11 }),
                    CreateInstanceNodeValidator({ b12 }),
                    CreateInstanceNodeValidator({ b13 }),
                    })),
            ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("2", { a21, a22, b21, b22 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a21 }),
                    CreateInstanceNodeValidator({ a22 }),
                    CreateInstanceNodeValidator({ b21 }),
                    CreateInstanceNodeValidator({ b22 }),
                    })),
            ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("3", { a3, b3 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a3 }),
                    CreateInstanceNodeValidator({ b3 }),
                    })),
            })
        );

    // validate hierarchy level filter descriptor (same for all hierarchy levels here)
    auto expectedDescriptor = CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator({ classA->GetPropertyP("LabelProp"), classB->GetPropertyP("LabelProp") }),
        CreatePropertiesFieldValidator({ classA->GetPropertyP("FilterProp"), classB->GetPropertyP("FilterProp") }),
        });
    ValidateHierarchyLevelDescriptor(*m_manager, params, expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[1].node.get()), expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[2].node.get()), expectedDescriptor);

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.FilterProp = 1", *classB, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateLabelGroupingNodeValidator("1", { b11, b12 }),
            {
            CreateInstanceNodeValidator({ b11 }),
            CreateInstanceNodeValidator({ b12 }),
            }),
        CreateInstanceNodeValidator({ b21 }),
        CreateInstanceNodeValidator({ b3 }),
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
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, true, { b1, b2 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ b1 }),
                    CreateInstanceNodeValidator({ b2 }),
                    })),
            })
        );

    // validate hierarchy level filter descriptor (same for all hierarchy levels here)
    auto expectedDescriptor = CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classB->GetPropertyP("Prop")),
        });
    ValidateHierarchyLevelDescriptor(*m_manager, params, expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), expectedDescriptor);

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1"));
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
DEFINE_SCHEMA(InstanceFiltering_FiltersBaseClassGroupingNodes_WhenFilterSpecifiesSelectClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersBaseClassGroupingNodes_WhenFilterSpecifiesSelectClass)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(1));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropB", ECValue(2));});
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("PropC", ECValue(1));});
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("PropC", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, "", ""));
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classB->GetSchema().GetName(), true, bvector<Utf8String>{ classB->GetName(), classC->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, true, { b1, b2, c1, c2 }),
                {
                CreateInstanceNodeValidator({ b1 }),
                CreateInstanceNodeValidator({ b2 }),
                CreateInstanceNodeValidator({ c1 }),
                CreateInstanceNodeValidator({ c2 }),
                }),
            })
        );

    // validate hierarchy level filter descriptor (same for all hierarchy levels here)
    auto expectedDescriptor = CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classB->GetPropertyP("PropB")),
        CreatePropertiesFieldValidator(*classC->GetPropertyP("PropC")),
        });
    ValidateHierarchyLevelDescriptor(*m_manager, params, expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), expectedDescriptor);

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.PropC = 1", *classC, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateClassGroupingNodeValidator(*classA, true, { c1 }),
            {
            CreateInstanceNodeValidator({ c1 }),
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersPropertyGroupingNodes_WhenFilteringByGroupingProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersPropertyGroupingNodes_WhenFilteringByGroupingProperty)
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
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a1 }, ValueList{ ECValue(1) }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a1 }),
                    })),
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a2 }, ValueList{ ECValue(2) }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a2 }),
                    })),
            })
        );

    // validate hierarchy level filter descriptor (same for all hierarchy levels here)
    auto expectedDescriptor = CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("Prop")),
        });
    ValidateHierarchyLevelDescriptor(*m_manager, params, expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[1].node.get()), expectedDescriptor);

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1"));
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
DEFINE_SCHEMA(InstanceFiltering_FiltersPropertyGroupingNodes_WhenFilteringByGroupingProperty_AndFilterSpecifiesSelectClass, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersPropertyGroupingNodes_WhenFilteringByGroupingProperty_AndFilterSpecifiesSelectClass)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Prop"));
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ b1, c1 }, ValueList{ ECValue(1) }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ b1 }),
                    CreateInstanceNodeValidator({ c1 }),
                    })),
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ b2, c2 }, ValueList{ ECValue(2) }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ b2 }),
                    CreateInstanceNodeValidator({ c2 }),
                    })),
            })
        );

    // validate hierarchy level filter descriptor (same for all hierarchy levels here)
    auto expectedDescriptor = CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator({ classB->GetPropertyP("Prop"), classC->GetPropertyP("Prop") }),
        });
    ValidateHierarchyLevelDescriptor(*m_manager, params, expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[1].node.get()), expectedDescriptor);

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1", *classC, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ c1 }, ValueList{ ECValue(1) }),
            {
            CreateInstanceNodeValidator({ c1 }),
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersPropertyGroupingNodes_WhenFilteringByNonGroupingProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="GroupingProp" typeName="int" />
        <ECProperty propertyName="FilteringProp" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersPropertyGroupingNodes_WhenFilteringByNonGroupingProperty)
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
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a1 }, ValueList{ ECValue(111) }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a1 }),
                    })),
            ExpectedHierarchyDef(CreatePropertyGroupingNodeValidator({ a2, a3 }, ValueList{ ECValue(222) }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ a2 }),
                    CreateInstanceNodeValidator({ a3 }),
                    })),
            })
        );

    // validate hierarchy level filter descriptor (same for all hierarchy levels here)
    auto expectedDescriptor = CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("GroupingProp")),
        CreatePropertiesFieldValidator(*classA->GetPropertyP("FilteringProp")),
        });
    ValidateHierarchyLevelDescriptor(*m_manager, params, expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[0].node.get()), expectedDescriptor);
    ValidateHierarchyLevelDescriptor(*m_manager, WithParentNode(params, hierarchy[1].node.get()), expectedDescriptor);

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.FilteringProp = 888"));
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
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a11, a12, a13 }),
            CreateInstanceNodeValidator({ a21, a22 }),
            CreateInstanceNodeValidator({ a3 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("LabelProp")),
        CreatePropertiesFieldValidator(*classA->GetPropertyP("FilterProp")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.FilterProp = 1"));
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
DEFINE_SCHEMA(InstanceFiltering_FiltersSameLabelInstanceNodes_WhenFilterSpecifiesSelectClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="LabelProp" typeName="string" />
        <ECProperty propertyName="FilterProp" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersSameLabelInstanceNodes_WhenFilterSpecifiesSelectClass)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr b11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr b12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr b13 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(2));});
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("1")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(2));});
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("2")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("3")); instance.SetValue("FilterProp", ECValue(1));});
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("LabelProp", ECValue("3")); instance.SetValue("FilterProp", ECValue(1));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("LabelProp") }));

    auto groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    rules->AddPresentationRule(*groupingRule);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ b11, b12, b13, c1 }),
            CreateInstanceNodeValidator({ b21, b22, c2 }),
            CreateInstanceNodeValidator({ b3, c3 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator({ classB->GetPropertyP("LabelProp"), classC->GetPropertyP("LabelProp") }),
        CreatePropertiesFieldValidator({ classB->GetPropertyP("FilterProp"), classC->GetPropertyP("FilterProp") }),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.FilterProp = 1", *classB, bvector<RelatedClassPath>()));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b11, b12 }),
        CreateInstanceNodeValidator({ b21 }),
        CreateInstanceNodeValidator({ b3 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideNodesInHierarchy_WithCustomNodeSpecificationInParentLevelAndInstanceNodesOfSpecificClassesInChildLevel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideNodesInHierarchy_WithCustomNodeSpecificationInParentLevelAndInstanceNodesOfSpecificClassesInChildLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2)); });

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    auto rootSpec = new CustomNodeSpecification(1, false, "T_CUSTOM", "TestLabel", "", "");
    rootSpec->SetHideNodesInHierarchy(true);
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule("ParentNode.Type = \"T_CUSTOM\"", 1, false);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ a2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideNodesInHierarchy_WithInstanceNodesOfSpecificClassesInParentAndChildLevels, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideNodesInHierarchy_WithInstanceNodesOfSpecificClassesInParentAndChildLevels)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

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
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classB->GetSchema().GetName(), true, bvector<Utf8String>{ classB->GetName() })
        }, {}));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classB->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideNodesInHierarchy_WithInstanceNodesOfSpecificClassesInParentLevelAndRelatedInstanceNodesInChildLevel, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideNodesInHierarchy_WithInstanceNodesOfSpecificClassesInParentLevelAndRelatedInstanceNodesInChildLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(3)); });
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
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            CreateInstanceNodeValidator({ b3 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classB->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop > 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b3 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideNodesInHierarchy_WithRelatedInstanceNodesInParentLevelAndInstanceNodesOfSpecificClassesInChildLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideNodesInHierarchy_WithRelatedInstanceNodesInParentLevelAndInstanceNodesOfSpecificClassesInChildLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);

    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRuleB = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRuleB->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleB);

    ChildNodeRule* childRuleC = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    childRuleC->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classC->GetSchema().GetName(), true, bvector<Utf8String>{ classC->GetName() })
        }, {}));
    rules->AddPresentationRule(*childRuleC);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ c1 }),
            CreateInstanceNodeValidator({ c2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classC->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ c2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideNodesInHierarchy_WithRelatedInstanceNodesInParentAndChildLevels, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
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
    <ECRelationshipClass typeName="BC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="false">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideNodesInHierarchy_WithRelatedInstanceNodesInParentAndChildLevels)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    IECInstancePtr c11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c11);
    IECInstancePtr c12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c12);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    IECInstancePtr c21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c21);
    IECInstancePtr c22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c22);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRuleB = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRuleB->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleB);

    ChildNodeRule* childRuleC = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classB->GetName().c_str(), classB->GetSchema().GetName().c_str()), 1, false);
    childRuleC->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward) })
        }));
    rules->AddPresentationRule(*childRuleC);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ c11 }),
            CreateInstanceNodeValidator({ c12 }),
            CreateInstanceNodeValidator({ c21 }),
            CreateInstanceNodeValidator({ c22 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classC->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ c12 }),
        CreateInstanceNodeValidator({ c22 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideNodesInHierarchy_WithSearchResultInstanceNodesInParentLevelAndInstanceNodesOfSpecificClassesInChildLevel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideNodesInHierarchy_WithSearchResultInstanceNodesInParentLevelAndInstanceNodesOfSpecificClassesInChildLevel)
    {
    // dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2)); });

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rootRule = new RootNodeRule();
    auto rootSpec = new SearchResultInstanceNodesSpecification(1, ChildrenHint::Unknown, true, false, false, false);
    rootSpec->AddQuerySpecification(*new StringQuerySpecification(
        Utf8PrintfString("select * from %s", classA->GetECSqlName().c_str()),
        classA->GetSchema().GetName(),
        classA->GetName()
        ));
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classB->GetSchema().GetName(), true, bvector<Utf8String>{ classB->GetName() })
        }, {}));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classB->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 2"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b2 }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithHideNodesInHierarchy_WhenMultipleSpecsAreUsedForChildren, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithHideNodesInHierarchy_WhenMultipleSpecsAreUsedForChildren)
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
        ExpectedHierarchyListDef(true,
            {
            CreateInstanceNodeValidator({ b1 }),
            CreateInstanceNodeValidator({ b2 }),
            CreateInstanceNodeValidator({ b3 }),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classB->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 15"));
    ValidateHierarchy(params,
        {
        CreateInstanceNodeValidator({ b3 }),
        });
    }

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
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ b1 }),
                    })),
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
                ExpectedHierarchyListDef(true,
                    {
                    CreateInstanceNodeValidator({ b2 }),
                    })),
            })
        );

    // validate hierarchy level filter descriptor
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1"));
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            {
            CreateInstanceNodeValidator({ b1 }),
            }),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_FiltersWithRecursiveRules, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_FiltersWithRecursiveRules)
    {
    // dataset
    ECClassCP classA = GetClass("A");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(2));});

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    rootRule->AddSpecification(*new CustomNodeSpecification(1, false, "T_ROOT", "Root", "", ""));
    rules->AddPresentationRule(*rootRule);

    ChildNodeRule* childRule = new ChildNodeRule(Utf8PrintfString("ParentNode.Type = \"T_ROOT\" OR ParentNode.IsOfClass(\"%s\", \"%s\")", classA->GetName().c_str(), classA->GetSchema().GetName().c_str()), 1, false);
    childRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {}));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateCustomNodeValidator("T_ROOT", "Root"),
                ExpectedHierarchyListDef(true,
                    {
                    ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
                        ExpectedHierarchyListDef(true,
                            {
                            CreateInstanceNodeValidator({ a1 }),
                            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
                                ExpectedHierarchyListDef(true,
                                    {
                                    CreateInstanceNodeValidator({ a1 }),
                                    CreateInstanceNodeValidator({ a2 }),
                                    })),
                            })),
                    ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
                        ExpectedHierarchyListDef(true,
                            {
                            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
                                ExpectedHierarchyListDef(true,
                                    {
                                    CreateInstanceNodeValidator({ a1 }),
                                    CreateInstanceNodeValidator({ a2 }),
                                    })),
                            CreateInstanceNodeValidator({ a2 }),
                            })),
                    })),
            }));

    // validate T_ROOT's hierarchy level filter descriptor
    params.SetParentNode(hierarchy[0].node.get());
    ValidateHierarchyLevelDescriptor(*m_manager, params, CreateDescriptorValidator(
        {
        CreatePropertiesFieldValidator(*classA->GetPropertyP("Prop")),
        }));

    // verify with instance filter
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1"));
    ValidateHierarchy(params,
        {
        ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
            ExpectedHierarchyListDef(true,
                {
                CreateInstanceNodeValidator({ a1 }),
                ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
                    ExpectedHierarchyListDef(true,
                        {
                        CreateInstanceNodeValidator({ a1 }),
                        CreateInstanceNodeValidator({ a2 }),
                        })),
                })),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromSpecificationsWithHideExpression, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromSpecificationsWithHideExpression)
    {
    // dataset
    ECClassCP classA = GetClass("A");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* rootRule = new RootNodeRule();
    InstanceNodesOfSpecificClassesSpecification* rootSpec = new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new MultiSchemaClass(classA->GetSchema().GetName(), true, bvector<Utf8String>{ classA->GetName() })
        }, {});
    rootSpec->SetHideExpression("FALSE");
    rootRule->AddSpecification(*rootSpec);
    rules->AddPresentationRule(*rootRule);

    // verify without instance filter, ensure the root hierarchy level is not filterable
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    ValidateHierarchy(params,
        ExpectedHierarchyListDef(false,
            {
            CreateInstanceNodeValidator({ a1 }),
            CreateInstanceNodeValidator({ a2 }),
            })
        );

    // getting descriptor for the root hierarchy level should throw
    ExpectThrowingHierarchyLevelDescriptorRequest(*m_manager, params);

    // attempting to filter the hierarchy level should throw
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1"));
    ExpectThrowingHierarchyLevelRequest(*m_manager, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromRelatedInstanceNodesSpecificationWithDeprecatedSkipRelatedLevel, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromRelatedInstanceNodesSpecificationWithDeprecatedSkipRelatedLevel)
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
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 1, "",
        RequiredRelationDirection_Forward, "", "", ""));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter, ensure the child hierarchy levels are not filterable
    // note: the A nodes have "has children = true" flag, but have no children - that needs additional investigation which is outside the scope of this test
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }), true,
                ExpectedHierarchyListDef(false,
                    {
                    })),
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }), true,
                ExpectedHierarchyListDef(false,
                    {
                    })),
            })
        );

    // getting descriptor for the child hierarchy levels should throw
    ExpectThrowingHierarchyLevelDescriptorRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    ExpectThrowingHierarchyLevelDescriptorRequest(*m_manager, WithParentNode(params, hierarchy[1].node.get()));

    // attempting to filter the hierarchy level should throw
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1"));
    ExpectThrowingHierarchyLevelRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    ExpectThrowingHierarchyLevelRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromRelatedInstanceNodesSpecificationWithDeprecatedSupportedSchemas, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromRelatedInstanceNodesSpecificationWithDeprecatedSupportedSchemas)
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
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection_Both, classA->GetSchema().GetName(), "", ""));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter, ensure the child hierarchy levels are not filterable
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
                ExpectedHierarchyListDef(false,
                    {
                    CreateInstanceNodeValidator({ b1 }),
                    })),
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
                ExpectedHierarchyListDef(false,
                    {
                    CreateInstanceNodeValidator({ b2 }),
                    })),
            })
        );

    // getting descriptor for the root hierarchy level should throw
    ExpectThrowingHierarchyLevelDescriptorRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    ExpectThrowingHierarchyLevelDescriptorRequest(*m_manager, WithParentNode(params, hierarchy[1].node.get()));

    // attempting to filter the hierarchy level should throw
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1"));
    ExpectThrowingHierarchyLevelRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    ExpectThrowingHierarchyLevelRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromRelatedInstanceNodesSpecificationWithDeprecatedRelationshipClassNames, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromRelatedInstanceNodesSpecificationWithDeprecatedRelationshipClassNames)
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
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", relAB->GetFullName(), ""));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter, ensure the child hierarchy levels are not filterable
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
                ExpectedHierarchyListDef(false,
                    {
                    CreateInstanceNodeValidator({ b1 }),
                    })),
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
                ExpectedHierarchyListDef(false,
                    {
                    CreateInstanceNodeValidator({ b2 }),
                    })),
            })
        );

    // getting descriptor for the root hierarchy level should throw
    ExpectThrowingHierarchyLevelDescriptorRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    ExpectThrowingHierarchyLevelDescriptorRequest(*m_manager, WithParentNode(params, hierarchy[1].node.get()));

    // attempting to filter the hierarchy level should throw
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1"));
    ExpectThrowingHierarchyLevelRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    ExpectThrowingHierarchyLevelRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromRelatedInstanceNodesSpecificationWithDeprecatedRelatedClassNames, R"*(
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
TEST_F(RulesDrivenECPresentationManagerNavigationTests, InstanceFiltering_DoesntSupportFilteringHierarchyLevelsFromRelatedInstanceNodesSpecificationWithDeprecatedRelatedClassNames)
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
    childRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, ChildrenHint::Unknown, false, false, false, false, 0, "",
        RequiredRelationDirection_Forward, "", "", classB->GetFullName()));
    rules->AddPresentationRule(*childRule);

    // verify without instance filter, ensure the child hierarchy levels are not filterable
    auto params = AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables());
    auto hierarchy = ValidateHierarchy(params,
        ExpectedHierarchyListDef(true,
            {
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a1 }),
                ExpectedHierarchyListDef(false,
                    {
                    CreateInstanceNodeValidator({ b1 }),
                    })),
            ExpectedHierarchyDef(CreateInstanceNodeValidator({ a2 }),
                ExpectedHierarchyListDef(false,
                    {
                    CreateInstanceNodeValidator({ b2 }),
                    })),
            })
        );

    // getting descriptor for the root hierarchy level should throw
    ExpectThrowingHierarchyLevelDescriptorRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    ExpectThrowingHierarchyLevelDescriptorRequest(*m_manager, WithParentNode(params, hierarchy[1].node.get()));

    // attempting to filter the hierarchy level should throw
    params.SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("this.Prop = 1"));
    ExpectThrowingHierarchyLevelRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    ExpectThrowingHierarchyLevelRequest(*m_manager, WithParentNode(params, hierarchy[0].node.get()));
    }
