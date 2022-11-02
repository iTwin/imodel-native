/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TContainer1, typename TContainer2>
static bool ContainsAll(TContainer1 const& container, TContainer2 const& check, bool compareSizes)
    {
    if (compareSizes && container.size() != check.size())
        return false;
    for (auto const& value : check)
        {
        if (!ContainerHelpers::Contains(container, value))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static RelatedPropertiesSpecification* CreateSelectAllPolymorphicallyRelatedPropertiesSpecification(Utf8String relationshipFullName, RequiredRelationDirection direction)
    {
    auto step = new RelationshipStepSpecification(relationshipFullName, direction);
    return new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*step), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_ReturnsValidDescriptorBasedOnSelectedClassesWhenJoiningPolymorphicallyRelatedProperties, R"*(
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect">
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ReturnsValidDescriptorBasedOnSelectedClassesWhenJoiningPolymorphicallyRelatedProperties)
    {
    ECEntityClassCP elementClass = GetClass("Element")->GetEntityClassCP();
    ECEntityClassCP aspectClass = GetClass("ElementUniqueAspect")->GetEntityClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();

    // set up dataset
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element, *aspect);

    // set up selection
    KeySetPtr input = KeySet::Create({elementClass});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
    rule->GetSpecifications().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        elementOwnsUniqueAspectRelationship->GetFullName(), aspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(elementClass, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields.front()->IsNestedContentField());
    EXPECT_STREQ(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass), fields.front()->GetUniqueName().c_str());
    ContentDescriptor::NestedContentField const* field = fields.front()->AsNestedContentField();
    ASSERT_EQ(1, field->GetFields().size());
    EXPECT_STREQ(FIELD_NAME(aspectClass, "AspectName"), field->GetFields().front()->GetUniqueName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedPropertyValuesAreCorrectWhenSelectionIncludesInstanceOfRelatedInstanceClass, R"*(
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A" >
        <ECProperty propertyName="Property2" typeName="string" />
        <ECProperty propertyName="Property1" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="B_A"  strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ba" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, RelatedPropertyValuesAreCorrectWhenSelectionIncludesInstanceOfRelatedInstanceClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    ECRelationshipClassCP relBA = GetClass("B_A")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property1", ECValue("Test A"));});
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property2", ECValue("Test B 1"));});
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property2", ECValue("Test B 2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBA, *instanceB1, *instanceA);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB2});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, relBA->GetFullName(), classB->GetFullName(), "Property2", RelationshipMeaning::RelatedInstance));
    spec->AddPropertyOverride(*new PropertySpecification("Property1", 1000, "", nullptr, true));
    spec->AddPropertyOverride(*new PropertySpecification("Property2", 1000, "", nullptr, true));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(3, descriptor->GetVisibleFields().size()); // A.Property1, related B.Property2, B.Property2
    ASSERT_EQ(descriptor->GetVisibleFields()[2]->AsNestedContentField()->AsRelatedContentField()->GetRelationshipMeaning(), RelationshipMeaning::RelatedInstance);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": "Test A",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "Test B 1"
                },
            "DisplayValues": {
                "%s": "Test B 1"
                },
            "MergedFieldNames": []
            }]
    })",
        descriptor->GetVisibleFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[1]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[2]->GetUniqueName().c_str(),
        classB->GetId().ToString().c_str(), instanceB1->GetInstanceId().c_str(),
        descriptor->GetVisibleFields()[2]->AsNestedContentField()->AsRelatedContentField()->GetFields().front()->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[2]->AsNestedContentField()->AsRelatedContentField()->GetFields().front()->GetUniqueName().c_str()
    ).c_str());
    EXPECT_EQ(expectedValues1, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": "Test B 2",
        "%s": null,
        "%s": []
    })",
        descriptor->GetVisibleFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[1]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[2]->GetUniqueName().c_str(),
        classB->GetId().ToString().c_str(), instanceB1->GetInstanceId().c_str(),
        descriptor->GetVisibleFields()[2]->AsNestedContentField()->AsRelatedContentField()->GetFields().front()->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[2]->AsNestedContentField()->AsRelatedContentField()->GetFields().front()->GetUniqueName().c_str()
    ).c_str());
    EXPECT_EQ(expectedValues2, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentModifierAppliesRelatedPropertiesSpecification, R"*(
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property2" typeName="string" />
        <ECProperty propertyName="Property1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A" >
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
        <ECNavigationProperty propertyName="B" relationshipName="B_A" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="B_A"  strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ba" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentModifierAppliesRelatedPropertiesSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relBA = GetClass("B_A")->GetRelationshipClassCP();

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property1", ECValue("InstanceA"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property1", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBA, *instanceB, *instanceA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "A");
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, relBA->GetFullName(),
        classB->GetFullName(), "Property1", RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // A.Property1, A.Property2, A.B, B.Property1

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "InstanceA",
        "%s": null,
        "%s": {"ECClassId": "%s", "ECInstanceId": "%s"},
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "InstanceB"
                },
            "DisplayValues": {
                "%s": "InstanceB"
                },
            "MergedFieldNames": []
            }]
    })",
        descriptor->GetVisibleFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[1]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[2]->GetUniqueName().c_str(), instanceB->GetClass().GetId().ToString().c_str(), instanceB->GetInstanceId().c_str(),
        descriptor->GetVisibleFields()[3]->GetUniqueName().c_str(),
        classB->GetId().ToString().c_str(), instanceB->GetInstanceId().c_str(),
        descriptor->GetVisibleFields()[3]->AsNestedContentField()->AsRelatedContentField()->GetFields().front()->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[3]->AsNestedContentField()->AsRelatedContentField()->GetFields().front()->GetUniqueName().c_str()
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotLoadRelatedPropertyWhenModifierWithHigherPriorityHasNoProperties, R"*(
    <ECEntityClass typeName="Element" />
    <ECEntityClass typeName="Aspect">
        <ECProperty propertyName="MyID" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesNotLoadRelatedPropertyWhenModifierWithHigherPriorityHasNoProperties)
    {
    // set up data set
    ECRelationshipClassCP elementOwnsAspects = GetClass("ElementOwnsAspects")->GetRelationshipClassCP();
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("AspectID"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsAspects, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier1 = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier1->SetPriority(2);
    rules->AddPresentationRule(*modifier1);
    modifier1->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(elementOwnsAspects->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance));

    ContentModifierP modifier2 = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier2->SetPriority(1);
    rules->AddPresentationRule(*modifier2);
    modifier2->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(elementOwnsAspects->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("MyID") }, RelationshipMeaning::RelatedInstance, false, false, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(0, descriptor->GetVisibleFields().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedPropertiesWithoutInstanceFilterAndHigherPriorityOverridesRelatedPropertiesWithInstanceFilter, R"*(
    <ECEntityClass typeName="Element" />
    <ECEntityClass typeName="Aspect">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, RelatedPropertiesWithoutInstanceFilterAndHigherPriorityOverridesRelatedPropertiesWithInstanceFilter)
    {
    // set up data set
    ECRelationshipClassCP elementOwnsAspects = GetClass("ElementOwnsAspects")->GetRelationshipClassCP();
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("Prop", ECValue(10));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsAspects, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier1 = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier1->SetPriority(2);
    rules->AddPresentationRule(*modifier1);
    modifier1->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(elementOwnsAspects->GetFullName(), RequiredRelationDirection_Forward),
        }), { }, RelationshipMeaning::RelatedInstance));

    ContentModifierP modifier2 = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier2->SetPriority(1);
    rules->AddPresentationRule(*modifier2);
    RelatedPropertiesSpecificationP relatedPropertySpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(elementOwnsAspects->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop") }, RelationshipMeaning::RelatedInstance, false, false, true);
    relatedPropertySpec->SetInstanceFilter("this.Prop = 10");
    modifier2->AddRelatedProperty(*relatedPropertySpec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    auto fields = descriptor->GetVisibleFields();
    EXPECT_EQ(0, descriptor->GetVisibleFields().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesSeparateFieldsWithDifferentIntermediateClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B1">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesSeparateFieldsWithDifferentIntermediateClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classB1 = GetClass("B1");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance)
        {
        instance.SetValue("Prop", ECValue("value1"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance)
        {
        instance.SetValue("Prop", ECValue("value2"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relAB->GetFullName(),
        classB->GetFullName(), "*", RelationshipMeaning::SameInstance, true));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relBC->GetFullName(),
        classC->GetFullName(), "*", RelationshipMeaning::SameInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // A_B1, A_B2

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": "value1"
                        },
                    "DisplayValues": {
                        "%s": "value1"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": "value1"
                        }
                    }]
                },
            "MergedFieldNames": []
            }],
        "%s": []
    })",
        descriptor->GetVisibleFields()[0]->GetUniqueName().c_str(),
        b1->GetClass().GetId().ToString().c_str(), b1->GetInstanceId().c_str(),
        descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str(),
        c1->GetClass().GetId().ToString().c_str(), c1->GetInstanceId().c_str(),
        descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[1]->GetUniqueName().c_str()
    ).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": "value2"
                        },
                    "DisplayValues": {
                        "%s": "value2"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": "value2"
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
    })",
        descriptor->GetVisibleFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[1]->GetUniqueName().c_str(),
        b2->GetClass().GetId().ToString().c_str(), b2->GetInstanceId().c_str(),
        descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str(),
        c2->GetClass().GetId().ToString().c_str(), c2->GetInstanceId().c_str(),
        descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str(),
        descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str()
    ).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotIncludeEmptyNestedFields, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D" />
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_D" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="cd" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="dc" polymorphic="true">
            <Class class="D"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesNotIncludeEmptyNestedFields)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relCD = GetClass("C_D")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c, *d);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relAB->GetFullName(),
        classB->GetFullName(), "*", RelationshipMeaning::SameInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relBC->GetFullName(),
        classC->GetFullName(), "*", RelationshipMeaning::SameInstance));
    modifier->GetRelatedProperties().back()->GetNestedRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        relCD->GetFullName(), classD->GetFullName(), "*", RelationshipMeaning::SameInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(0, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetCategories().size()); // default category
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesNestedContentWhenBaseIntermediateClassDoesntHaveRelationshipToTarget, R"*(
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
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_D" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="bd" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="db" polymorphic="true">
            <Class class="D"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesNestedContentWhenBaseIntermediateClassDoesntHaveRelationshipToTarget)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBD = GetClass("B_D")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBD, *c, *d);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBD->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    auto fields = descriptor->GetVisibleFields();
    EXPECT_EQ(1, fields.size());
    EXPECT_STREQ(classB->GetDisplayLabel().c_str(), fields[0]->GetLabel().c_str());
    EXPECT_EQ(1, fields[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ(classD->GetDisplayLabel().c_str(), fields[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_EQ(1, fields[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("Prop", fields[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    ValidateFieldCategoriesHierarchy(*fields[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classD->GetName(),
        classB->GetName()
        });

    // validate content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}],
                    "Values": {
                        "%s": null
                    },
                    "DisplayValues": {
                        "%s": null
                    },
                    "MergedFieldNames": []
                }]
            },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": null
                    }
                }]
            },
            "MergedFieldNames": []
        }]
    })",
        NESTED_CONTENT_FIELD_NAME(classA, classB),
        classC->GetId().ToString().c_str(), c->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(classC, classD),
        classD->GetId().ToString().c_str(), d->GetInstanceId().c_str(),
        FIELD_NAME(classD, "Prop"), FIELD_NAME(classD, "Prop"),
        NESTED_CONTENT_FIELD_NAME(classC, classD),
        FIELD_NAME(classD, "Prop")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesNestedContentWhenBaseIntermediateClassDoesntHaveRelationshipToAnotherIntermediateClass, R"*(
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
    <ECEntityClass typeName="D" />
    <ECEntityClass typeName="E">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_D" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="bd" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="db" polymorphic="true">
            <Class class="D"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="D_E" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="de" polymorphic="true">
            <Class class="D"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ed" polymorphic="true">
            <Class class="E"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesNestedContentWhenBaseIntermediateClassDoesntHaveRelationshipToAnotherIntermediateClass)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECClassCP classE = GetClass("E");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBD = GetClass("B_D")->GetRelationshipClassCP();
    ECRelationshipClassCP relDE = GetClass("D_E")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBD, *c, *d);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relDE, *d, *e);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBD->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->GetNestedRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relDE->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    auto fields = descriptor->GetVisibleFields();
    EXPECT_EQ(1, fields.size());
    EXPECT_STREQ(classB->GetDisplayLabel().c_str(), fields[0]->GetLabel().c_str());
    EXPECT_EQ(1, fields[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ(classD->GetDisplayLabel().c_str(), fields[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_EQ(1, fields[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ(classE->GetDisplayLabel().c_str(), fields[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_EQ(1, fields[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("Prop", fields[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    ValidateFieldCategoriesHierarchy(*fields[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classE->GetName(),
        classD->GetName(),
        classB->GetName()
        });

    // validate content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}],
                    "Values": {
                        "%s": [{
                            "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}],
                            "Values": {
                                "%s": null
                            },
                            "DisplayValues": {
                                "%s": null
                            },
                            "MergedFieldNames": []
                        }]
                    },
                    "DisplayValues": {
                        "%s": [{
                            "DisplayValues": {
                                "%s": null
                            }
                        }]
                    },
                    "MergedFieldNames": []
                }]
            },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": [{
                            "DisplayValues": {
                                "%s": null
                            }
                        }]
                    }
                }]
            },
            "MergedFieldNames": []
        }]
    })",
        NESTED_CONTENT_FIELD_NAME(classA, classB),
        classC->GetId().ToString().c_str(), c->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(classC, classD),
        classD->GetId().ToString().c_str(), d->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(classD, classE),
        classE->GetId().ToString().c_str(), e->GetInstanceId().c_str(),
        FIELD_NAME(classE, "Prop"), FIELD_NAME(classE, "Prop"),
        NESTED_CONTENT_FIELD_NAME(classD, classE),
        FIELD_NAME(classE, "Prop"),
        NESTED_CONTENT_FIELD_NAME(classC, classD),
        NESTED_CONTENT_FIELD_NAME(classD, classE),
        FIELD_NAME(classE, "Prop")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToManyRelatedInstancesAsArrays, R"*(
    <ECEntityClass typeName="ChildClass1">
        <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildren1" direction="Backward" />
    </ECEntityClass>
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="ChildClass2">
        <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildren2" direction="Backward" />
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="ParentClass">
        <ECProperty propertyName="ParentProperty" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ParentHasChildren1" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="ClassD Has ClassE" polymorphic="False">
            <Class class="ParentClass" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ClassE Has ClassD" polymorphic="False">
            <Class class="ChildClass1" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ParentHasChildren2" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="ClassD Has ClassE" polymorphic="False">
            <Class class="ParentClass" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ClassE Has ClassD" polymorphic="False">
            <Class class="ChildClass2" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedInstancesAsArrays)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP parentClass = GetClass("ParentClass");
    ECClassCP childClass1 = GetClass("ChildClass1");
    ECClassCP childClass2 = GetClass("ChildClass2");
    ECRelationshipClassCP rel1 = GetRelationshipClass("ParentHasChildren1");
    ECRelationshipClassCP rel2 = GetRelationshipClass("ParentHasChildren2");
    IECInstancePtr parent = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *parentClass);
    IECInstancePtr child1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *childClass1, [&](IECInstanceR instance)
        {
        instance.SetValue("Parent", ECValue(RulesEngineTestHelpers::GetInstanceKey(*parent).GetId(), rel1));
        });
    IECInstancePtr child2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *childClass2, [&](IECInstanceR instance)
        {
        instance.SetValue("Parent", ECValue(RulesEngineTestHelpers::GetInstanceKey(*parent).GetId(), rel2));
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("StringProperty", ECValue("111"));

        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);

        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));

        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    IECInstancePtr child3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *childClass2, [&](IECInstanceR instance)
        {
        instance.SetValue("Parent", ECValue(RulesEngineTestHelpers::GetInstanceKey(*parent).GetId(), rel2));
        instance.SetValue("IntProperty", ECValue(2));
        instance.SetValue("StringProperty", ECValue("222"));

        instance.AddArrayElements("ArrayProperty", 1);
        instance.SetValue("ArrayProperty", ECValue(3), 0);

        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));

        instance.AddArrayElements("StructArrayProperty", 2);

        IECInstancePtr structInstance1 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance1->SetValue("IntProperty", ECValue(456));
        structInstance1->SetValue("StringProperty", ECValue("def"));
        ECValue structValue1;
        structValue1.SetStruct(structInstance1.get());
        instance.SetValue("StructArrayProperty", structValue1, 0);

        IECInstancePtr structInstance2 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance2->SetValue("IntProperty", ECValue(789));
        structInstance2->SetValue("StringProperty", ECValue("ghi"));
        ECValue structValue2;
        structValue2.SetStruct(structInstance2.get());
        instance.SetValue("StructArrayProperty", structValue2, 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", parentClass->GetFullName(), false, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel1->GetFullName(),
        childClass1->GetFullName(), "*", RelationshipMeaning::RelatedInstance));
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel2->GetFullName(),
        childClass2->GetFullName(), "*", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); // ParentClass_ParentProperty, Nested<ChildClass1 properties>, Nested<ChildClass2 properties>

    rapidjson::Document expectedChildClass1FieldType;
    expectedChildClass1FieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Struct",
        "TypeName": "ChildClass1",
        "Members": [{
            "Name": "%s",
            "Label": "Parent",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "navigation"
                }
            }]
        })",
        FIELD_NAME(childClass1, "Parent")).c_str());
    rapidjson::Document actualChildClass1FieldType = descriptor->GetVisibleFields()[1]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedChildClass1FieldType, actualChildClass1FieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedChildClass1FieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualChildClass1FieldType);

    rapidjson::Document expectedChildClass2FieldType;
    expectedChildClass2FieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Struct",
        "TypeName": "ChildClass2",
        "Members": [{
            "Name": "%s",
            "Label": "Parent",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "navigation"
                }
            },{
            "Name": "%s",
            "Label": "IntProperty",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "int"
                }
            },{
            "Name": "%s",
            "Label": "StringProperty",
            "Type": {
                "ValueFormat": "Primitive",
                "TypeName": "string"
                }
            },{
            "Name": "%s",
            "Label": "ArrayProperty",
            "Type": {
                "ValueFormat": "Array",
                "TypeName": "int[]",
                "MemberType": {
                    "ValueFormat": "Primitive",
                    "TypeName": "int"
                    }
                }
            },{
            "Name": "%s",
            "Label": "StructProperty",
            "Type": {
                "ValueFormat": "Struct",
                "TypeName": "MyStruct",
                "Members": [{
                    "Name": "IntProperty",
                    "Label": "IntProperty",
                    "Type": {
                        "ValueFormat": "Primitive",
                        "TypeName": "int"
                        }
                    },{
                    "Name": "StringProperty",
                    "Label": "StringProperty",
                    "Type": {
                        "ValueFormat": "Primitive",
                        "TypeName": "string"
                        }
                    }]
                }
            },{
            "Name": "%s",
            "Label": "StructArrayProperty",
            "Type": {
                "ValueFormat": "Array",
                "TypeName": "MyStruct[]",
                "MemberType": {
                    "ValueFormat": "Struct",
                    "TypeName": "MyStruct",
                    "Members": [{
                        "Name": "IntProperty",
                        "Label": "IntProperty",
                        "Type": {
                            "ValueFormat": "Primitive",
                            "TypeName": "int"
                            }
                        },{
                        "Name": "StringProperty",
                        "Label": "StringProperty",
                        "Type": {
                            "ValueFormat": "Primitive",
                            "TypeName": "string"
                            }
                        }]
                    }
                }
            }]
        })",
        FIELD_NAME(childClass2, "Parent"), FIELD_NAME(childClass2, "IntProperty"), FIELD_NAME(childClass2, "StringProperty"),
        FIELD_NAME(childClass2, "ArrayProperty"), FIELD_NAME(childClass2, "StructProperty"),
        FIELD_NAME(childClass2, "StructArrayProperty")).c_str());
    rapidjson::Document actualChildClass2FieldType = descriptor->GetVisibleFields()[2]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedChildClass2FieldType, actualChildClass2FieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedChildClass2FieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualChildClass2FieldType);

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": {"ECClassId": "%s", "ECInstanceId": "%s"}
                },
            "DisplayValues": {
                "%s": "%s"
                },
            "MergedFieldNames": []
            }],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": {"ECClassId": "%s", "ECInstanceId": "%s"},
                "%s": 1,
                "%s": "111",
                "%s": [2, 1],
                "%s": {
                    "IntProperty": 123,
                    "StringProperty": "abc"
                    },
                "%s": [{
                   "IntProperty": 123,
                   "StringProperty": "abc"
                   }]
                },
            "DisplayValues": {
                "%s": "%s",
                "%s": "1",
                "%s": "111",
                "%s": ["2", "1"],
                "%s": {
                    "IntProperty": "123",
                    "StringProperty": "abc"
                    },
                "%s": [{
                   "IntProperty": "123",
                   "StringProperty": "abc"
                   }]
                },
            "MergedFieldNames": []
            },{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": {"ECClassId": "%s", "ECInstanceId": "%s"},
                "%s": 2,
                "%s": "222",
                "%s": [3],
                "%s": {
                    "IntProperty": 456,
                    "StringProperty": "def"
                    },
                "%s": [{
                   "IntProperty": 456,
                   "StringProperty": "def"
                   },{
                   "IntProperty": 789,
                   "StringProperty": "ghi"
                   }]
                },
            "DisplayValues": {
                "%s": "%s",
                "%s": "2",
                "%s": "222",
                "%s": ["3"],
                "%s": {
                    "IntProperty": "456",
                    "StringProperty": "def"
                    },
                "%s": [{
                   "IntProperty": "456",
                   "StringProperty": "def"
                   },{
                   "IntProperty": "789",
                   "StringProperty": "ghi"
                   }]
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(parentClass, "ParentProperty"),
        NESTED_CONTENT_FIELD_NAME(parentClass, childClass1),
        childClass1->GetId().ToString().c_str(), child1->GetInstanceId().c_str(),
        FIELD_NAME(childClass1, "Parent"), parent->GetClass().GetId().ToString().c_str(), parent->GetInstanceId().c_str(),
        FIELD_NAME(childClass1, "Parent"), CommonStrings::RULESENGINE_NOTSPECIFIED,
        NESTED_CONTENT_FIELD_NAME(parentClass, childClass2),
        childClass2->GetId().ToString().c_str(), child2->GetInstanceId().c_str(),
        FIELD_NAME(childClass2, "Parent"), parent->GetClass().GetId().ToString().c_str(), parent->GetInstanceId().c_str(),
        FIELD_NAME(childClass2, "IntProperty"), FIELD_NAME(childClass2, "StringProperty"), FIELD_NAME(childClass2, "ArrayProperty"),
        FIELD_NAME(childClass2, "StructProperty"), FIELD_NAME(childClass2, "StructArrayProperty"),
        FIELD_NAME(childClass2, "Parent"), CommonStrings::RULESENGINE_NOTSPECIFIED,
        FIELD_NAME(childClass2, "IntProperty"), FIELD_NAME(childClass2, "StringProperty"), FIELD_NAME(childClass2, "ArrayProperty"),
        FIELD_NAME(childClass2, "StructProperty"), FIELD_NAME(childClass2, "StructArrayProperty"),
        childClass2->GetId().ToString().c_str(), child3->GetInstanceId().c_str(),
        FIELD_NAME(childClass2, "Parent"), parent->GetClass().GetId().ToString().c_str(),  parent->GetInstanceId().c_str(),
        FIELD_NAME(childClass2, "IntProperty"), FIELD_NAME(childClass2, "StringProperty"), FIELD_NAME(childClass2, "ArrayProperty"),
        FIELD_NAME(childClass2, "StructProperty"), FIELD_NAME(childClass2, "StructArrayProperty"),
        FIELD_NAME(childClass2, "Parent"), CommonStrings::RULESENGINE_NOTSPECIFIED,
        FIELD_NAME(childClass2, "IntProperty"), FIELD_NAME(childClass2, "StringProperty"), FIELD_NAME(childClass2, "ArrayProperty"),
        FIELD_NAME(childClass2, "StructProperty"), FIELD_NAME(childClass2, "StructArrayProperty")).c_str());
        EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToManyRelatedInstancesAsArrays_MergesArrayValuesWhenArraySizeIsOneElementAndValuesAreEqual, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="LongProperty" typeName="long" />
        <ECNavigationProperty propertyName="A" relationshipName="A_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A Has B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B Has A" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedInstancesAsArrays_MergesArrayValuesWhenArraySizeIsOneElementAndValuesAreEqual)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_B")->GetRelationshipClassCP();

    // set up data set
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA1).GetId(), rel));
        });
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA2).GetId(), rel));
        });
    IECInstancePtr instanceA3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA3).GetId(), rel));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(),
        classB->GetFullName(), "*", RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": [{
            "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}, {"ECClassId":"%s", "ECInstanceId":"%s"}, {"ECClassId":"%s", "ECInstanceId":"%s"}],
            "Values": {
                "%s": null,
                "%s": 1,
                "%s": 111
                },
            "DisplayValues": {
                "%s": "%s",
                "%s": "1",
                "%s": "111"
                },
            "MergedFieldNames": ["%s"]
            }]
        })",
        FIELD_NAME(classA, "StringProperty"), NESTED_CONTENT_FIELD_NAME(classA, classB),
        classB->GetId().ToString().c_str(), instanceB1->GetInstanceId().c_str(),
        classB->GetId().ToString().c_str(), instanceB2->GetInstanceId().c_str(),
        classB->GetId().ToString().c_str(), instanceB3->GetInstanceId().c_str(),
        FIELD_NAME(classB, "A"),
        FIELD_NAME(classB, "IntProperty"), FIELD_NAME(classB, "LongProperty"),
        FIELD_NAME(classB, "A"), varies_string.c_str(),
        FIELD_NAME(classB, "IntProperty"), FIELD_NAME(classB, "LongProperty"),
        FIELD_NAME(classB, "A")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(contentSet.Get(0)->IsMerged("A_B"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToManyRelatedInstancesAsArrays_MergesArrayValuesWhenSizesNotEqual, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="LongProperty" typeName="long" />
        <ECNavigationProperty propertyName="A" relationshipName="A_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A Has B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B Has A" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedInstancesAsArrays_MergesArrayValuesWhenSizesNotEqual)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_B")->GetRelationshipClassCP();

    // set up data set
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA1).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA1).GetId(), rel));
        });
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA2).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA2).GetId(), rel));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA2).GetId(), rel));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(),
        classB->GetFullName(), "*", RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // A_StringProperty, Array<B_IntProperty + B_LongProperty>

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": "%s"
        })",
        FIELD_NAME(classA, "StringProperty"), NESTED_CONTENT_FIELD_NAME(classA, classB),
        varies_string.c_str()
    ).c_str());

    EXPECT_EQ(expectedValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    EXPECT_TRUE(contentSet.Get(0)->IsMerged(NESTED_CONTENT_FIELD_NAME(classA, classB)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToManyRelatedInstancesAsArrays_MergesArrayValuesWhenValuesNotEqual, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="LongProperty" typeName="long" />
        <ECNavigationProperty propertyName="A" relationshipName="A_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A Has B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B Has A" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedInstancesAsArrays_MergesArrayValuesWhenValuesNotEqual)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_B")->GetRelationshipClassCP();
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(1));
        instance.SetValue("LongProperty", ECValue((int64_t)111));
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA1).GetId(), rel));
        });
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(4));
        instance.SetValue("LongProperty", ECValue((int64_t)444));
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA2).GetId(), rel));
        });
    IECInstancePtr instanceA3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(7));
        instance.SetValue("LongProperty", ECValue((int64_t)777));
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA3).GetId(), rel));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(),
        classB->GetFullName(), "*", RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // A_StringProperty, Array<B_IntProperty + B_LongProperty>

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}, {"ECClassId": "%s", "ECInstanceId": "%s"}, {"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": null,
                "%s": null,
                "%s": null
                },
            "DisplayValues": {
                "%s": "%s",
                "%s": "%s",
                "%s": "%s"
                },
            "MergedFieldNames": ["%s", "%s", "%s"]
        }]
    })",
        FIELD_NAME(classA, "StringProperty"), NESTED_CONTENT_FIELD_NAME(classA, classB),
        classB->GetId().ToString().c_str(), instanceB1->GetInstanceId().c_str(),
        classB->GetId().ToString().c_str(), instanceB2->GetInstanceId().c_str(),
        classB->GetId().ToString().c_str(), instanceB3->GetInstanceId().c_str(),
        FIELD_NAME(classB, "A"), FIELD_NAME(classB, "IntProperty"), FIELD_NAME(classB, "LongProperty"),
        FIELD_NAME(classB, "A"), varies_string.c_str(),
        FIELD_NAME(classB, "IntProperty"), varies_string.c_str(),
        FIELD_NAME(classB, "LongProperty"), varies_string.c_str(),
        FIELD_NAME(classB, "A"), FIELD_NAME(classB, "IntProperty"), FIELD_NAME(classB, "LongProperty")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToManyRelatedNestedInstancesAsArraysWhenShowingIntermediateProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B" >
        <ECProperty propertyName="PropertyB" typeName="string" />
        <ECNavigationProperty propertyName="A" relationshipName="A_B" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="C" >
        <ECProperty propertyName="PropertyC" typeName="string" />
        <ECNavigationProperty propertyName="B" relationshipName="B_C" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B"  strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C"  strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedNestedInstancesAsArraysWhenShowingIntermediateProperties)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    // set up data set
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA).GetId(), relAB));
        });
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [&](IECInstanceR instance)
        {
        instance.SetValue("B", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceB).GetId(), relBC));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relAB->GetFullName(), classB->GetFullName(), "*", RelationshipMeaning::RelatedInstance));
    spec->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relBC->GetFullName(), classC->GetFullName(), "*", RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": null,
                "%s": {"ECClassId": "%s", "ECInstanceId": "%s"},
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": null,
                        "%s": {"ECClassId": "%s", "ECInstanceId": "%s"}
                        },
                    "DisplayValues": {
                        "%s": null,
                        "%s": "%s"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": null,
                "%s": "%s",
                "%s": [{
                    "DisplayValues": {
                        "%s": null,
                        "%s": "%s"
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(classA, "PropertyA"), NESTED_CONTENT_FIELD_NAME(classA, classB),
        classB->GetId().ToString().c_str(), instanceB->GetInstanceId().c_str(),
        FIELD_NAME(classB, "PropertyB"),
        FIELD_NAME(classB, "A"), instanceA->GetClass().GetId().ToString().c_str(), instanceA->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(classB, classC),
        classC->GetId().ToString().c_str(), instanceC->GetInstanceId().c_str(),
        FIELD_NAME(classC, "PropertyC"),
        FIELD_NAME(classC, "B"), instanceB->GetClass().GetId().ToString().c_str(), instanceB->GetInstanceId().c_str(),
        FIELD_NAME(classC, "PropertyC"), FIELD_NAME(classC, "B"), CommonStrings::RULESENGINE_NOTSPECIFIED,
        FIELD_NAME(classB, "PropertyB"), FIELD_NAME(classB, "A"), CommonStrings::RULESENGINE_NOTSPECIFIED,
        NESTED_CONTENT_FIELD_NAME(classB, classC),
        FIELD_NAME(classC, "PropertyC"), FIELD_NAME(classC, "B"), CommonStrings::RULESENGINE_NOTSPECIFIED
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToManyRelatedNestedInstancesAsArraysWhenSkippingIntermediateProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B" >
        <ECProperty propertyName="PropertyB" typeName="string" />
        <ECNavigationProperty propertyName="A" relationshipName="A_B" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="C" >
        <ECProperty propertyName="PropertyC" typeName="string" />
        <ECNavigationProperty propertyName="B" relationshipName="B_C" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B"  strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C"  strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedNestedInstancesAsArraysWhenSkippingIntermediateProperties)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    // set up data set
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("A", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceA).GetId(), relAB));
        });
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [&](IECInstanceR instance)
        {
        instance.SetValue("B", ECValue(RulesEngineTestHelpers::GetInstanceKey(*instanceB).GetId(), relBC));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relAB->GetFullName(), classB->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    spec->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relBC->GetFullName(), classC->GetFullName(), "*", RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null,
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": null,
                        "%s": {"ECClassId": "%s", "ECInstanceId": "%s"}
                        },
                    "DisplayValues": {
                        "%s": null,
                        "%s": "%s"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": null,
                        "%s": "%s"
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(classA, "PropertyA"),
        NESTED_CONTENT_FIELD_NAME(classA, classB), classB->GetId().ToString().c_str(), instanceB->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(classB, classC), classC->GetId().ToString().c_str(), instanceC->GetInstanceId().c_str(),
        FIELD_NAME(classC, "PropertyC"),
        FIELD_NAME(classC, "B"), instanceB->GetClass().GetId().ToString().c_str(), instanceB->GetInstanceId().c_str(),
        FIELD_NAME(classC, "PropertyC"), FIELD_NAME(classC, "B"), CommonStrings::RULESENGINE_NOTSPECIFIED,
        NESTED_CONTENT_FIELD_NAME(classB, classC),
        FIELD_NAME(classC, "PropertyC"), FIELD_NAME(classC, "B"), CommonStrings::RULESENGINE_NOTSPECIFIED
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToOneToManyRelatedNestedInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <ECProperty propertyName="PropD" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="CB" polymorphic="False">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToOneToManyRelatedNestedInstances)
    {

    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("PropB", ECValue(123));
        });
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [&](IECInstanceR instance)
        {
        instance.SetValue("PropC", ECValue(456));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, [&](IECInstanceR instance)
        {
        instance.SetValue("PropD", ECValue(789));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA,classD}), true, true));
    rules->AddPresentationRule(*rule);

    auto modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 123,
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": 456
                        },
                    "DisplayValues": {
                        "%s": "456"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": "123",
                "%s": [{
                    "DisplayValues": {
                        "%s": "456"
                        }
                    }]
                },
            "MergedFieldNames": []
            }],
        "%s": null
        })",
        NESTED_CONTENT_FIELD_NAME(classA, classB),
        classB->GetId().ToString().c_str(), b->GetInstanceId().c_str(),
        FIELD_NAME(classB, "PropB"),
        NESTED_CONTENT_FIELD_NAME(classB, classC),
        classC->GetId().ToString().c_str(), c->GetInstanceId().c_str(),
        FIELD_NAME(classC, "PropC"), FIELD_NAME(classC, "PropC"),
        FIELD_NAME(classB, "PropB"),
        NESTED_CONTENT_FIELD_NAME(classB, classC),
        FIELD_NAME(classC, "PropC"),
        FIELD_NAME(classD, "PropD")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [],
        "%s": 789
        })",
        NESTED_CONTENT_FIELD_NAME(classA, classB),
        FIELD_NAME(classD, "PropD")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToManyRelatedContentWithRelatedCompositePropertiesFromDifferentUnrelatedClasses, R"*(
    <ECStructClass typeName="S">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECStructClass>
    <ECEntityClass typeName="AR">
        <ECStructProperty propertyName="S" typeName="S" />
    </ECEntityClass>
    <ECEntityClass typeName="BR">
        <ECStructProperty propertyName="S" typeName="S" />
    </ECEntityClass>
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_R" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="AR" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="RA" polymorphic="False">
            <Class class="AR" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_R" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="BR" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="RB" polymorphic="False">
            <Class class="BR" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedContentWithRelatedCompositePropertiesFromDifferentUnrelatedClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classAR = GetClass("AR");
    ECClassCP classBR = GetClass("BR");
    ECRelationshipClassCP relAR = GetClass("A_R")->GetRelationshipClassCP();
    ECRelationshipClassCP relBR = GetClass("B_R")->GetRelationshipClassCP();

    IECInstancePtr ar = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classAR, [](IECInstanceR instance)
        {
        instance.SetValue("S.Prop", ECValue(123));
        });
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAR, *a, *ar);

    IECInstancePtr br = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classBR, [](IECInstanceR instance)
        {
        instance.SetValue("S.Prop", ECValue(456));
        });
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBR, *b, *br);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*rule);

    auto modifierA = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifierA->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAR->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifierA);

    auto modifierB = new ContentModifier(classB->GetSchema().GetName(), classB->GetName());
    modifierB->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBR->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifierB);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create(bvector<IECInstancePtr>{ a, b }))));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{
                "ECClassId": "%s",
                "ECInstanceId": "%s"
            }],
            "Values": {
                "%s": {
                    "Prop": 123
                }
            },
            "DisplayValues": {
                "%s": {
                    "Prop": "123"
                }
            },
            "MergedFieldNames": []
        }],
        "%s": []
    })",
        NESTED_CONTENT_FIELD_NAME(classA, classAR),
        ar->GetClass().GetId().ToString().c_str(), ar->GetInstanceId().c_str(),
        FIELD_NAME(classAR, "S"), FIELD_NAME(classAR, "S"),
        NESTED_CONTENT_FIELD_NAME(classB, classBR)
    ).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{
                "ECClassId": "%s",
                "ECInstanceId": "%s"
            }],
            "Values": {
                "%s": {
                    "Prop": 456
                }
            },
            "DisplayValues": {
                "%s": {
                    "Prop": "456"
                }
            },
            "MergedFieldNames": []
        }]
    })",
        NESTED_CONTENT_FIELD_NAME(classA, classAR),
        NESTED_CONTENT_FIELD_NAME(classB, classBR),
        br->GetClass().GetId().ToString().c_str(), br->GetInstanceId().c_str(),
        FIELD_NAME(classBR, "S"), FIELD_NAME(classBR, "S")
    ).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsXToManyRelatedContentWithRelatedCompositePropertiesWhenOnlyOneOfContentClassesHasIt, R"*(
    <ECStructClass typeName="S">
        <ECProperty propertyName="PropS" typeName="int" />
    </ECStructClass>
    <ECEntityClass typeName="R">
        <ECProperty propertyName="PropR" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="Z">
        <ECStructProperty propertyName="PropZ" typeName="S" />
    </ECEntityClass>
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
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
    <ECRelationshipClass typeName="A_R" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="AR" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="RA" polymorphic="True">
            <Class class="R" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Z" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="BZ" polymorphic="False">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ZB" polymorphic="False">
            <Class class="Z" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsXToManyRelatedContentWithRelatedCompositePropertiesWhenOnlyOneOfContentClassesHasIt)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classR = GetClass("R");
    ECClassCP classZ = GetClass("Z");
    ECRelationshipClassCP relAR = GetClass("A_R")->GetRelationshipClassCP();
    ECRelationshipClassCP relBZ = GetClass("B_Z")->GetRelationshipClassCP();

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr br = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classR, [](IECInstanceR instance)
        {
        instance.SetValue("PropR", ECValue(123));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAR, *b, *br);
    IECInstancePtr bz = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classZ, [](IECInstanceR instance)
        {
        instance.SetValue("PropZ.PropS", ECValue(456));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBZ, *b, *bz);

    IECInstancePtr cr = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classR, [](IECInstanceR instance)
        {
        instance.SetValue("PropR", ECValue(789));
        });
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAR, *c, *cr);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*rule);

    auto modifierA = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifierA->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAR->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    rules->AddPresentationRule(*modifierA);

    auto modifierB = new ContentModifier(classB->GetSchema().GetName(), classB->GetName());
    modifierB->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBZ->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifierB);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::PropertyPane, 0, *KeySet::Create(bvector<IECInstancePtr>{ b, c }))));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);
    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{
                "ECClassId": "%s",
                "ECInstanceId": "%s"
            }, {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
            }],
            "Values": {
                "%s": null
            },
            "DisplayValues": {
                "%s": "%s"
            },
            "MergedFieldNames": ["%s"]
        }],
        "%s": null
    })",
        NESTED_CONTENT_FIELD_NAME(classA, classR),
        classR->GetId().ToString().c_str(), br->GetInstanceId().c_str(),
        classR->GetId().ToString().c_str(), cr->GetInstanceId().c_str(),
        FIELD_NAME(classR, "PropR"), FIELD_NAME(classR, "PropR"), varies_string.c_str(),
        FIELD_NAME(classR, "PropR"),
        NESTED_CONTENT_FIELD_NAME(classB, classZ)
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* make concrete aspect properties are included into the content when requesting it
* with a ContentInstancesOfSpecificClassesSpecification
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForInstancesOfSpecificClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForInstancesOfSpecificClasses)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element"));});
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance){instance.SetValue("AspectName", ECValue("my aspect"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseElementClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseElementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_MyAspect

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect"
                },
            "DisplayValues": {
                "%s": "my aspect"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(baseElementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "AspectName"), FIELD_NAME(aspectClass, "AspectName")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsMultiStepPolymorphicallyRelatedPropertiesForInstancesOfSpecificClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementRefersToElement" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsMultiStepPolymorphicallyRelatedPropertiesForInstancesOfSpecificClasses)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    ECRelationshipClassCP elementRefersToElementRelationship = GetClass("ElementRefersToElement")->GetRelationshipClassCP();

    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("ElementName", ECValue("element 1")); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseElementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementRefersToElementRelationship, *element2, *element1);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance) {instance.SetValue("AspectName", ECValue("aspect")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({
        new RelationshipStepSpecification(elementRefersToElementRelationship->GetFullName(), RequiredRelationDirection_Backward),
        new RelationshipStepSpecification(elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName()),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "aspect"
                },
            "DisplayValues": {
                "%s": "aspect"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{elementClass, baseElementClass}), aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "AspectName"), FIELD_NAME(aspectClass, "AspectName")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* make sure concrete element properties are included into the content when requesting it
* backward with a ContentInstancesOfSpecificClassesSpecification
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyBackwardRelatedPropertiesForInstancesOfSpecificClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyBackwardRelatedPropertiesForInstancesOfSpecificClasses)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element"));});
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance){instance.SetValue("AspectName", ECValue("my aspect"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseAspectClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseAspectClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseElementClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // rel_ElementUniqueAspect_MyElement_ElementName

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId":"%s", "ECInstanceId":"%s"}],
            "Values": {
                "%s": "my element"
            },
            "DisplayValues": {
                "%s": "my element"
            },
            "MergedFieldNames": []
        }]
    })",
        NESTED_CONTENT_FIELD_NAME(baseAspectClass, elementClass),
        elementClass->GetId().ToString().c_str(), element->GetInstanceId().c_str(),
        FIELD_NAME(elementClass, "ElementName"), FIELD_NAME(elementClass, "ElementName")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* make sure we don't include aspects into the content if they're not actually related
* with the instances we're asking content for
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntLoadPolymorphicallyRelatedPropertiesWhenThereAreNoRelatedInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntLoadPolymorphicallyRelatedPropertiesWhenThereAreNoRelatedInstances)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element"));});
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance){instance.SetValue("AspectName", ECValue("my aspect"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseElementClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseElementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(0, descriptor->GetVisibleFields().size()); // aspect property field not included
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different aspects, but only one of them matches instance filter
* - make sure we don't include aspect of instance which doesn't match the filter
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForInstancesOfSpecificClassesMatchingInstanceFilter, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForInstancesOfSpecificClassesMatchingInstanceFilter)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "this.ElementName = \"my element 1\"", elementClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects of the
* selected elements are included into the content
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstances)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*element1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple different types of elements have different related aspects - make sure that
* only aspects of the specified element classes are included into the content
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForSelectedClassInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="AspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="AspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForSelectedClassInstances)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP elementAClass = GetClass("ElementA");
    ECClassCP elementBClass = GetClass("ElementB");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("AspectA");
    ECClassCP aspectBClass = GetClass("AspectB");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr elementA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr elementB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    IECInstancePtr aspectA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspectB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *elementA, *aspectA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *elementB, *aspectB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create({ECClassInstanceKey(elementAClass, ECInstanceId())});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_MyAspectA

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(elementAClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspectA->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsMultiStepPolymorphicallyRelatedPropertiesForSelectedNodeInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementRefersToElement" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsMultiStepPolymorphicallyRelatedPropertiesForSelectedNodeInstances)
    {
    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    ECRelationshipClassCP elementRefersToElementRelationship = GetClass("ElementRefersToElement")->GetRelationshipClassCP();

    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("ElementName", ECValue("element 1")); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseElementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementRefersToElementRelationship, *element2, *element1);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance) {instance.SetValue("AspectName", ECValue("aspect")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({
        new RelationshipStepSpecification(elementRefersToElementRelationship->GetFullName(), RequiredRelationDirection_Backward),
        new RelationshipStepSpecification(elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName()),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*element1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "aspect"
                },
            "DisplayValues": {
                "%s": "aspect"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{elementClass, baseElementClass}), aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "AspectName"), FIELD_NAME(aspectClass, "AspectName")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenRelatedClassesAreNotSpecified, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenRelatedClassesAreNotSpecified)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    // unused - ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        "", "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*element1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenRelationshipsAreNotSpecified, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenRelationshipsAreNotSpecified)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "",
        baseAspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*element1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects of the
* related selected elements are included into the content
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForRelatedNodeInstances, R"*(
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
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward,
        modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*model1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsMultiStepPolymorphicallyRelatedPropertiesForRelatedNodeInstances, R"*(
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
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementRefersToElement" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="AspectName" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsMultiStepPolymorphicallyRelatedPropertiesForRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementClass = GetClass("MyElement");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectClass = GetClass("MyAspect");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    ECRelationshipClassCP elementRefersToElementRelationship = GetClass("ElementRefersToElement")->GetRelationshipClassCP();

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance) {instance.SetValue("ElementName", ECValue("element 1")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model, *element1);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseElementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementRefersToElementRelationship, *element2, *element1);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance) {instance.SetValue("AspectName", ECValue("aspect")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward,
        modelContainsElementsRelationship->GetFullName(), baseElementClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({
        new RelationshipStepSpecification(elementRefersToElementRelationship->GetFullName(), RequiredRelationDirection_Backward),
        new RelationshipStepSpecification(elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName()),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*model);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "aspect"
                },
            "DisplayValues": {
                "%s": "aspect"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{elementClass, baseElementClass}), aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "AspectName"), FIELD_NAME(aspectClass, "AspectName")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects whose
* elements match the instance filter are included into the content
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForRelatedNodeInstancesMatchingInstanceFilter, R"*(
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
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForRelatedNodeInstancesMatchingInstanceFilter)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "this.ElementName = \"my element 1\"", RequiredRelationDirection_Forward,
        modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*model);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects whose
* elements are recursively related to the selected element are included into the content
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_LoadsPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances, R"*(
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
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectC">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_C_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectD">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_D_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_LoadsPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECClassCP aspectCClass = GetClass("MyAspectC");
    ECClassCP aspectDClass = GetClass("MyAspectD");
    ECRelationshipClassCP elementOwnsChildElementsRelationship = GetClass("ElementOwnsChildElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 3"));});
    IECInstancePtr element4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 4"));});
    IECInstancePtr element5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 5"));});
    IECInstancePtr element6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 6"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectCClass, [](IECInstanceR instance){instance.SetValue("Aspect_C_Name", ECValue("my aspect c"));});
    IECInstancePtr aspect6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectDClass, [](IECInstanceR instance){instance.SetValue("Aspect_D_Name", ECValue("my aspect d"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element1, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element2, *element3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element1, *element4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element5, *element6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element3, *aspect3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element4, *aspect4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element6, *aspect6);

    /* hierarchy:
            el1         el5
            / \          |
          el2  el4      el6
           |
          el3
    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Forward,
        elementOwnsChildElementsRelationship->GetFullName(), elementClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(),
        baseAspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*element1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(4, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA, Element_MyAspectB, Element_MyAspectC

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 2",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }],
        "%s": [],
        "%s": []
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 3",
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect b"
                },
            "DisplayValues": {
                "%s": "my aspect b"
                },
            "MergedFieldNames": []
            }],
        "%s": []
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass),
        aspectBClass->GetId().ToString().c_str(), aspect3->GetInstanceId().c_str(),
        FIELD_NAME(aspectBClass, "Aspect_B_Name"), FIELD_NAME(aspectBClass, "Aspect_B_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 4",
        "%s": [],
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect c"
                },
            "DisplayValues": {
                "%s": "my aspect c"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass),
        aspectCClass->GetId().ToString().c_str(), aspect4->GetInstanceId().c_str(),
        FIELD_NAME(aspectCClass, "Aspect_C_Name"), FIELD_NAME(aspectCClass, "Aspect_C_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* multiple elements have different related aspects - make sure that only aspects whose
* elements are recursively related to the selected element are included into the content
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances, R"*(
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
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectC">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_C_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectD">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_D_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECClassCP aspectCClass = GetClass("MyAspectC");
    ECClassCP aspectDClass = GetClass("MyAspectD");
    ECRelationshipClassCP elementOwnsChildElementsRelationship = GetClass("ElementOwnsChildElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 3"));});
    IECInstancePtr element4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 4"));});
    IECInstancePtr element5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 5"));});
    IECInstancePtr element6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 6"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectCClass, [](IECInstanceR instance){instance.SetValue("Aspect_C_Name", ECValue("my aspect c"));});
    IECInstancePtr aspect6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectDClass, [](IECInstanceR instance){instance.SetValue("Aspect_D_Name", ECValue("my aspect d"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element1, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element2, *element3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element1, *element4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *element5, *element6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element3, *aspect3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element4, *aspect4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element6, *aspect6);

    /* hierarchy:
            el1         el5
            / \          |
           /   \         |
        el2(A) el4(C)  el6(D)
          |
        el3(B)
    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification(
        {
        new RepeatableRelationshipStepSpecification(elementOwnsChildElementsRelationship->GetFullName(), RequiredRelationDirection_Forward, elementClass->GetFullName(), 0),
        })}));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*new RelationshipStepSpecification(
        elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName()
    )), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*element1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Element_ElementName, Element_MyAspectA, Element_MyAspectB, Element_MyAspectC

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 3 content set item (el2, el4, el3)
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 2",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }],
        "%s": [],
        "%s": []
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 3",
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect b"
                },
            "DisplayValues": {
                "%s": "my aspect b"
                },
            "MergedFieldNames": []
            }],
        "%s": []
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass),
        aspectBClass->GetId().ToString().c_str(), aspect3->GetInstanceId().c_str(),
        FIELD_NAME(aspectBClass, "Aspect_B_Name"), FIELD_NAME(aspectBClass, "Aspect_B_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my element 4",
        "%s": [],
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect c"
                },
            "DisplayValues": {
                "%s": "my aspect c"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(elementClass, "ElementName"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass),
        aspectCClass->GetId().ToString().c_str(), aspect4->GetInstanceId().c_str(),
        FIELD_NAME(aspectCClass, "Aspect_C_Name"), FIELD_NAME(aspectCClass, "Aspect_C_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForInstancesOfSpecificClassesMatchingInstanceFilter, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
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
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForInstancesOfSpecificClassesMatchingInstanceFilter)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 1"));});
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 2"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "this.ModelName = \"my model 1\"", modelClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), modelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(), baseAspectClass->GetFullName(), "*", RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Model_ModelName, Model_Element

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": "my aspect a"
                        },
                    "DisplayValues": {
                        "%s": "my aspect a"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": "my aspect a"
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"), NESTED_CONTENT_FIELD_NAME(modelClass, elementClass),
        elementClass->GetId().ToString().c_str(), element1->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
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
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 1"));});
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 2"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), modelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(), baseAspectClass->GetFullName(), "*", RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*model1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Model_ModelName, Model_Element

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": "my aspect a"
                        },
                    "DisplayValues": {
                        "%s": "my aspect a"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": "my aspect a"
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"), NESTED_CONTENT_FIELD_NAME(modelClass, elementClass),
        elementClass->GetId().ToString().c_str(), element1->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* A -> B1 | B2 -> C -> D -> E
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenFrontIntermediateRelatedPropertiesSpecificationContainsMultipleClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B1">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D" />
    <ECEntityClass typeName="E">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_D" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="cd" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="dc" polymorphic="true">
            <Class class="D" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="D_E" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="de" polymorphic="true">
            <Class class="D"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ed" polymorphic="true">
            <Class class="E" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenFrontIntermediateRelatedPropertiesSpecificationContainsMultipleClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classB1 = GetClass("B1");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECClassCP classE = GetClass("E");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relCD = GetClass("C_D")->GetRelationshipClassCP();
    ECRelationshipClassCP relDE = GetClass("D_E")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue("One")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c1, *d1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relDE, *d1, *e1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue("Two")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c2, *d2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relDE, *d2, *e2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward,
            Utf8PrintfString("%s:%s,%s", GetSchema()->GetName().c_str(), classB1->GetName().c_str(), classB2->GetName().c_str())),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()),
        new RelationshipStepSpecification(relCD->GetFullName(), RequiredRelationDirection_Forward, classD->GetFullName()),
        new RelationshipStepSpecification(relDE->GetFullName(), RequiredRelationDirection_Forward, classE->GetFullName()),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{ a1, a2 });
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // A_B_C_D_E-Prop

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 2 content set items
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "One"
                },
            "DisplayValues": {
                "%s": "One"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{classA, classB, classC, classD}), classE),
        classE->GetId().ToString().c_str(), e1->GetInstanceId().c_str(),
        FIELD_NAME(classE, "Prop"), FIELD_NAME(classE, "Prop")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "Two"
                },
            "DisplayValues": {
                "%s": "Two"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{classA, classB, classC, classD}), classE),
        classE->GetId().ToString().c_str(), e2->GetInstanceId().c_str(),
        FIELD_NAME(classE, "Prop"), FIELD_NAME(classE, "Prop")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* A -> B -> C1 | C2 -> D -> E
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenMidIntermediateRelatedPropertiesSpecificationContainsMultipleClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C1">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C2">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D" />
    <ECEntityClass typeName="E">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_D" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="cd" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="dc" polymorphic="true">
            <Class class="D" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="D_E" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="de" polymorphic="true">
            <Class class="D"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ed" polymorphic="true">
            <Class class="E" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenMidIntermediateRelatedPropertiesSpecificationContainsMultipleClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classC1 = GetClass("C1");
    ECClassCP classC2 = GetClass("C2");
    ECClassCP classD = GetClass("D");
    ECClassCP classE = GetClass("E");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relCD = GetClass("C_D")->GetRelationshipClassCP();
    ECRelationshipClassCP relDE = GetClass("D_E")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC1);
    IECInstancePtr d1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("One"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c1, *d1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relDE, *d1, *e1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC2);
    IECInstancePtr d2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("Two"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c2, *d2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relDE, *d2, *e2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, classB->GetFullName()),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward,
            Utf8PrintfString("%s:%s,%s", GetSchema()->GetName().c_str(), classC1->GetName().c_str(), classC2->GetName().c_str())),
        new RelationshipStepSpecification(relCD->GetFullName(), RequiredRelationDirection_Forward, classD->GetFullName()),
        new RelationshipStepSpecification(relDE->GetFullName(), RequiredRelationDirection_Forward, classE->GetFullName()),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{ a1, a2 });
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // A_B_C_D_E-Prop

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 2 content set items
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "One"
                },
            "DisplayValues": {
                "%s": "One"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{classA, classB, classC, classD}), classE),
        classE->GetId().ToString().c_str(), e1->GetInstanceId().c_str(),
        FIELD_NAME(classE, "Prop"), FIELD_NAME(classE, "Prop")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "Two"
                },
            "DisplayValues": {
                "%s": "Two"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{classA, classB, classC, classD}), classE),
        classE->GetId().ToString().c_str(), e2->GetInstanceId().c_str(),
        FIELD_NAME(classE, "Prop"), FIELD_NAME(classE, "Prop")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* A -> B -> C -> D1 | D2-> E
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenBackIntermediateRelatedPropertiesSpecificationContainsMultipleClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="D1">
        <BaseClass>D</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D2">
        <BaseClass>D</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_D" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="cd" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="dc" polymorphic="true">
            <Class class="D" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="D_E" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="de" polymorphic="true">
            <Class class="D"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ed" polymorphic="true">
            <Class class="E" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForSelectedNodeInstancesWhenBackIntermediateRelatedPropertiesSpecificationContainsMultipleClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECClassCP classD1 = GetClass("D1");
    ECClassCP classD2 = GetClass("D2");
    ECClassCP classE = GetClass("E");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relCD = GetClass("C_D")->GetRelationshipClassCP();
    ECRelationshipClassCP relDE = GetClass("D_E")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD1);
    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("One"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c1, *d1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relDE, *d1, *e1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD2);
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("Two"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c2, *d2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relDE, *d2, *e2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, classB->GetFullName()),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()),
        new RelationshipStepSpecification(relCD->GetFullName(), RequiredRelationDirection_Forward,
            Utf8PrintfString("%s:%s,%s", GetSchema()->GetName().c_str(), classD1->GetName().c_str(), classD2->GetName().c_str())),
        new RelationshipStepSpecification(relDE->GetFullName(), RequiredRelationDirection_Forward, classE->GetFullName()),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{ a1, a2 });
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // A_B_C_D_E-Prop

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 2 content set items
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "One"
                },
            "DisplayValues": {
                "%s": "One"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{classA, classB, classC, classD}), classE),
        classE->GetId().ToString().c_str(), e1->GetInstanceId().c_str(),
        FIELD_NAME(classE, "Prop"), FIELD_NAME(classE, "Prop")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "Two"
                },
            "DisplayValues": {
                "%s": "Two"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{classA, classB, classC, classD}), classE),
        classE->GetId().ToString().c_str(), e2->GetInstanceId().c_str(),
        FIELD_NAME(classE, "Prop"), FIELD_NAME(classE, "Prop")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsActualSourceClassesOnRelatedContentFields, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A1">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropA1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A2">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropA2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A3">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropA3" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A4">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropA4" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B1">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsActualSourceClassesOnRelatedContentFields)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classA1 = GetClass("A1");
    ECClassCP classA2 = GetClass("A2");
    ECClassCP classA3 = GetClass("A3");
    ECClassCP classA4 = GetClass("A4");
    ECClassCP classB1 = GetClass("B1");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA1);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA2);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA3);
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA4);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a3, *b21);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a4, *b22);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b21, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(7, descriptor->GetVisibleFields().size());

    size_t fieldIndex = 0;
    EXPECT_STREQ("PropA", descriptor->GetVisibleFields()[fieldIndex]->GetLabel().c_str());

    ++fieldIndex;
    EXPECT_STREQ("PropA1", descriptor->GetVisibleFields()[fieldIndex]->GetLabel().c_str());

    ++fieldIndex;
    EXPECT_STREQ("PropA2", descriptor->GetVisibleFields()[fieldIndex]->GetLabel().c_str());

    ++fieldIndex;
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->IsNestedContentField());
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->AsRelatedContentField());
    EXPECT_EQ(classB1, &descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->AsRelatedContentField()->GetContentClass());
    EXPECT_TRUE(ContainsAll(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->AsRelatedContentField()->GetActualSourceClasses(), bvector<ECClassCP>{ classA2 }, true));
    EXPECT_EQ(1, descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("PropB", descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());

    ++fieldIndex;
    EXPECT_STREQ("PropA3", descriptor->GetVisibleFields()[fieldIndex]->GetLabel().c_str());

    ++fieldIndex;
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->IsNestedContentField());
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->AsRelatedContentField());
    EXPECT_EQ(classB2, &descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->AsRelatedContentField()->GetContentClass());
    EXPECT_TRUE(ContainsAll(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->AsRelatedContentField()->GetActualSourceClasses(), bvector<ECClassCP>{ classA3, classA4 }, true));
    EXPECT_EQ(2, descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("PropB", descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->IsNestedContentField());
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->AsRelatedContentField());
    EXPECT_TRUE(ContainsAll(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->AsRelatedContentField()->GetActualSourceClasses(), bvector<ECClassCP>{ classA3 }, true));
    EXPECT_EQ(1, descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("PropC", descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());

    ++fieldIndex;
    EXPECT_STREQ("PropA4", descriptor->GetVisibleFields()[fieldIndex]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesValidDescriptorWhenDifferentRelatedClassesHaveDeeplyNestedContent,
    R"*(
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
    <ECEntityClass typeName="T">
        <ECProperty propertyName="PropT" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_T" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="T" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="S">
        <ECProperty propertyName="PropS" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="T_S" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="T" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="S" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesValidDescriptorWhenDifferentRelatedClassesHaveDeeplyNestedContent)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classT = GetClass("T");
    ECClassCP classS = GetClass("S");
    ECRelationshipClassCP relAT = GetClass("A_T")->GetRelationshipClassCP();
    ECRelationshipClassCP relTS = GetClass("T_S")->GetRelationshipClassCP();

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr t = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classT);
    IECInstancePtr s = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classS);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAT, *b, *t);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAT, *c, *t);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relTS, *t, *s);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, true));

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAT->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relTS->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));
    rules->AddPresentationRule(*modifier);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());

    size_t fieldIndex = 0;
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->IsNestedContentField());
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->AsRelatedContentField());
    EXPECT_EQ(classT, &descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->AsRelatedContentField()->GetContentClass());
    EXPECT_TRUE(ContainsAll(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->AsRelatedContentField()->GetActualSourceClasses(), bvector<ECClassCP>{ classB, classC }, true));
    EXPECT_EQ(2, descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("PropT", descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->IsNestedContentField());
    EXPECT_TRUE(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->AsRelatedContentField());
    EXPECT_EQ(classS, &descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->AsRelatedContentField()->GetContentClass());
    EXPECT_TRUE(ContainsAll(descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->AsRelatedContentField()->GetActualSourceClasses(), bvector<ECClassCP>{ classB, classC }, true));
    EXPECT_EQ(1, descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("PropS", descriptor->GetVisibleFields()[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsCategories" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Category" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Category">
        <ECProperty propertyName="CategoryName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="CategoryContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Category"/>
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
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP categoryClass = GetClass("Category");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECRelationshipClassCP modelContainsCategoriesRelationship = GetClass("ModelContainsCategories")->GetRelationshipClassCP();
    ECRelationshipClassCP categoryContainsElementsRelationship = GetClass("CategoryContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr category1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *categoryClass, [](IECInstanceR instance){instance.SetValue("CategoryName", ECValue("my category 1"));});
    IECInstancePtr category2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *categoryClass, [](IECInstanceR instance){instance.SetValue("CategoryName", ECValue("my category 1"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsCategoriesRelationship, *model1, *category1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsCategoriesRelationship, *model2, *category2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *categoryContainsElementsRelationship, *category1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *categoryContainsElementsRelationship, *category2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward,
        modelContainsCategoriesRelationship->GetFullName(), categoryClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), categoryClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, categoryContainsElementsRelationship->GetFullName(), elementClass->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(), baseAspectClass->GetFullName(), "*", RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*model1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Category_CategoryName, Category_Element

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my category 1",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": "my aspect a"
                        },
                    "DisplayValues": {
                        "%s": "my aspect a"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": "my aspect a"
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(categoryClass, "CategoryName"), NESTED_CONTENT_FIELD_NAME(categoryClass, elementClass),
        elementClass->GetId().ToString().c_str(), element1->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), aspectAClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"), NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_LoadsNestedPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelOwnsSubModel" modifier="Sealed" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="owns sub" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Model"/>
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
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectC">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_C_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectD">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_D_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_LoadsNestedPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECClassCP aspectCClass = GetClass("MyAspectC");
    ECClassCP aspectDClass = GetClass("MyAspectD");
    ECRelationshipClassCP modelOwnsSubModelRelationship = GetClass("ModelOwnsSubModel")->GetRelationshipClassCP();
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 1"));});
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 2"));});
    IECInstancePtr model3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 3"));});
    IECInstancePtr model4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 4"));});
    IECInstancePtr model5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 5"));});
    IECInstancePtr model6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 6"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 3"));});
    IECInstancePtr element4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 4"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectCClass, [](IECInstanceR instance){instance.SetValue("Aspect_C_Name", ECValue("my aspect c"));});
    IECInstancePtr aspect6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectDClass, [](IECInstanceR instance){instance.SetValue("Aspect_D_Name", ECValue("my aspect d"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model1, *model2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model2, *model3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model1, *model4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model5, *model6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model3, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model4, *element3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model6, *element4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element3, *aspect4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element4, *aspect6);

    /* hierarchy:
             m1          m5
            / \          |
           m2  m4        m6
           |
           m3
    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Forward,
        modelOwnsSubModelRelationship->GetFullName(), modelClass->GetFullName()));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), modelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, modelContainsElementsRelationship->GetFullName(), elementClass->GetFullName(), "_none_", RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(
        *new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsUniqueAspectRelationship->GetFullName(), baseAspectClass->GetFullName(), "*", RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*model1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Model_ModelName, Model_Element

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 2",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": "my aspect a"
                        },
                    "DisplayValues": {
                        "%s": "my aspect a"
                        },
                    "MergedFieldNames": []
                    }],
                "%s": [],
                "%s": []
                },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": "my aspect a"
                        }
                    }],
                "%s": [],
                "%s": []
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME(modelClass, elementClass), elementClass->GetId().ToString().c_str(), element1->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), aspectAClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 3",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [],
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": "my aspect b"
                        },
                    "DisplayValues": {
                        "%s": "my aspect b"
                        },
                    "MergedFieldNames": []
                    }],
                "%s": []
                },
            "DisplayValues": {
                "%s": [],
                "%s": [{
                    "DisplayValues": {
                        "%s": "my aspect b"
                        }
                    }],
                "%s": []
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME(modelClass, elementClass), elementClass->GetId().ToString().c_str(), element2->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass), aspectBClass->GetId().ToString().c_str(), aspect3->GetInstanceId().c_str(),
        FIELD_NAME(aspectBClass, "Aspect_B_Name"), FIELD_NAME(aspectBClass, "Aspect_B_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass),
        FIELD_NAME(aspectBClass, "Aspect_B_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass)
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 4",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [],
                "%s": [],
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": "my aspect c"
                        },
                    "DisplayValues": {
                        "%s": "my aspect c"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": [],
                "%s": [],
                "%s": [{
                    "DisplayValues": {
                        "%s": "my aspect c"
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME(modelClass, elementClass), elementClass->GetId().ToString().c_str(), element3->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass), aspectCClass->GetId().ToString().c_str(), aspect4->GetInstanceId().c_str(),
        FIELD_NAME(aspectCClass, "Aspect_C_Name"), FIELD_NAME(aspectCClass, "Aspect_C_Name"),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectAClass), NESTED_CONTENT_FIELD_NAME(elementClass, aspectBClass),
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectCClass), FIELD_NAME(aspectCClass, "Aspect_C_Name")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* TFS#882817
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelOwnsSubModel" modifier="Sealed" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="owns sub" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Model"/>
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
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspectA">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_A_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectB">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_B_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectC">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_C_Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyAspectD">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Aspect_D_Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedPolymorphicallyRelatedPropertiesForRecursivelyRelatedNodeInstances)
    {
    // set up data set
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP aspectAClass = GetClass("MyAspectA");
    ECClassCP aspectBClass = GetClass("MyAspectB");
    ECClassCP aspectCClass = GetClass("MyAspectC");
    ECClassCP aspectDClass = GetClass("MyAspectD");
    ECRelationshipClassCP modelOwnsSubModelRelationship = GetClass("ModelOwnsSubModel")->GetRelationshipClassCP();
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsUniqueAspectRelationship = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 1"));});
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 2"));});
    IECInstancePtr model3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 3"));});
    IECInstancePtr model4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 4"));});
    IECInstancePtr model5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 5"));});
    IECInstancePtr model6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("ModelName", ECValue("my model 6"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 1"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 2"));});
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 3"));});
    IECInstancePtr element4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementName", ECValue("my element 4"));});
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass, [](IECInstanceR instance){instance.SetValue("Aspect_A_Name", ECValue("my aspect a"));});
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass, [](IECInstanceR instance){instance.SetValue("Aspect_B_Name", ECValue("my aspect b"));});
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectCClass, [](IECInstanceR instance){instance.SetValue("Aspect_C_Name", ECValue("my aspect c"));});
    IECInstancePtr aspect6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectDClass, [](IECInstanceR instance){instance.SetValue("Aspect_D_Name", ECValue("my aspect d"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model1, *model2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model2, *model3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model1, *model4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelOwnsSubModelRelationship, *model5, *model6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model3, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model4, *element3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model6, *element4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element1, *aspect2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element2, *aspect3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element3, *aspect4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsUniqueAspectRelationship, *element4, *aspect6, nullptr, true);

    /* hierarchy:
             m1          m5
            / \          |
           m2  m4        m6
         / |   |         |
        /  m3 e3(a4)   e4(a6)
   e1(a2)  |
         e2(a3)

    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(
        modelOwnsSubModelRelationship->GetFullName(), RequiredRelationDirection_Forward, modelClass->GetFullName(), 0))}));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), modelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({
        new RelationshipStepSpecification(modelContainsElementsRelationship->GetFullName(), RequiredRelationDirection_Forward, elementClass->GetFullName()),
        new RelationshipStepSpecification(elementOwnsUniqueAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName())
    }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*model1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Model_ModelName, Model_Element_MyAspectA, Model_Element_MyAspectB, Model_Element_MyAspectC

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 3 content set item (m2, m3, m4)
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 2",
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect a"
                },
            "DisplayValues": {
                "%s": "my aspect a"
                },
            "MergedFieldNames": []
            }],
        "%s": [],
        "%s": []
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        aspectAClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectAClass, "Aspect_A_Name"), FIELD_NAME(aspectAClass, "Aspect_A_Name"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 3",
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect b"
                },
            "DisplayValues": {
                "%s": "my aspect b"
                },
            "MergedFieldNames": []
            }],
        "%s": []
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        aspectBClass->GetId().ToString().c_str(), aspect3->GetInstanceId().c_str(),
        FIELD_NAME(aspectBClass, "Aspect_B_Name"), FIELD_NAME(aspectBClass, "Aspect_B_Name"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass)).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "my model 4",
        "%s": [],
        "%s": [],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "my aspect c"
                },
            "DisplayValues": {
                "%s": "my aspect c"
                },
            "MergedFieldNames": []
            }]
        })",
        FIELD_NAME(modelClass, "ModelName"),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectAClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectBClass),
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{modelClass, elementClass}), aspectCClass),
        aspectCClass->GetId().ToString().c_str(), aspect4->GetInstanceId().c_str(),
        FIELD_NAME(aspectCClass, "Aspect_C_Name"), FIELD_NAME(aspectCClass, "Aspect_C_Name")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelatedPropertiesForMultipleInstancesOfSameClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsMultiAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementMultiAspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelatedPropertiesForMultipleInstancesOfSameClasses)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("ElementMultiAspect");
    ECRelationshipClassCP elementOwnsMultipleAspectRelationship = GetClass("ElementOwnsMultiAspects")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element1, *aspect1);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("def"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsMultipleAspectRelationship->GetFullName(),
        aspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, false));

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{element1, element2});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_ElementMultiAspect

    // set the "merge results" flag
    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                },
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 123,
                "%s": null
                },
            "DisplayValues": {
                "%s": "123",
                "%s": "%s"
                },
            "MergedFieldNames": [
                "%s"
                ]
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        aspectClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        varies_string.c_str(),
        FIELD_NAME(aspectClass, "StringProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelatedPropertiesForMultipleInstancesOfDifferentClassesDerivingFromSameBaseClass, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsMultiAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementMultiAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="MyElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelatedPropertiesForMultipleInstancesOfDifferentClassesDerivingFromSameBaseClass)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementAClass = GetClass("MyElementA");
    ECClassCP elementBClass = GetClass("MyElementB");
    ECClassCP aspectClass = GetClass("ElementMultiAspect");
    ECRelationshipClassCP elementOwnsMultipleAspectRelationship = GetClass("ElementOwnsMultiAspects")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element1, *aspect1);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("def"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseElementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsMultipleAspectRelationship->GetFullName(),
        aspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{element1, element2});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_ElementMultiAspect

    // set the "merge results" flag
    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                },
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 123,
                "%s": null
                },
            "DisplayValues": {
                "%s": "123",
                "%s": "%s"
                },
            "MergedFieldNames": [
                "%s"
                ]
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(baseElementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        aspectClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        varies_string.c_str(),
        FIELD_NAME(aspectClass, "StringProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelatedPropertiesWhenUsingDerivedRelationship, R"*(
    <ECEntityClass typeName="Element" />
    <ECEntityClass typeName="LinkElement">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="Url" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasLinks" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="LinkElement"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementHasMyLinks" modifier="None" strength="embedding">
        <BaseClass>ElementHasLinks</BaseClass>
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="MyLink"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyLink">
        <BaseClass>LinkElement</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelatedPropertiesWhenUsingDerivedRelationship)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP linkClass = GetClass("LinkElement");
    ECClassCP myLinkClass = GetClass("MyLink");
    ECRelationshipClassCP elementHasLinksRelationship = GetClass("ElementHasLinks")->GetRelationshipClassCP();
    ECRelationshipClassCP elementHasMyLinksRelationship = GetClass("ElementHasMyLinks")->GetRelationshipClassCP();
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr link = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *myLinkClass, [](IECInstanceR instance)
        {
        instance.SetValue("Url", ECValue("def"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasMyLinksRelationship, *element, *link);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasLinksRelationship->GetFullName(),
        linkClass->GetFullName(), "*", RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{element});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": "def"
                },
            "DisplayValues": {
                "%s": "def"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(elementClass, myLinkClass),
        myLinkClass->GetId().ToString().c_str(), link->GetInstanceId().c_str(),
        FIELD_NAME(linkClass, "Url"), FIELD_NAME(linkClass, "Url")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* Merges nested related properties into same field when nested ralationship ends
* matches (Both hierarchies ends with same related class (ElementMultiAspect)).
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsNestedRelatedPropertiesForMultipleInstancesOfDifferentClasses, R"*(
    <ECEntityClass typeName="Model">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" modifier="None" strength="embedding">
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
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsMultiAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementMultiAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyModelA">
        <BaseClass>Model</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="MyModelB">
        <BaseClass>Model</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="MyElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="MyElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsNestedRelatedPropertiesForMultipleInstancesOfDifferentClasses)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP baseModelClass = GetClass("Model");
    ECClassCP baseElementClass = GetClass("Element");
    ECClassCP elementAClass = GetClass("MyElementA");
    ECClassCP elementBClass = GetClass("MyElementB");
    ECClassCP modelAClass = GetClass("MyModelA");
    ECClassCP modelBClass = GetClass("MyModelB");
    ECClassCP aspectClass = GetClass("ElementMultiAspect");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementOwnsMultipleAspectRelationship = GetClass("ElementOwnsMultiAspects")->GetRelationshipClassCP();
    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelAClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model1, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element1, *aspect1);
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelBClass);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("def"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model2, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element2, *aspect2);

    /* hierarchies:
        model1(MyModelA) -> element1(MyElementA) -> aspect1(ElementMultiAspect)
        model2(MyModelB) -> element2(MyElementB) -> aspect2(ElementMultiAspect)
    */

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), baseModelClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(modelContainsElementsRelationship->GetFullName(), RequiredRelationDirection_Forward, baseElementClass->GetFullName()),
        new RelationshipStepSpecification(elementOwnsMultipleAspectRelationship->GetFullName(), RequiredRelationDirection_Forward, aspectClass->GetFullName()),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{model1, model2});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Model_Element_ElementMultiAspect

    // set the "merge results" flag
    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                },
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 123,
                "%s": null
                },
            "DisplayValues": {
                "%s": "123",
                "%s": "%s"
                },
            "MergedFieldNames": [
                "%s"
                ]
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{baseModelClass, baseElementClass}), aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect1->GetInstanceId().c_str(),
        aspectClass->GetId().ToString().c_str(), aspect2->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        FIELD_NAME(aspectClass, "IntProperty"), FIELD_NAME(aspectClass, "StringProperty"),
        varies_string.c_str(),
        FIELD_NAME(aspectClass, "StringProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelatedPropertiesForMultipleInstancesOfSameClassesOnlySecondInstanceHasAspect, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsMultiAspects" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementMultiAspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelatedPropertiesForMultipleInstancesOfSameClassesOnlySecondInstanceHasAspect)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("ElementMultiAspect");
    ECRelationshipClassCP elementOwnsMultipleAspectRelationship = GetClass("ElementOwnsMultiAspects")->GetRelationshipClassCP();
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [] (IECInstanceR instance)
        {
        instance.SetValue("IntProperty", ECValue(123));
        instance.SetValue("StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsMultipleAspectRelationship, *element2, *aspect2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementOwnsMultipleAspectRelationship->GetFullName(),
        aspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance, false));

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{element1, element2});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // set the "merge results" flag
    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 1 content set item
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();

    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": null
    })",
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass)
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"({
        "%s": "%s"
    })",
        NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass),
        varies_string.c_str()
    ).c_str());
    EXPECT_EQ(expectedDisplayValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);

    EXPECT_EQ(1, contentSet.Get(0)->GetMergedFieldNames().size());
    EXPECT_STREQ(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass), contentSet.Get(0)->GetMergedFieldNames().back().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* VSTS#29087
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HandlesContentWithPolymorphicallyRelatedPropertiesAndRelatedInstanceUsedInInstanceFilterCorrectly,
R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class = "Aspect" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="DerivedAspect">
        <BaseClass>Aspect</BaseClass>
        <ECProperty propertyName="intProp" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="InfoAspect">
        <BaseClass>Aspect</BaseClass>
        <ECProperty propertyName="intProp" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, HandlesContentWithPolymorphicallyRelatedPropertiesAndRelatedInstanceUsedInInstanceFilterCorrectly)
    {
    // prepare the dataset
    ECRelationshipClassCP rel = dynamic_cast<ECRelationshipClass const *>(GetSchema()->GetClassCP("ElementHasAspect"));
    ECClassCP ecClassElement = GetSchema()->GetClassCP("Element");
    ECClassCP ecClassAspect = GetSchema()->GetClassCP("Aspect");
    ECClassCP ecClassInfoAspect = GetSchema()->GetClassCP("InfoAspect");

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClassElement);
    IECInstancePtr infoAspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClassInfoAspect, [](IECInstanceR instance) {instance.SetValue("intProp", ECValue(75)); });

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element, *infoAspect, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecificationP specification = new ContentInstancesOfSpecificClassesSpecification(1, "b.intProp < 100", ecClassElement->GetFullName(), true, false);
    specification->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), ecClassInfoAspect->GetFullName(), "b", true));
    specification->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), ecClassAspect->GetFullName(), "*",
        RelationshipMeaning::RelatedInstance, true));

    rule->AddSpecification(*specification);
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    // get content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ContentCPtr content = GetVerifiedContent(*descriptor);

    // assert
    ASSERT_TRUE(content.IsValid());
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [
                {
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 75
                },
            "DisplayValues": {
                "%s": "75"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(ecClassElement, ecClassInfoAspect),
        ecClassInfoAspect->GetId().ToString().c_str(), infoAspect->GetInstanceId().c_str(),
        FIELD_NAME(ecClassInfoAspect, "intProp"), FIELD_NAME(ecClassInfoAspect, "intProp")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesAutoExpandedNestedFieldForRelatedProperties,
    R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesAutoExpandedNestedFieldForRelatedProperties)
    {
    // set up data set
    ECClassCP element = GetClass("Element");
    ECClassCP aspect = GetClass("Aspect");

    ECRelationshipClassCP elementHasAspect = dynamic_cast<ECRelationshipClass const *>(GetSchema()->GetClassCP("ElementHasAspect"));

    IECInstancePtr elementInst = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *element);
    IECInstancePtr aspectInst = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspect, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("test")); });

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspect, *elementInst, *aspectInst, nullptr, true);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", element->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(element->GetSchema().GetName(), element->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspect->GetFullName(), "", "*", RelationshipMeaning::RelatedInstance, false, true));

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());

    ASSERT_TRUE(fields[0]->IsNestedContentField());
    EXPECT_TRUE(fields[0]->AsNestedContentField()->ShouldAutoExpand());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#202530
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotIncludeHiddenRelatedClassPropertiesUnlessSpecificallyAskedFor,
    R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="HiddenAspect1">
        <BaseClass>Aspect</BaseClass>
        <ECCustomAttributes>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="HiddenAspect2">
        <BaseClass>Aspect</BaseClass>
        <ECCustomAttributes>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="HiddenAspect3">
        <BaseClass>Aspect</BaseClass>
        <ECCustomAttributes>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Prop3" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Aspect4">
        <BaseClass>Aspect</BaseClass>
        <ECProperty propertyName="Prop4" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesNotIncludeHiddenRelatedClassPropertiesUnlessSpecificallyAskedFor)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectBaseClass = GetClass("Aspect");
    ECClassCP aspectClass1 = GetClass("HiddenAspect1");
    ECClassCP aspectClass2 = GetClass("HiddenAspect2");
    ECClassCP aspectClass3 = GetClass("HiddenAspect3");
    ECClassCP aspectClass4 = GetClass("Aspect4");
    ECRelationshipClassCP elementHasAspectRel = dynamic_cast<ECRelationshipClass const *>(GetSchema()->GetClassCP("ElementHasAspect"));

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass1);
    IECInstancePtr aspect2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass2);
    IECInstancePtr aspect3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass3);
    IECInstancePtr aspect4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect1, nullptr);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect2, nullptr);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect3, nullptr);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect4, nullptr, true);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspectRel->GetFullName(),
        aspectBaseClass->GetFullName(), "*", RelationshipMeaning::RelatedInstance, true));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspectRel->GetFullName(),
        aspectClass2->GetFullName(), "*", RelationshipMeaning::RelatedInstance, true));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspectRel->GetFullName(),
        aspectClass3->GetFullName(), "*", RelationshipMeaning::RelatedInstance, false));

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(3, fields.size());

    // HiddenAspect1.Prop1 is _not_ included because it's hidden and we're including it's base class - not it specifically
    // HiddenAspect2.Prop2 is included because we have a rule that specifically requests it polymorphically
    EXPECT_TRUE(fields.end() != std::find_if(fields.begin(), fields.end(), [&](ContentDescriptor::Field const* f){return f->GetUniqueName().Equals(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass2));}));
    // HiddenAspect3.Prop3 is included because we have a rule that specifically requests it non-polymorphically
    EXPECT_TRUE(fields.end() != std::find_if(fields.begin(), fields.end(), [&](ContentDescriptor::Field const* f){return f->GetUniqueName().Equals(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass3));}));
    // Aspect4.Prop4 is included because it's not hidden
    EXPECT_TRUE(fields.end() != std::find_if(fields.begin(), fields.end(), [&](ContentDescriptor::Field const* f){return f->GetUniqueName().Equals(NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass4));}));
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#278739
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AppendsTheSameRelatedContentMultipleTimes, R"*(
    <ECEntityClass typeName="Element" />
    <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ElementUniqueAspect"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="MyAspect">
        <BaseClass>ElementUniqueAspect</BaseClass>
        <ECProperty propertyName="Prop1" typeName="string" />
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, AppendsTheSameRelatedContentMultipleTimes)
    {
    // set up the dataset
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseAspectClass = GetClass("ElementUniqueAspect");
    ECClassCP myAspectClass = GetClass("MyAspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *myAspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("Prop1", ECValue("a"));
        instance.SetValue("Prop2", ECValue("b"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element, *aspect);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);

    ContentModifier* modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, baseAspectClass->GetFullName())),
        { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true)); // all aspect properties
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, myAspectClass->GetFullName())),
        {new PropertySpecification("Prop1", 1000, "Custom 1")}, RelationshipMeaning::SameInstance, false)); // Prop1 as "Custom 1"
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, myAspectClass->GetFullName())),
        {new PropertySpecification("Prop1", 1000, "Custom 2")}, RelationshipMeaning::SameInstance, false)); // Prop1 as "Custom 2"
    rules->AddPresentationRule(*modifier);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    ASSERT_TRUE(descriptor->GetVisibleFields().back()->IsNestedContentField());

    bvector<ContentDescriptor::Field*> nestedFields = descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields();
    EXPECT_EQ(4, nestedFields.size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "a",
                "%s": "b",
                "%s": "a",
                "%s": "a"
                },
            "DisplayValues": {
                "%s": "a",
                "%s": "b",
                "%s": "a",
                "%s": "a"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(elementClass, myAspectClass),
        myAspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(myAspectClass, "Prop1"), FIELD_NAME(myAspectClass, "Prop2"), FIELD_NAME_C(myAspectClass, "Prop1", 2), FIELD_NAME_C(myAspectClass, "Prop1", 3),
        FIELD_NAME(myAspectClass, "Prop1"), FIELD_NAME(myAspectClass, "Prop2"), FIELD_NAME_C(myAspectClass, "Prop1", 2), FIELD_NAME_C(myAspectClass, "Prop1", 3)
        ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectsAllRelatedPropertiesAndOverridesJustASubsetOfThem, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="P1" typeName="string" />
        <ECProperty propertyName="P2" typeName="string" />
        <ECProperty propertyName="P3" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectsAllRelatedPropertiesAndOverridesJustASubsetOfThem)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{a});

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1000, false);
    auto nodesSpec = new SelectedNodeInstancesSpecification(1000, false, "", "", true);
    auto relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification({new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)}),
        {
        new PropertySpecification("*", 1000),
        new PropertySpecification("P3", 1000, "Custom Label"),
        }, RelationshipMeaning::SameInstance);
    nodesSpec->AddRelatedProperty(*relatedPropertiesSpec);
    rule->AddSpecification(*nodesSpec);
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));

    bvector<ContentDescriptor::Field*> aFields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, aFields.size());
    ASSERT_TRUE(aFields[0]->IsNestedContentField());

    bvector<ContentDescriptor::Field*> bFields = aFields[0]->AsNestedContentField()->GetFields();
    ASSERT_EQ(3, bFields.size());

    size_t fieldIndex = 0;
    ASSERT_TRUE(bFields[fieldIndex]->IsPropertiesField());
    ASSERT_EQ(1, bFields[fieldIndex]->AsPropertiesField()->GetProperties().size());
    EXPECT_EQ(classB->GetPropertyP("P1"), &bFields[fieldIndex]->AsPropertiesField()->GetProperties()[0].GetProperty());

    ++fieldIndex;
    ASSERT_TRUE(bFields[fieldIndex]->IsPropertiesField());
    ASSERT_EQ(1, bFields[fieldIndex]->AsPropertiesField()->GetProperties().size());
    EXPECT_EQ(classB->GetPropertyP("P2"), &bFields[fieldIndex]->AsPropertiesField()->GetProperties()[0].GetProperty());

    ++fieldIndex;
    ASSERT_TRUE(bFields[fieldIndex]->IsPropertiesField());
    ASSERT_EQ(1, bFields[fieldIndex]->AsPropertiesField()->GetProperties().size());
    EXPECT_EQ(classB->GetPropertyP("P3"), &bFields[fieldIndex]->AsPropertiesField()->GetProperties()[0].GetProperty());
    EXPECT_STREQ("Custom Label", bFields[fieldIndex]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesDifferentOverridesWhenSelectingAllRelatedProperties, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="P1" typeName="string" />
        <ECProperty propertyName="P2" typeName="string" />
        <ECProperty propertyName="P3" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesDifferentOverridesWhenSelectingAllRelatedProperties)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{a});

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1000, false);
    auto nodesSpec = new SelectedNodeInstancesSpecification(1000, false, "", "", true);

    // set up property overrides
    auto propSpecStar = new PropertySpecification("*", 1000, "PropSpecStar Label", PropertyCategoryIdentifier::CreateForId("PropSpecStar Category"), true,
        new CustomRendererSpecification("PropSpecStar Renderer"), new PropertyEditorSpecification("PropSpecStar Editor"));
    auto propSpecLabelOverride = new PropertySpecification("P1", 2000, "OverridenLabel");
    auto propSpecCategoryOverride = new PropertySpecification("P1", 2000, "", PropertyCategoryIdentifier::CreateForId("OverridenCategory"));
    auto propSpecRendererOverride = new PropertySpecification("P1", 2000, "", nullptr, true, new CustomRendererSpecification("OverriddenRenderer"));
    auto propSpecEditorOverride = new PropertySpecification("P1", 2000, "", nullptr, true, nullptr, new PropertyEditorSpecification("OverriddenEditor"));
    auto propSpecDisplayOverride = new PropertySpecification("P2", 2000, "", nullptr, false);

    auto relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification({ new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward) }),
        {
        propSpecStar,
        propSpecLabelOverride,
        propSpecCategoryOverride,
        propSpecRendererOverride,
        propSpecEditorOverride,
        propSpecDisplayOverride
        }, RelationshipMeaning::SameInstance);
    nodesSpec->AddRelatedProperty(*relatedPropertiesSpec);
    nodesSpec->AddPropertyCategory(*new PropertyCategorySpecification("PropSpecStar Category", "PropSpecStar Category"));
    nodesSpec->AddPropertyCategory(*new PropertyCategorySpecification("OverridenCategory", "OverridenCategory"));
    rule->AddSpecification(*nodesSpec);
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));

    bvector<ContentDescriptor::Field*> visibleFields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, visibleFields.size());
    ASSERT_TRUE(visibleFields[0]->IsNestedContentField());

    bvector<ContentDescriptor::Field*> nestedFields = visibleFields[0]->AsNestedContentField()->GetFields();
    ASSERT_EQ(2, nestedFields.size());

    //P1 property
    EXPECT_EQ(classB->GetPropertyP("P1"), &nestedFields[0]->AsPropertiesField()->GetProperties()[0].GetProperty());
    EXPECT_STREQ("OverridenLabel", nestedFields[0]->GetLabel().c_str());
    EXPECT_STREQ("OverridenCategory", nestedFields[0]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("OverriddenRenderer", nestedFields[0]->GetRenderer()->GetName().c_str());
    EXPECT_STREQ("OverriddenEditor", nestedFields[0]->GetEditor()->GetName().c_str());

    //P3 property
    EXPECT_EQ(classB->GetPropertyP("P3"), &nestedFields[1]->AsPropertiesField()->GetProperties()[0].GetProperty());
    EXPECT_STREQ("PropSpecStar Label", nestedFields[1]->GetLabel().c_str());
    EXPECT_STREQ("PropSpecStar Category", nestedFields[1]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("PropSpecStar Renderer", nestedFields[1]->GetRenderer()->GetName().c_str());
    EXPECT_STREQ("PropSpecStar Editor", nestedFields[1]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_RelatedPropertiesAccountsForVariables, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B1">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_RelatedPropertiesAccountsForVariables)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB1 = GetClass("B1");
    ECClassCP classB2 = GetClass("B2");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1000, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, "GetVariableIntValues(\"ids\").AnyMatches(x => x = this.ECInstanceId)", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    ContentModifier* modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    rules->AddPresentationRule(*modifier);

    // request content with variables (a2 id)
    RulesetVariables variables;
    variables.SetIntValues("ids", { (int64_t)BeInt64Id::FromString(a2->GetInstanceId().c_str()).GetValueUnchecked() });

    auto descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), variables, "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ("B2", descriptor->GetVisibleFields()[0]->GetLabel().c_str());

    auto content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(1, content->GetContentSet().GetSize());
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ a2.get() }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntDuplicateRelatedPropertyWhenModifierHasSkipIfDuplicateFlag, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="IntProp" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntDuplicateRelatedPropertyWhenModifierHasSkipIfDuplicateFlag)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    auto modifier1 = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier1->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    modifier1->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("IntProp", 1000, "Label 1", PropertyCategoryIdentifier::CreateForId("category1")) }, RelationshipMeaning::SameInstance, true, false, true));
    rules->AddPresentationRule(*modifier1);

    auto modifier2 = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier2->SetPriority(2000);
    modifier2->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier2->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("IntProp", 1000, "Label 2", PropertyCategoryIdentifier::CreateForId("category2")) }, RelationshipMeaning::SameInstance, true, false, true));
    rules->AddPresentationRule(*modifier2);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // verify there's only one field
    auto fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields[0]->IsNestedContentField());
    EXPECT_STREQ("B", fields[0]->GetLabel().c_str());
    ASSERT_EQ(1, fields[0]->AsNestedContentField()->GetFields().size());
    ASSERT_STREQ("Label 2", fields[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    ASSERT_STREQ("Category 2", fields[0]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DuplicatesRelatedPropertyWhenModifierDoesntHaveSkipIfDuplicateFlag, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="IntProp" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DuplicatesRelatedPropertyWhenModifierDoesntHaveSkipIfDuplicateFlag)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    auto modifier1 = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier1->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    modifier1->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("IntProp", 1000, "Label 1", PropertyCategoryIdentifier::CreateForId("category1")) }, RelationshipMeaning::SameInstance, true, false, false));
    rules->AddPresentationRule(*modifier1);

    auto modifier2 = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier2->SetPriority(2000);
    modifier2->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier2->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("IntProp", 1000, "Label 2", PropertyCategoryIdentifier::CreateForId("category2")) }, RelationshipMeaning::SameInstance, true, false, false));
    rules->AddPresentationRule(*modifier2);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // verify there's only one field
    auto fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields[0]->IsNestedContentField());
    EXPECT_STREQ("B", fields[0]->GetLabel().c_str());
    ASSERT_EQ(2, fields[0]->AsNestedContentField()->GetFields().size());
    ASSERT_STREQ("Label 2", fields[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    ASSERT_STREQ("Category 2", fields[0]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetLabel().c_str());
    ASSERT_STREQ("Label 1", fields[0]->AsNestedContentField()->GetFields()[1]->GetLabel().c_str());
    ASSERT_STREQ("Category 1", fields[0]->AsNestedContentField()->GetFields()[1]->GetCategory()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelatedPropertiesCorrectlyWhenUsingBaseClassRelationshipWithDerivedTargetClass, R"*(
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
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="PropD" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelatedPropertiesCorrectlyWhenUsingBaseClassRelationshipWithDerivedTargetClass)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("PropC", ECValue(1));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *c);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, [](IECInstanceR instance){instance.SetValue("PropD", ECValue(2));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *d);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    auto modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));
    rules->AddPresentationRule(*modifier);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::PropertyPane, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // verify there's only one field
    auto fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields[0]->IsNestedContentField());
    EXPECT_STREQ("C", fields[0]->GetLabel().c_str());
    ASSERT_EQ(1, fields[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("PropC", fields[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());

    // verify the value is correct
    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ a.get() }, *content);
    ContentSetItemCPtr primaryRecord = content->GetContentSet()[0];
    auto relatedContentIter = primaryRecord->GetNestedContent().find(fields[0]->GetUniqueName());
    ASSERT_TRUE(primaryRecord->GetNestedContent().end() != relatedContentIter);
    ASSERT_EQ(1, relatedContentIter->second.size());
    ContentSetItemCPtr relatedContentRecord = relatedContentIter->second[0];
    EXPECT_STREQ("1", relatedContentRecord->GetDisplayValues()[fields[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentForSelectedInstanceNode_WithManyRelatedInstances)
    {
    // set up the schema
    RulesEngineTestHelpers::ImportSchema(s_project->GetECDb(), [&](ECSchemaR schema)
        {
        ECEntityClassP elementClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(elementClass, "Element"));
        ASSERT_TRUE(nullptr != elementClass);

        ECEntityClassP aspectClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(aspectClass, "Aspect"));
        ASSERT_TRUE(nullptr != aspectClass);
        IECInstancePtr classMapCustomAttribute = GetClass("ECDbMap", "ClassMap")->GetDefaultStandaloneEnabler()->CreateInstance();
        classMapCustomAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy"));
        ASSERT_EQ(ECObjectsStatus::Success, aspectClass->SetCustomAttribute(*classMapCustomAttribute));

        ECRelationshipClassP relationshipClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateRelationshipClass(relationshipClass, "ElementOwnsAspect", *elementClass, "Source", *aspectClass, "Target"));
        ASSERT_EQ(ECObjectsStatus::Success, relationshipClass->GetTarget().SetMultiplicity("(0..*)"));

        bvector<ECEntityClassP> derivedClasses = RulesEngineTestHelpers::CreateNDerivedClasses(schema, *aspectClass, 100);
        int index = 0;
        for (ECEntityClassP derivedClass : derivedClasses)
            {
            PrimitiveECPropertyP prop = nullptr;
            ASSERT_EQ(ECObjectsStatus::Success, derivedClass->CreatePrimitiveProperty(prop, Utf8PrintfString("String_Prop_%d", index), PRIMITIVETYPE_String));
            }
        });

    ECEntityClassCP elementClass = GetClass("Element")->GetEntityClassCP();
    ECEntityClassCP aspectClass = GetClass("Aspect")->GetEntityClassCP();
    ECRelationshipClassCP rel = GetClass("ElementOwnsAspect")->GetRelationshipClassCP();
    bvector<ECEntityClassCP> derivedClasses;
    for (int i = 0; i < 100; ++i)
        derivedClasses.push_back(GetClass(Utf8PrintfString("Class%d", i + 1).c_str())->GetEntityClassCP());

    // insert just one of the derived class instances
    IECInstancePtr elementInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    bvector<IECInstancePtr> derivedInstances;
    for (ECEntityClassCP derivedClass : derivedClasses)
        {
        derivedInstances.push_back(RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass));
        RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *elementInstance, *derivedInstances.back());
        }

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*rule);

    ContentModifier* modifier = new ContentModifier(GetSchema()->GetName(), elementClass->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, aspectClass->GetFullName())),
        { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));
    rules->AddPresentationRule(*modifier);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::PropertyPane, 0, *KeySet::Create(*elementInstance))));

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HandlesNestedPropertiesInDuplicateRelatedPropertiesSpecification, R"*(
    <ECEntityClass typeName="Base">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="A">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ExternalSource">
        <ECProperty propertyName="ExternalProp" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Repository">
        <ECProperty propertyName="Url" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasExternalSources" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Base"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="ExternalSource"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ExternalSourceIsInRepository" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="ExternalSource"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by" polymorphic="true">
            <Class class="Repository"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, HandlesNestedPropertiesInDuplicateRelatedPropertiesSpecification)
    {
    // set up data set
    ECClassCP classBase = GetClass("Base");
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classExternalSource = GetClass("ExternalSource");
    ECClassCP classRepository = GetClass("Repository");
    ECRelationshipClassCP relElementHasExternalSources = GetClass("ElementHasExternalSources")->GetRelationshipClassCP();
    ECRelationshipClassCP relExternalSourceIsInRepository = GetClass("ExternalSourceIsInRepository")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    IECInstancePtr aExternalSource = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classExternalSource);
    IECInstancePtr bExternalSource = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classExternalSource);
    IECInstancePtr aRepository = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classRepository);
    IECInstancePtr bRepository = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classRepository);

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relElementHasExternalSources, *a, *aExternalSource);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relExternalSourceIsInRepository, *aExternalSource, *aRepository);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relElementHasExternalSources, *b, *bExternalSource);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relExternalSourceIsInRepository, *bExternalSource, *bRepository);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1000, false, "", "", true));
    rules->AddPresentationRule(*rule);

    auto externalSourcePropertiesModifier = new ContentModifier(classBase->GetSchema().GetName(), classBase->GetName());
    rules->AddPresentationRule(*externalSourcePropertiesModifier);
    externalSourcePropertiesModifier->AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relElementHasExternalSources->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        { new PropertySpecification("ExternalProp")}, RelationshipMeaning::SameInstance, false, false, true));

    auto repositoryPropertiesModifier = new ContentModifier(classBase->GetSchema().GetName(), classBase->GetName());
    rules->AddPresentationRule(*repositoryPropertiesModifier);
    repositoryPropertiesModifier->AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relElementHasExternalSources->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, false, false, true));
    repositoryPropertiesModifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        *new RelationshipStepSpecification(relExternalSourceIsInRepository->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        { new PropertySpecification("Url") }, RelationshipMeaning::SameInstance, false, false, false));

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>({a, b}));

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::PropertyPane, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    GetVerifiedContent(*descriptor);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HandlesMultiStepRelatedPropertiesWithPolymorphicMiddleStep, R"*(
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="A">
        <ECProperty propertyName="AProp" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B1">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="CProp" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AToB" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BToC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, HandlesMultiStepRelatedPropertiesWithPolymorphicMiddleStep)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB1 = GetClass("B1");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAToB = GetClass("AToB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBToC = GetClass("BToC")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAToB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAToB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBToC, *b1, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1000, false, "", "", true));
    rules->AddPresentationRule(*rule);

    auto relatedPropModifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    rules->AddPresentationRule(*relatedPropModifier);

    auto relatedPropAToB = new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification({
            new RelationshipStepSpecification(relAToB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward),
            new RelationshipStepSpecification(relBToC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true);
    relatedPropModifier->AddRelatedProperty(*relatedPropAToB);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>({ a1, a2 }));

    // get the content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ a1.get(), a2.get() }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PicksUpOnlyPropertiesOfRelatedInstancesMatchingInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PicksUpOnlyPropertiesOfRelatedInstancesMatchingInstanceFilter)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("Prop", ECValue(111));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("Prop", ECValue(666));});
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("Prop", ECValue(222));});
    IECInstancePtr b4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("Prop", ECValue(777));});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, false, "",
        bvector<MultiSchemaClass*>{ new MultiSchemaClass(classA->GetSchema().GetName(), true, { classA->GetName() }) },
        bvector<MultiSchemaClass*>(), true));
    rules->AddPresentationRule(*rule);

    auto relatedPropertyModifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    auto relatedPropertySpec = CreateSelectAllPolymorphicallyRelatedPropertiesSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward);
    relatedPropertySpec->SetInstanceFilter("this.Prop > 500");
    relatedPropertyModifier->AddRelatedProperty(*relatedPropertySpec);
    rules->AddPresentationRule(*relatedPropertyModifier);

    // get the content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 666
                },
            "DisplayValues": {
                "%s": "666"
                },
            "MergedFieldNames": []
            }, {
            "PrimaryKeys": [{
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 777
                },
            "DisplayValues": {
                "%s": "777"
                },
            "MergedFieldNames": []
            }]
        })",

        NESTED_CONTENT_FIELD_NAME(classA, classB),

        classB->GetId().ToString().c_str(), b2->GetInstanceId().c_str(),
        FIELD_NAME(classB, "Prop"), FIELD_NAME(classB, "Prop"),

        classB->GetId().ToString().c_str(), b4->GetInstanceId().c_str(),
        FIELD_NAME(classB, "Prop"), FIELD_NAME(classB, "Prop")

        ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PicksUpOnlyPropertiesOfRelatedInstancesMatchingIntermediateInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PicksUpOnlyPropertiesOfRelatedInstancesMatchingIntermediateInstanceFilter)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("PropB", ECValue(111));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("PropB", ECValue(222));});
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR inst){inst.SetValue("PropC", ECValue(333));});
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR inst){inst.SetValue("PropC", ECValue(444));});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, false, "",
        bvector<MultiSchemaClass*>{ new MultiSchemaClass(classA->GetSchema().GetName(), true, { classA->GetName() }) },
        bvector<MultiSchemaClass*>(), true));
    rules->AddPresentationRule(*rule);

    auto relatedPropertyModifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    auto relatedPropertySpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification({ new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward) }),
        { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true);
    relatedPropertySpec->SetInstanceFilter("this.PropB = 222");
    relatedPropertySpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({ new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward) }),
        { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    relatedPropertyModifier->AddRelatedProperty(*relatedPropertySpec);
    rules->AddPresentationRule(*relatedPropertyModifier);

    // get the content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{
                "ECClassId": "%s",
                "ECInstanceId": "%s"
                }],
            "Values": {
                "%s": 222,
                "%s": [{
                    "PrimaryKeys": [{
                        "ECClassId": "%s",
                        "ECInstanceId": "%s"
                        }],
                    "Values": {
                        "%s": 444
                        },
                    "DisplayValues": {
                        "%s": "444"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": "222",
                "%s": [{
                    "DisplayValues": {
                        "%s": "444"
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
        })",

        NESTED_CONTENT_FIELD_NAME(classA, classB),

        classB->GetId().ToString().c_str(), b2->GetInstanceId().c_str(),
        FIELD_NAME(classB, "PropB"), NESTED_CONTENT_FIELD_NAME(classB, classC),

        classC->GetId().ToString().c_str(), c2->GetInstanceId().c_str(),
        FIELD_NAME(classC, "PropC"), FIELD_NAME(classC, "PropC"),

        FIELD_NAME(classB, "PropB"), NESTED_CONTENT_FIELD_NAME(classB, classC),

        FIELD_NAME(classC, "PropC"), FIELD_NAME(classC, "PropC")

    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelatedPropertiesAndRelationshipProperties, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <ECProperty propertyName="PropAB" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelatedPropertiesAndRelationshipProperties)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("PropB", ECValue(2));});
    ECInstanceKey abKey = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(4));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, false, false, false, {new PropertySpecification("*")}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // get the content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 4
                },
            "DisplayValues": {
                "%s": "4"
                },
            "MergedFieldNames": []
            }],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 2
                },
            "DisplayValues": {
                "%s": "2"
                },
            "MergedFieldNames": []
            }]
    })",
        NESTED_CONTENT_FIELD_NAME(classA, relAB),
        relAB->GetId().ToString().c_str(), abKey.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB, "PropAB"), FIELD_NAME(relAB, "PropAB"),
        NESTED_CONTENT_FIELD_NAME(classA, classB),
        classB->GetId().ToString().c_str(), b->GetInstanceId().c_str(),
        FIELD_NAME(classB, "PropB"), FIELD_NAME(classB, "PropB")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsOnlyRelationshipProperties, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <ECProperty propertyName="PropAB" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsOnlyRelationshipProperties)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    ECInstanceKey abKey = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(4));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), {}, RelationshipMeaning::RelatedInstance, false, false, false, {new PropertySpecification("*")}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // get the content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 4
                },
            "DisplayValues": {
                "%s": "4"
                },
            "MergedFieldNames": []
            }]
    })",
        NESTED_CONTENT_FIELD_NAME(classA, relAB),
        relAB->GetId().ToString().c_str(), abKey.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB, "PropAB"), FIELD_NAME(relAB, "PropAB")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelationshipPropertiesOfAllRelationships, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D" />
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <ECProperty propertyName="PropAB" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" modifier="None" strength="embedding">
        <ECProperty propertyName="PropBC" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_D" modifier="None" strength="embedding">
        <ECProperty propertyName="PropCD" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="cd" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="dc" polymorphic="true">
            <Class class="D"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelationshipPropertiesOfAllRelationships)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relCD = GetClass("C_D")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    ECInstanceKey abKey = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(4));});
    ECInstanceKey bcKey = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c, [](IECInstanceR instance) {instance.SetValue("PropBC", ECValue(5));});
    ECInstanceKey cdKey = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c, *d, [](IECInstanceR instance) {instance.SetValue("PropCD", ECValue(6));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, false, false, false, {new PropertySpecification("*")}));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, false, false, false, {new PropertySpecification("*")}));
    modifier->GetRelatedProperties().back()->GetNestedRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relCD->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, false, false, false, {new PropertySpecification("*")}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // get the content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 4
                },
            "DisplayValues": {
                "%s": "4"
                },
            "MergedFieldNames": []
            }],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": 5
                        },
                    "DisplayValues": {
                        "%s": "5"
                        },
                    "MergedFieldNames": []
                    }],
                "%s": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "%s": [{
                            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                            "Values": {
                                "%s": 6
                                },
                            "DisplayValues": {
                                "%s": "6"
                                },
                            "MergedFieldNames": []
                            }]
                        },
                    "DisplayValues": {
                        "%s": [{
                            "DisplayValues": {
                                "%s": "6"
                                }
                            }]
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "%s": [{
                    "DisplayValues": {
                        "%s": "5"
                        }
                    }],
                "%s": [{
                    "DisplayValues": {
                        "%s": [{
                            "DisplayValues": {
                                "%s": "6"
                                }
                            }]
                        }
                    }]
                },
            "MergedFieldNames": []
            }]
    })",
        NESTED_CONTENT_FIELD_NAME(classA, relAB),
        relAB->GetId().ToString().c_str(), abKey.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB, "PropAB"), FIELD_NAME(relAB, "PropAB"),
        NESTED_CONTENT_FIELD_NAME(classA, classB),
        classB->GetId().ToString().c_str(), b->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(classB, relBC),
        relBC->GetId().ToString().c_str(), bcKey.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relBC, "PropBC"), FIELD_NAME(relBC, "PropBC"),
        NESTED_CONTENT_FIELD_NAME(classB, classC),
        classC->GetId().ToString().c_str(), c->GetInstanceId().c_str(),
        NESTED_CONTENT_FIELD_NAME(classC, relCD),
        relCD->GetId().ToString().c_str(), cdKey.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relCD, "PropCD"), FIELD_NAME(relCD, "PropCD"),
        NESTED_CONTENT_FIELD_NAME(classC, relCD), FIELD_NAME(relCD, "PropCD"),
        NESTED_CONTENT_FIELD_NAME(classB, relBC), FIELD_NAME(relBC, "PropBC"),
        NESTED_CONTENT_FIELD_NAME(classB, classC), NESTED_CONTENT_FIELD_NAME(classC, relCD), FIELD_NAME(relCD, "PropCD")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelationshipPropertiesWhenTwoInstancesAreRelatedToTheSameInstance, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <ECProperty propertyName="PropAB" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelationshipPropertiesWhenTwoInstancesAreRelatedToTheSameInstance)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    ECInstanceKey abKey1 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b1, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(4));});
    ECInstanceKey abKey2 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b2, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(5));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, false, false, false, {new PropertySpecification("*")}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // get the content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 4
                },
            "DisplayValues": {
                "%s": "4"
                },
            "MergedFieldNames": []
            },{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 5
                },
            "DisplayValues": {
                "%s": "5"
                },
            "MergedFieldNames": []
            }]
    })",
        NESTED_CONTENT_FIELD_NAME(classA, relAB),
        relAB->GetId().ToString().c_str(), abKey1.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB, "PropAB"), FIELD_NAME(relAB, "PropAB"),
        relAB->GetId().ToString().c_str(), abKey2.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB, "PropAB"), FIELD_NAME(relAB, "PropAB")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelationshipPropertiesWhenIntermediateClassesAreDifferent, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B1">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <ECProperty propertyName="PropAB" typeName="int" />
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="Sealed">
        <ECProperty propertyName="PropBC" typeName="int" />
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelationshipPropertiesWhenIntermediateClassesAreDifferent)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classB1 = GetClass("B1");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(1));});
    ECInstanceKey bcKey1 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1, [](IECInstanceR instance) {instance.SetValue("PropBC", ECValue(2));});

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(3));});
    ECInstanceKey bcKey2 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2, [](IECInstanceR instance) {instance.SetValue("PropBC", ECValue(4));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward,
            Utf8PrintfString("%s:%s,%s", GetSchema()->GetName().c_str(), classB1->GetName().c_str(), classB2->GetName().c_str())),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()),
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, true, false, false, {new PropertySpecification("*")}));

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{ a1, a2 });
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // get the content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 2
                },
            "DisplayValues": {
                "%s": "2"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{classA, classB}), relBC),
        relBC->GetId().ToString().c_str(), bcKey1.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relBC, "PropBC"), FIELD_NAME(relBC, "PropBC")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 4
                },
            "DisplayValues": {
                "%s": "4"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{classA, classB}), relBC),
        relBC->GetId().ToString().c_str(), bcKey2.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relBC, "PropBC"), FIELD_NAME(relBC, "PropBC")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelationshipPropertiesWhenInstanceHasTwoRelatedInstancesWithDifferentIntermediateClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B1">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <ECProperty propertyName="PropAB" typeName="int" />
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="Sealed">
        <ECProperty propertyName="PropBC" typeName="int" />
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelationshipPropertiesWhenInstanceHasTwoRelatedInstancesWithDifferentIntermediateClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classB1 = GetClass("B1");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b1, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(1));});
    ECInstanceKey bcKey1 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1, [](IECInstanceR instance) {instance.SetValue("PropBC", ECValue(2));});

    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b2, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(3));});
    ECInstanceKey bcKey2 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2, [](IECInstanceR instance) {instance.SetValue("PropBC", ECValue(4));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward,
            Utf8PrintfString("%s:%s,%s", GetSchema()->GetName().c_str(), classB1->GetName().c_str(), classB2->GetName().c_str())),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()),
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, true, false, false, {new PropertySpecification("*")}));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*a);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // get the content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 2
                },
            "DisplayValues": {
                "%s": "2"
                },
            "MergedFieldNames": []
            },{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 4
                },
            "DisplayValues": {
                "%s": "4"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME((bvector<ECClassCP>{classA, classB}), relBC),
        relBC->GetId().ToString().c_str(), bcKey1.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relBC, "PropBC"), FIELD_NAME(relBC, "PropBC"),
        relBC->GetId().ToString().c_str(), bcKey2.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relBC, "PropBC"), FIELD_NAME(relBC, "PropBC")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsRelationshipPropertiesWhenSourceInstanceTargetsMultipleInstancesOfTheSameClassUsingDifferentRelationships, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Abstract">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_B_1" strength="embedding" modifier="Sealed">
        <BaseClass>A_B</BaseClass>
        <ECProperty propertyName="PropAB1" typeName="int" />
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_B_2" strength="embedding" modifier="Sealed">
        <BaseClass>A_B</BaseClass>
        <ECProperty propertyName="PropAB2" typeName="int" />
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsRelationshipPropertiesWhenSourceInstanceTargetsMultipleInstancesOfTheSameClassUsingDifferentRelationships)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB1 = GetClass("A_B_1")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB2 = GetClass("A_B_2")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    ECInstanceKey ab1 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB1, *a, *b1, [](IECInstanceR instance){instance.SetValue("PropAB1", ECValue(123));});

    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    ECInstanceKey ab2 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB2, *a, *b2, [](IECInstanceR instance){instance.SetValue("PropAB2", ECValue(456));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true, false, false, { new PropertySpecification("*") }));

    // validate descriptor
    KeySetPtr input = KeySet::Create(*a);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // get the content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 123
                },
            "DisplayValues": {
                "%s": "123"
                },
            "MergedFieldNames": []
            }],
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 456
                },
            "DisplayValues": {
                "%s": "456"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(classA, relAB1),
        relAB1->GetId().ToString().c_str(), ab1.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB1, "PropAB1"), FIELD_NAME(relAB1, "PropAB1"),
        NESTED_CONTENT_FIELD_NAME(classA, relAB2),
        relAB2->GetId().ToString().c_str(), ab2.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB2, "PropAB2"), FIELD_NAME(relAB2, "PropAB2")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortsRelationshipProperties, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <ECProperty propertyName="PropAB" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SortsRelationshipProperties)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    ECInstanceKey abKey1 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b1, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(3));});
    ECInstanceKey abKey2 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b2, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(1));});
    ECInstanceKey abKey3 = RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b3, [](IECInstanceR instance) {instance.SetValue("PropAB", ECValue(2));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    SortingRuleP sortingRule = new SortingRule("", 1, relAB->GetSchema().GetName(), relAB->GetName(), "PropAB", true, false, false);
    rules->AddPresentationRule(*sortingRule);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, false, false, false, {new PropertySpecification("*")}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // get the content
    ContentCPtr content = GetVerifiedContent(*descriptor);

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 1
                },
            "DisplayValues": {
                "%s": "1"
                },
            "MergedFieldNames": []
            },{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 2
                },
            "DisplayValues": {
                "%s": "2"
                },
            "MergedFieldNames": []
            },{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": 3
                },
            "DisplayValues": {
                "%s": "3"
                },
            "MergedFieldNames": []
            }]
        })",
        NESTED_CONTENT_FIELD_NAME(classA, relAB),
        relAB->GetId().ToString().c_str(), abKey2.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB, "PropAB"), FIELD_NAME(relAB, "PropAB"),
        relAB->GetId().ToString().c_str(), abKey3.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB, "PropAB"), FIELD_NAME(relAB, "PropAB"),
        relAB->GetId().ToString().c_str(), abKey1.GetInstanceId().ToString().c_str(),
        FIELD_NAME(relAB, "PropAB"), FIELD_NAME(relAB, "PropAB")
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntDuplicateRelatedPropertyWithLowerPriorityUsingDerivedRelationshipClassWhenPropertySpecificationHasSkipIfDuplicateFlag, R"*(
    <ECEntityClass typeName="A"/>
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <ECProperty propertyName="PropB" typeName="int" />
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="AToB_Base" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="AToB_Derived" modifier="None" strength="embedding">
        <BaseClass>AToB_Base</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="ab2" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b2a" polymorphic="true">
            <Class class="B2"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntDuplicateRelatedPropertyWithLowerPriorityUsingDerivedRelationshipClassWhenPropertySpecificationHasSkipIfDuplicateFlag)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB2 = GetClass("B2");
    ECRelationshipClassCP relAB2 = GetClass("AToB_Derived")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("AToB_Base")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2, [&](IECInstanceR instance){ instance.SetValue("PropB", ECValue(10)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB2, *a, *b2);

    KeySetPtr input = KeySet::Create({ classA });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB2->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    modifier->SetPriority(2000);
    rules->AddPresentationRule(*modifier);

    ContentModifierP modifier2 = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier2->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true, false, true));
    rules->AddPresentationRule(*modifier2);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(classA, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());

    ASSERT_EQ(2, descriptor->GetAllFields().size());
    ASSERT_EQ(CommonStrings::ECPRESENTATION_DISPLAYLABEL, descriptor->GetAllFields()[0]->GetLabel());

    ASSERT_TRUE(descriptor->GetAllFields()[1]->IsNestedContentField());
    ASSERT_EQ(classB2->GetDisplayLabel(), descriptor->GetAllFields()[1]->GetLabel());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CorrectlyHandlesDifferentLengthRelatedPropertyPathsWithSimilarFirstStep, R"*(
    <ECEntityClass typeName="A"/>
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <ECProperty propertyName="PropB" typeName="int" />
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AToB_Base" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="AToB_Derived" modifier="None" strength="embedding">
        <BaseClass>AToB_Base</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="ab2" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b2a" polymorphic="true">
            <Class class="B2"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BToC" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CorrectlyHandlesDifferentLengthRelatedPropertyPathsWithSimilarFirstStep)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB2 = GetClass("AToB_Derived")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("AToB_Base")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BToC")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2, [&](IECInstanceR instance){ instance.SetValue("PropB", ECValue(10)); });
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [&](IECInstanceR instance){ instance.SetValue("PropC", ECValue(20)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB2, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c);

    KeySetPtr input = KeySet::Create({ classA });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB2->GetFullName(), RequiredRelationDirection_Forward),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    modifier->SetPriority(2000);
    rules->AddPresentationRule(*modifier);

    ContentModifierP modifier2 = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier2->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    rules->AddPresentationRule(*modifier2);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(
        AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(classA, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());

    ASSERT_EQ(3, descriptor->GetAllFields().size());
    ASSERT_EQ(CommonStrings::ECPRESENTATION_DISPLAYLABEL, descriptor->GetAllFields()[0]->GetLabel());

    ASSERT_TRUE(descriptor->GetAllFields()[1]->IsNestedContentField());
    ASSERT_EQ(classC->GetDisplayLabel(), descriptor->GetAllFields()[1]->GetLabel());
    RelatedClassPath& fieldPathFromSelectToContentClass = descriptor->GetAllFields()[1]->AsNestedContentField()->AsRelatedContentField()->GetPathFromSelectToContentClass();
    ASSERT_EQ(2, fieldPathFromSelectToContentClass.size());
    ASSERT_EQ(relAB2, &fieldPathFromSelectToContentClass[0].GetRelationship().GetClass());
    ASSERT_EQ(relBC, &fieldPathFromSelectToContentClass[1].GetRelationship().GetClass());

    ASSERT_TRUE(descriptor->GetAllFields()[2]->IsNestedContentField());
    ASSERT_EQ(classB2->GetDisplayLabel(), descriptor->GetAllFields()[2]->GetLabel());
    fieldPathFromSelectToContentClass = descriptor->GetAllFields()[2]->AsNestedContentField()->AsRelatedContentField()->GetPathFromSelectToContentClass();
    ASSERT_EQ(1, fieldPathFromSelectToContentClass.size());
    ASSERT_EQ(relAB2, &fieldPathFromSelectToContentClass[0].GetRelationship().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NestsRelatedPropertyWhenDifferentLengthPathsHaveSimilarFirstStep, R"*(
    <ECEntityClass typeName="A"/>
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <ECProperty propertyName="PropB" typeName="int" />
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AToB_Base" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="AToB_Derived" modifier="None" strength="embedding">
        <BaseClass>AToB_Base</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="ab2" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b2a" polymorphic="true">
            <Class class="B2"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BToC" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, NestsRelatedPropertyWhenDifferentLengthPathsHaveSimilarFirstStep)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB2 = GetClass("AToB_Derived")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("AToB_Base")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BToC")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2, [&](IECInstanceR instance){ instance.SetValue("PropB", ECValue(10)); });
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [&](IECInstanceR instance){ instance.SetValue("PropC", ECValue(20)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB2, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c);

    KeySetPtr input = KeySet::Create({ classA });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB2->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    modifier->SetPriority(2000);
    rules->AddPresentationRule(*modifier);

    ContentModifierP modifier2 = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier2->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    rules->AddPresentationRule(*modifier2);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(
        AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(classA, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());

    ASSERT_EQ(2, descriptor->GetAllFields().size());
    ASSERT_EQ(CommonStrings::ECPRESENTATION_DISPLAYLABEL, descriptor->GetAllFields()[0]->GetLabel());
    ASSERT_TRUE(descriptor->GetAllFields()[1]->IsNestedContentField());
    ASSERT_EQ(classB2->GetDisplayLabel(), descriptor->GetAllFields()[1]->GetLabel());

    RelatedClassPath& fieldPathFromSelectToContentClass = descriptor->GetAllFields()[1]->AsNestedContentField()->AsRelatedContentField()->GetPathFromSelectToContentClass();
    ASSERT_EQ(1, fieldPathFromSelectToContentClass.size());
    ASSERT_EQ(relAB2, &fieldPathFromSelectToContentClass[0].GetRelationship().GetClass());

    ASSERT_EQ(2, descriptor->GetAllFields()[1]->AsNestedContentField()->GetFields().size());
    ASSERT_EQ("PropB", descriptor->GetAllFields()[1]->AsNestedContentField()->GetFields()[0]->GetLabel());
    ASSERT_EQ(classC->GetDisplayLabel(), descriptor->GetAllFields()[1]->AsNestedContentField()->GetFields()[1]->GetLabel());

    fieldPathFromSelectToContentClass = descriptor->GetAllFields()[1]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->AsRelatedContentField()->GetPathFromSelectToContentClass();
    ASSERT_EQ(1, fieldPathFromSelectToContentClass.size());
    ASSERT_EQ(relBC, &fieldPathFromSelectToContentClass[0].GetRelationship().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CorrectlyHandlesRelatedPropertiesUsingDerivedRelationshipClassesWhenPropertySpecificationHasSkipIfDuplicateFlag, R"*(
    <ECEntityClass typeName="A"/>
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <ECProperty propertyName="PropB2" typeName="int" />
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B3">
        <ECProperty propertyName="PropB3" typeName="int" />
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="AToB_Base" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="AToB2_Derived" modifier="None" strength="embedding">
        <BaseClass>AToB_Base</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="ab2" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b2a" polymorphic="true">
            <Class class="B2"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="AToB3_Derived" modifier="None" strength="embedding">
        <BaseClass>AToB_Base</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="ab2" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b2a" polymorphic="true">
            <Class class="B3"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CorrectlyHandlesRelatedPropertiesUsingDerivedRelationshipClassesWhenPropertySpecificationHasSkipIfDuplicateFlag)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classB3 = GetClass("B3");
    ECRelationshipClassCP relAB2 = GetClass("AToB2_Derived")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB3 = GetClass("AToB3_Derived")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2, [&](IECInstanceR instance){ instance.SetValue("PropB2", ECValue(10)); });
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB3, [&](IECInstanceR instance){ instance.SetValue("PropB3", ECValue(20)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB2, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB3, *a, *b3);

    KeySetPtr input = KeySet::Create({ classA });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB2->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    modifier->SetPriority(2000);
    rules->AddPresentationRule(*modifier);

    ContentModifierP modifier2 = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier2->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB3->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true, false, true));
    rules->AddPresentationRule(*modifier2);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(classA, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());

    ASSERT_EQ(3, descriptor->GetAllFields().size());
    ASSERT_EQ(CommonStrings::ECPRESENTATION_DISPLAYLABEL, descriptor->GetAllFields()[0]->GetLabel());

    ASSERT_TRUE(descriptor->GetAllFields()[1]->IsNestedContentField());
    ASSERT_EQ(classB2->GetDisplayLabel(), descriptor->GetAllFields()[1]->GetLabel());

    ASSERT_TRUE(descriptor->GetAllFields()[2]->IsNestedContentField());
    ASSERT_EQ(classB3->GetDisplayLabel(), descriptor->GetAllFields()[2]->GetLabel());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CorrectlyHandlesRelatedPropertiesUsingDerivedAndBaseRelationshipClassesWhenPropertySpecificationHasSkipIfDuplicateFlag, R"*(
    <ECEntityClass typeName="A"/>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="AToB_Base" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="AToB_Derived" modifier="None" strength="embedding">
        <BaseClass>AToB_Base</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="ab2" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b2a" polymorphic="true">
            <Class class="B2"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CorrectlyHandlesRelatedPropertiesUsingDerivedAndBaseRelationshipClassesWhenPropertySpecificationHasSkipIfDuplicateFlag)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classB2 = GetClass("B2");
    ECRelationshipClassCP relAB2 = GetClass("AToB_Derived")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("AToB_Base")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance){ instance.SetValue("PropB", ECValue(10)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2, [&](IECInstanceR instance){ instance.SetValue("PropB", ECValue(20)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB2, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);

    KeySetPtr input = KeySet::Create({ classA });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB2->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    modifier->SetPriority(2000);
    rules->AddPresentationRule(*modifier);

    ContentModifierP modifier2 = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier2->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true, false, true));
    rules->AddPresentationRule(*modifier2);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(classA, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());

    ASSERT_EQ(3, descriptor->GetAllFields().size());
    ASSERT_EQ(CommonStrings::ECPRESENTATION_DISPLAYLABEL, descriptor->GetAllFields()[0]->GetLabel());

    ASSERT_TRUE(descriptor->GetAllFields()[1]->IsNestedContentField());
    ASSERT_EQ(classB2->GetDisplayLabel(), descriptor->GetAllFields()[1]->GetLabel());

    ASSERT_TRUE(descriptor->GetAllFields()[2]->IsNestedContentField());
    ASSERT_EQ(classB->GetDisplayLabel(), descriptor->GetAllFields()[2]->GetLabel());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntDuplicateRelatedPropertyWithLowerPriorityUsingBaseRelationshipClassWhenPropertySpecificationHasSkipIfDuplicateFlag, R"*(
    <ECEntityClass typeName="A"/>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="AToB_Base" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="AToB_Derived" modifier="None" strength="embedding">
        <BaseClass>AToB_Base</BaseClass>
        <Source multiplicity="(0..1)" roleLabel="ab2" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b2a" polymorphic="true">
            <Class class="B2"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntDuplicateRelatedPropertyWithLowerPriorityUsingBaseRelationshipClassWhenPropertySpecificationHasSkipIfDuplicateFlag)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classB2 = GetClass("B2");
    ECRelationshipClassCP relAB2 = GetClass("AToB_Derived")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("AToB_Base")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance){ instance.SetValue("PropB", ECValue(10)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2, [&](IECInstanceR instance){ instance.SetValue("PropB", ECValue(10)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB2, *a, *b2);

    KeySetPtr input = KeySet::Create({ classA });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB2->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));
    modifier->SetPriority(2000);
    rules->AddPresentationRule(*modifier);

    ContentModifierP modifier2 = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier2->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true, false, true));
    rules->AddPresentationRule(*modifier2);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(classA, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());

    ASSERT_EQ(2, descriptor->GetAllFields().size());
    ASSERT_EQ(CommonStrings::ECPRESENTATION_DISPLAYLABEL, descriptor->GetAllFields()[0]->GetLabel());

    ASSERT_TRUE(descriptor->GetAllFields()[1]->IsNestedContentField());
    ASSERT_EQ(classB2->GetDisplayLabel(), descriptor->GetAllFields()[1]->GetLabel());
    }
