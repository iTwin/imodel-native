/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentIntegrationTests.h"

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_GetDistinctValues, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_GetDistinctValues)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));

    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    overridenDescriptor->ExclusivelyIncludeFields({ overridenDescriptor->FindField(PropertiesContentFieldMatcher(*classA->GetPropertyP("Property"), {})) });
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // validate descriptor
    EXPECT_EQ(1, overridenDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*overridenDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR values1 = contentSet.Get(0)->GetValues();
    ASSERT_EQ(1, values1.MemberCount());
    EXPECT_STREQ("Test1", values1[FIELD_NAME(classA, "Property")].GetString());
    RapidJsonValueCR values2 = contentSet.Get(1)->GetValues();
    ASSERT_EQ(1, values2.MemberCount());
    EXPECT_STREQ("Test2", values2[FIELD_NAME(classA, "Property")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_GetDistinctValuesFromRelatedInstancesWithSeveralRelationships, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_Bs" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_GetDistinctValuesFromRelatedInstancesWithSeveralRelationships)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relABs = GetClass("A_Bs")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){ instance.SetValue("Property", ECValue("Test1"));});
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test2"));});
    IECInstancePtr instanceB3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relABs, *instanceA, *instanceB1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relABs, *instanceA, *instanceB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relABs, *instanceA, *instanceB3);

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB2);

    KeySetPtr keyset = KeySet::Create();
    keyset->Add(classA, ECInstanceId(ECInstanceId::FromString(instanceA->GetInstanceId().c_str())));

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentRelatedInstancesSpecification* spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection::RequiredRelationDirection_Both,"", classB->GetFullName());
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *keyset)));

    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    overridenDescriptor->ExclusivelyIncludeFields({ overridenDescriptor->FindField(PropertiesContentFieldMatcher(*classB->GetPropertyP("Property"), {})) });
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // validate descriptor
    EXPECT_EQ(1, overridenDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*overridenDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR values1 = contentSet.Get(0)->GetValues();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({"%s": "Test2"})", FIELD_NAME(classB, "Property")).c_str());
    EXPECT_EQ(expectedValues1, values1)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(values1);

    RapidJsonValueCR values2 = contentSet.Get(1)->GetValues();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({"%s": "Test1"})", FIELD_NAME(classB, "Property")).c_str());
    EXPECT_EQ(expectedValues2, values2)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(values2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_GetDistinctValuesOfCalculatedProperty, R"*(
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="B_A" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_GetDistinctValuesOfCalculatedProperty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relBA = GetClass("B_A")->GetRelationshipClassCP();

    //set up data
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property",ECValue("Test1"));});
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test2"));});
    IECInstancePtr instanceB3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBA, *instanceA, *instanceB1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBA, *instanceA, *instanceB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBA, *instanceA, *instanceB3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    CalculatedPropertiesSpecification* calcProp1 = new CalculatedPropertiesSpecification("MyCalculatedProperty", 1000, "this.Property+\"Calculated\"");
    ContentInstancesOfSpecificClassesSpecification* specificSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    specificSpec->AddCalculatedProperty(*calcProp1);
    contentRule->AddSpecification(*specificSpec);
    CalculatedPropertiesSpecification* calcProp2 = new CalculatedPropertiesSpecification("MyCalculatedProperty", 1000, "this.Property+\"Calculated\"");
    ContentRelatedInstancesSpecification* relatedSpec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection::RequiredRelationDirection_Both, relBA->GetFullName(), classA->GetFullName());
    relatedSpec->AddCalculatedProperty(*calcProp2);
    contentRule->AddSpecification(*relatedSpec);
    SelectedNodeInstancesSpecification* selectedSpec = new SelectedNodeInstancesSpecification();
    CalculatedPropertiesSpecification* calcProp3 = new CalculatedPropertiesSpecification("MyCalculatedProperty", 1000, "this.Property+\"Calculated\"");
    selectedSpec->AddCalculatedProperty(*calcProp3);
    contentRule->AddSpecification(*selectedSpec);

    rules->AddPresentationRule(*contentRule);

    KeySetPtr keys = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB1, instanceB2, instanceB3});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *keys)));

    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    overridenDescriptor->ExclusivelyIncludeFields({ overridenDescriptor->FindField(NamedContentFieldMatcher("CalculatedProperty_0")) });
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // validate descriptor
    EXPECT_EQ(1, overridenDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*overridenDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR values1 = contentSet.Get(0)->GetValues();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({"CalculatedProperty_0": "Test1Calculated"})").c_str());
    EXPECT_EQ(expectedValues1, values1)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(values1);

    RapidJsonValueCR values2 = contentSet.Get(1)->GetValues();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({"CalculatedProperty_0": "Test2Calculated"})").c_str());
    EXPECT_EQ(expectedValues2, values2)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(values2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_GetDistinctValuesOfDisplayLabel, R"*(
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="C" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_GetDistinctValuesOfDisplayLabel)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test2"));});
    IECInstancePtr instanceA3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("Test1"));});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceB, *instanceA1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceB, *instanceA2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceB, *instanceA3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* specificSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*specificSpec);
    ContentRelatedInstancesSpecification* relatedSpec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection::RequiredRelationDirection_Both, relAB->GetFullName(), classA->GetFullName());
    contentRule->AddSpecification(*relatedSpec);
    SelectedNodeInstancesSpecification* selectedSpec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*selectedSpec);
    rules->AddPresentationRule(*contentRule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property"));

    KeySetPtr keys = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceA1, instanceA2, instanceA3});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *keys)));

    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    overridenDescriptor->ExclusivelyIncludeFields({ overridenDescriptor->FindField(GenericContentFieldMatcher([](auto const& field){return field.IsDisplayLabelField();})) });
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);
    overridenDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(0, overridenDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*overridenDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ("Test1", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("Test2",  contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_GetDistinctValuesFromMergedField, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DEPRECATED_GetDistinctValuesFromMergedField)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2, instance3, instance4});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    overridenDescriptor->ExclusivelyIncludeFields({ overridenDescriptor->FindField(PropertiesContentFieldMatcher(*classA->GetPropertyP("Property"), {})) });
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // request for content
    ContentCPtr content = GetVerifiedContent(*overridenDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetValues();
    EXPECT_STREQ("InstanceA", jsonValues[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "Property")].GetString());
    RapidJsonValueCR jsonValues2 = contentSet.Get(1)->GetValues();
    EXPECT_STREQ("InstanceB", jsonValues2[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "Property")].GetString());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctDisplayLabelValues, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctDisplayLabelValues)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Label"));

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", (int)ContentFlags::ShowLabels, *KeySet::Create())));
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(),
        *descriptor, std::make_unique<NamedContentFieldMatcher>(ContentDescriptor::DisplayLabelField::NAME))));

    // validate
    ASSERT_EQ(2, values.GetSize());

    EXPECT_STREQ("A", values[0]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[0]->GetRawValues().size());
    EXPECT_EQ(rapidjson::Value("A"), values[0]->GetRawValues()[0]);

    EXPECT_STREQ("B", values[1]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[1]->GetRawValues().size());
    EXPECT_EQ(rapidjson::Value("B"), values[1]->GetRawValues()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctCalculatedValues, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Width" typeName="int" />
        <ECProperty propertyName="Length" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctCalculatedValues)
    {
    // set up data
    ECClassCP classA = GetClass("A");

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Width", ECValue(1)); instance.SetValue("Length", ECValue(4));}); // Area = 4
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Width", ECValue(2)); instance.SetValue("Length", ECValue(3));}); // Area = 6
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Width", ECValue(3)); instance.SetValue("Length", ECValue(2));}); // Area = 6
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Width", ECValue(4)); instance.SetValue("Length", ECValue(1));}); // Area = 4

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("Area", 1000, "this.Width * this.Length"));
    rules->AddPresentationRule(*modifier);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ContentDescriptor::Field const* calculatedField = descriptor->GetAllFields().back();
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(),
        *descriptor, std::make_unique<NamedContentFieldMatcher>(calculatedField->GetUniqueName()))));

    // validate
    ASSERT_EQ(2, values.GetSize());

    EXPECT_STREQ("4", values[0]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[0]->GetRawValues().size());
    EXPECT_EQ(rapidjson::Value("4"), values[0]->GetRawValues()[0]);

    EXPECT_STREQ("6", values[1]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[1]->GetRawValues().size());
    EXPECT_EQ(rapidjson::Value("6"), values[1]->GetRawValues()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctPrimitiveValues, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctPrimitiveValues)
    {
    // set up the dataset
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("Test1"));});
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("Test1"));});
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue("Test2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(),
        *descriptor, std::make_unique<PropertiesContentFieldMatcher>(*classA->GetPropertyP("Prop"), RelatedClassPath()))));

    // validate
    ASSERT_EQ(2, values.GetSize());

    EXPECT_STREQ("Test1", values[0]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[0]->GetRawValues().size());
    EXPECT_EQ(rapidjson::Value("Test1"), values[0]->GetRawValues()[0]);

    EXPECT_STREQ("Test2", values[1]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[1]->GetRawValues().size());
    EXPECT_EQ(rapidjson::Value("Test2"), values[1]->GetRawValues()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctNavigationPropertyValues, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECNavigationProperty propertyName="A" relationshipName="A_To_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassC Uses ClassA" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassA is used by ClassC" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctNavigationPropertyValues)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A1"));});
    IECInstancePtr b11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b11);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b12);

    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A2"));});
    IECInstancePtr a22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A2"));});
    IECInstancePtr b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a21, *b21);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a22, *b22);

    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A3"));});
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a3, *b3);

    IECInstancePtr b4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create a ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), true, false));
    ruleset->AddPresentationRule(*rule);

    ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Label"));

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(),
        *descriptor, std::make_unique<PropertiesContentFieldMatcher>(*classB->GetPropertyP("A"), RelatedClassPath()))));

    // validate
    ASSERT_EQ(4, values.GetSize());

    EXPECT_STREQ("", values[0]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[0]->GetRawValues().size());
    EXPECT_TRUE(values[0]->GetRawValues()[0].IsNull());

    EXPECT_STREQ("A1", values[1]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[1]->GetRawValues().size());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*a1).GetClass()->GetId().ToString(), values[1]->GetRawValues()[0]["ECClassId"].GetString());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*a1).GetId().ToString(), values[1]->GetRawValues()[0]["ECInstanceId"].GetString());

    EXPECT_STREQ("A2", values[2]->GetDisplayValue().c_str());
    EXPECT_EQ(2, values[2]->GetRawValues().size());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*a21).GetClass()->GetId().ToString(), values[2]->GetRawValues()[0]["ECClassId"].GetString());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*a21).GetId().ToString(), values[2]->GetRawValues()[0]["ECInstanceId"].GetString());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*a22).GetClass()->GetId().ToString(), values[2]->GetRawValues()[1]["ECClassId"].GetString());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*a22).GetId().ToString(), values[2]->GetRawValues()[1]["ECInstanceId"].GetString());

    EXPECT_STREQ("A3", values[3]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[3]->GetRawValues().size());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*a3).GetClass()->GetId().ToString(), values[3]->GetRawValues()[0]["ECClassId"].GetString());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*a3).GetId().ToString(), values[3]->GetRawValues()[0]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctXToOneRelatedPrimitivePropertyValues, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctXToOneRelatedPrimitivePropertyValues)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_To_B")->GetRelationshipClassCP();

    // case: two source elements have the same related element
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A1")); });
    IECInstancePtr b11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b11);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b12);

    // case: two source elements have two different related elements, but the value is the same
    IECInstancePtr a21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A2")); });
    IECInstancePtr a22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A2")); });
    IECInstancePtr b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a21, *b21);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a22, *b22);

    // case: one source element has one related element
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A3")); });
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a3, *b3);

    // case: no related element
    IECInstancePtr b4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create a ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);

    ContentRuleP rule = new ContentRule("", 1, false);
    auto spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), true, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Backward)),
        {new PropertySpecification("Label")}, RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);
    ruleset->AddPresentationRule(*rule);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    auto fieldMatcher = std::make_shared<PropertiesContentFieldMatcher>(
        *classA->GetPropertyP("Label"),
        RelatedClassPath { RelatedClass(*classB, SelectClass<ECRelationshipClass>(*rel, ""), false, SelectClass<ECClass>(*classA, "")) }
        );
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(), *descriptor, fieldMatcher)));

    // validate
    ASSERT_EQ(4, values.GetSize());
    size_t valueIndex = 0;

    EXPECT_STREQ("", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_TRUE(values[valueIndex]->GetRawValues()[0].IsNull());
    ++valueIndex;

    EXPECT_STREQ("A1", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("A1", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("A2", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("A2", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("A3", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("A3", values[valueIndex]->GetRawValues()[0].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctXToManyRelatedPrimitivePropertyValues, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctXToManyRelatedPrimitivePropertyValues)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_To_B")->GetRelationshipClassCP();

    // case: one source element has two related elements with the same value
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B1")); });
    IECInstancePtr b12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B1")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b11);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b12);

    // case: one source element has two related elements with different values
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B21")); });
    IECInstancePtr b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B22")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *b21);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *b22);

    // case: two source elements have different related elements with the same value
    IECInstancePtr a31 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a32 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b31 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B3")); });
    IECInstancePtr b32 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B3")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a31, *b31);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a32, *b32);

    // case: one source element has one related element
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B4")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a4, *b4);

    // case: no related element
    IECInstancePtr a5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create a ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);

    ContentRuleP rule = new ContentRule("", 1, false);
    auto spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)),
        {new PropertySpecification("Label")}, RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);
    ruleset->AddPresentationRule(*rule);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    auto fieldMatcher = std::make_shared<PropertiesContentFieldMatcher>(
        *classB->GetPropertyP("Label"),
        RelatedClassPath { RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*classB, "b")) }
        );
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(), *descriptor, fieldMatcher)));

    // validate
    ASSERT_EQ(6, values.GetSize());
    size_t valueIndex = 0;

    EXPECT_STREQ("", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_TRUE(values[valueIndex]->GetRawValues()[0].IsNull());
    ++valueIndex;

    EXPECT_STREQ("B1", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("B1", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("B21", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("B21", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("B22", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("B22", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("B3", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("B3", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("B4", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("B4", values[valueIndex]->GetRawValues()[0].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctXToManyMultiStepRelatedPrimitivePropertyValues, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="bc" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctXToManyMultiStepRelatedPrimitivePropertyValues)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_To_C")->GetRelationshipClassCP();

    // case: one related element per nesting level
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("C1")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relBC, *b1, *c1);

    // case: one related element on first level, two related elements on second level (same value)
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("C2")); });
    IECInstancePtr c22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("C2")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relBC, *b2, *c21);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relBC, *b2, *c22);

    // case: one related element on first level, two related elements on second level (different values)
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c31 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("C31")); });
    IECInstancePtr c32 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("C32")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a3, *b3);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relBC, *b3, *c31);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relBC, *b3, *c32);

    // case: diamond
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b41 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b42 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("C4")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a4, *b41);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a4, *b42);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relBC, *b41, *c4);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relBC, *b42, *c4);

    // case: no second relationship
    IECInstancePtr a5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *a5, *b5);

    // case: no first or second relationship
    IECInstancePtr a6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create a ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);

    ContentRuleP rule = new ContentRule("", 1, false);
    auto spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), {new PropertySpecification("Label")}, RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);
    ruleset->AddPresentationRule(*rule);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    auto fieldMatcher = std::make_shared<PropertiesContentFieldMatcher>(
        *classC->GetPropertyP("Label"),
        RelatedClassPath
            {
            RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "r1"), true, SelectClass<ECClass>(*classB, "b")),
            RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "r2"), true, SelectClass<ECClass>(*classC, "c"))
            }
        );
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(), *descriptor, fieldMatcher)));

    // validate
    ASSERT_EQ(6, values.GetSize());
    size_t valueIndex = 0;

    EXPECT_STREQ("", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_TRUE(values[valueIndex]->GetRawValues()[0].IsNull());
    ++valueIndex;

    EXPECT_STREQ("C1", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("C1", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("C2", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("C2", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("C31", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("C31", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("C32", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("C32", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("C4", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("C4", values[valueIndex]->GetRawValues()[0].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctValuesOfCorrectRelatedPropertyWhenThereAreMultipleSimilarPropertiesInDescriptor, R"*(
    <ECEntityClass typeName="Base">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="A">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctValuesOfCorrectRelatedPropertyWhenThereAreMultipleSimilarPropertiesInDescriptor)
    {
    // set up dataset
    ECClassCP classBase = GetClass("Base");
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_To_B")->GetRelationshipClassCP();

    // case: two source elements have two different related elements, but the value is the same
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A1")); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("A2")); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B1")); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("B2")); });
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *a2, *b2);

    // create a ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);

    ContentRuleP rule = new ContentRule("", 1, false);
    auto spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward)),
        {new PropertySpecification("Label")}, RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);
    ruleset->AddPresentationRule(*rule);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    auto fieldMatcher = std::make_shared<PropertiesContentFieldMatcher>(
        *classBase->GetPropertyP("Label"),
        RelatedClassPath { RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*classB, "b")) }
        );
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(), *descriptor, fieldMatcher)));

    // validate
    ASSERT_EQ(2, values.GetSize());
    size_t valueIndex = 0;
    EXPECT_STREQ("B1", values[valueIndex++]->GetDisplayValue().c_str());
    EXPECT_STREQ("B2", values[valueIndex++]->GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctRelatedValuesWhenPathFromSelectToPropertyClassSpecifiesBaseSourceClass, R"*(
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
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AC" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ac" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ca" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctRelatedValuesWhenPathFromSelectToPropertyClassSpecifiesBaseSourceClass)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAC = GetClass("AC")->GetRelationshipClassCP();

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Label", ECValue("C"));});
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAC, *b, *c);

    // create a ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);

    ContentRuleP rule = new ContentRule("", 1, false);
    auto spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, true);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*new RelationshipStepSpecification(relAC->GetFullName(), RequiredRelationDirection_Forward)),
        { new PropertySpecification("Label") }, RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);
    ruleset->AddPresentationRule(*rule);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    auto fieldMatcher = std::make_shared<PropertiesContentFieldMatcher>(
        *classC->GetPropertyP("Label"),
        RelatedClassPath { RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAC, "r"), true, SelectClass<ECClass>(*classC, "c")) }
        );
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(), *descriptor, fieldMatcher)));

    // validate
    ASSERT_EQ(1, values.GetSize());
    size_t valueIndex = 0;
    EXPECT_STREQ("C", values[valueIndex++]->GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctValuesWhenThereAreMultipleSelectClasses, R"*(
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
        <ECProperty propertyName="X" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctValuesWhenThereAreMultipleSelectClasses)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("X", ECValue("xxx"));});

    // create a ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*ruleset);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, true));
    ruleset->AddPresentationRule(*rule);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(),
        *descriptor, std::make_unique<PropertiesContentFieldMatcher>(*classC->GetPropertyP("X"), RelatedClassPath()))));

    // validate
    ASSERT_EQ(1, values.GetSize());
    size_t valueIndex = 0;
    EXPECT_STREQ("xxx", values[valueIndex++]->GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctPrimitiveValuesFromMergedField, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctPrimitiveValuesFromMergedField)
    {
    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("Label1"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("Label2"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("Label3"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("Label1"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("Label2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false));
    rules->AddPresentationRule(*contentRule);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(),
        *descriptor, std::make_unique<PropertiesContentFieldMatcher>(*classA->GetPropertyP("Label"), RelatedClassPath()))));

    // validate
    ASSERT_EQ(3, values.GetSize());
    size_t valueIndex = 0;

    EXPECT_STREQ("Label1", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("Label1", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("Label2", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("Label2", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("Label3", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("Label3", values[valueIndex]->GetRawValues()[0].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctPrimitiveValuesFromMergedFieldWhenFormatterFormatsThemDifferently, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctPrimitiveValuesFromMergedFieldWhenFormatterFormatsThemDifferently)
    {
    // set up manager with a custom formatter
    TestPropertyFormatter formatter;
    formatter.SetValueFormatter([](Utf8StringR result, ECPropertyCR prop, ECValueCR value, ECPresentation::UnitSystem)
        {
        result = Utf8PrintfString("%s-%s", prop.GetClass().GetName().c_str(), value.ToString().c_str());
        return SUCCESS;
        });
    ECPresentationManager::Params managerParams = CreateManagerParams();
    managerParams.SetECPropertyFormatter(&formatter);
    ReCreatePresentationManager(managerParams);

    // set up dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("Label"));});
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Label", ECValue("Label"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false));
    rules->AddPresentationRule(*contentRule);

    // get distinct values
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(),
        *descriptor, std::make_unique<PropertiesContentFieldMatcher>(*classB->GetPropertyP("Label"), RelatedClassPath()))));

    // validate
    ASSERT_EQ(2, values.GetSize());
    size_t valueIndex = 0;

    EXPECT_STREQ("A-Label", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("Label", values[valueIndex]->GetRawValues()[0].GetString());
    ++valueIndex;

    EXPECT_STREQ("B-Label", values[valueIndex]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[valueIndex]->GetRawValues().size());
    EXPECT_STREQ("Label", values[valueIndex]->GetRawValues()[0].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetsContentDescriptorWithNavigationPropertiesFromDifferentContentSpecifications, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECNavigationProperty propertyName="A" relationshipName="A_B" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECNavigationProperty propertyName="B" relationshipName="B_C" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetsContentDescriptorWithNavigationPropertiesFromDifferentContentSpecifications)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    // prepare dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(1));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    ContentInstancesOfSpecificClassesSpecificationP specB = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), true, false);
    ContentInstancesOfSpecificClassesSpecificationP specC = new ContentInstancesOfSpecificClassesSpecification(1, "", classC->GetFullName(), true, false);
    specB->AddPropertyOverride(*new PropertySpecification("A", 1500, "", nullptr, true));
    specC->AddPropertyOverride(*new PropertySpecification("B", 1500, "", nullptr, true));
    rule->AddSpecification(*specB);
    rule->AddSpecification(*specC);

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetValues();
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), jsonValues[FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), jsonValues[FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classC, "B")].IsNull());

    RapidJsonValueCR jsonValues2 = contentSet.Get(1)->GetValues();
    EXPECT_EQ(instanceB->GetClass().GetId().ToString(), jsonValues2[FIELD_NAME(classC, "B")]["ECClassId"].GetString());
    EXPECT_EQ(instanceB->GetInstanceId(), jsonValues2[FIELD_NAME(classC, "B")]["ECInstanceId"].GetString());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(classB, "A")].IsNull());
    }

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDistinctNavigationProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECNavigationProperty propertyName="A" relationshipName="A_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="referencing" strengthDirection="Forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDistinctNavigationProperties)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    // prepare dataset
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *instanceA1, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *relAB, *instanceA2, *instanceB);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    ContentInstancesOfSpecificClassesSpecificationP specB = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), true, false);
    specB->AddPropertyOverride(*new PropertySpecification("A", 1500, "", nullptr, true));
    rule->AddSpecification(*specB);

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // modify content descriptor
    ContentDescriptorPtr overridenDescriptor = ContentDescriptor::Create(*descriptor);
    overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

    // request for content
    ContentCPtr content = GetVerifiedContent(*overridenDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetDisplayValues();
    EXPECT_STREQ("InstanceA", jsonValues[FIELD_NAME(classB, "A")].GetString());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, GetsDistinctRelatedValueWhenThereAreManyColumnsOnSiblingRelatedClasses)
    {
    // set up the schema
    RulesEngineTestHelpers::ImportSchema(s_project->GetECDb(), [&](ECSchemaR schema)
        {
        ECEntityClassP classA = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(classA, "A"));

        ECEntityClassP classB = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(classB, "B"));
        ASSERT_TRUE(nullptr != classB);
        CreateNProperties(*classB, 1);

        ECEntityClassP classC = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(classC, "C"));
        ASSERT_TRUE(nullptr != classC);
        CreateNProperties(*classC, 1500);

        ECEntityClassP classD = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(classD, "D"));
        ASSERT_TRUE(nullptr != classD);
        CreateNProperties(*classD, 1500);

        ECRelationshipClassP relAB = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateRelationshipClass(relAB, "AB", *classA, "A", *classB, "B"));

        ECRelationshipClassP relBC = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateRelationshipClass(relBC, "BC", *classB, "B", *classC, "C"));

        ECRelationshipClassP relBD = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateRelationshipClass(relBD, "BD", *classB, "B", *classD, "D"));
        });

    auto classA = GetClass("A")->GetEntityClassCP();
    auto classB = GetClass("B")->GetEntityClassCP();
    auto classC = GetClass("C")->GetEntityClassCP();
    auto classD = GetClass("D")->GetEntityClassCP();
    auto relAB = GetClass("AB")->GetRelationshipClassCP();
    auto relBC = GetClass("BC")->GetRelationshipClassCP();
    auto relBD = GetClass("BD")->GetRelationshipClassCP();

    // insert an instance of each class
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBD, *b, *d);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    auto rule = new ContentRule();
    auto spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, true);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance));
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward)
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance));
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        new RelationshipStepSpecification(relBD->GetFullName(), RequiredRelationDirection_Forward)
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance));
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    auto fieldMatcher = std::make_shared<PropertiesContentFieldMatcher>(
        *GetClass("C")->GetPropertyP("Prop999"),
        RelatedClassPath
            {
            RelatedClass(*GetClass("A"), SelectClass<ECRelationshipClass>(*GetClass("AB")->GetRelationshipClassCP(), ""), true, SelectClass<ECClass>(*GetClass("B"), "")),
            RelatedClass(*GetClass("B"), SelectClass<ECRelationshipClass>(*GetClass("BC")->GetRelationshipClassCP(), ""), true, SelectClass<ECClass>(*GetClass("C"), "")),
            }
        );
    PagedDataContainer<DisplayValueGroupCPtr> values = GetValidatedResponse(m_manager->GetDistinctValues(AsyncDistinctValuesRequestParams::Create(s_project->GetECDb(), *descriptor, fieldMatcher)));

    ASSERT_EQ(1, values.GetSize());
    EXPECT_STREQ("", values[0]->GetDisplayValue().c_str());
    EXPECT_EQ(1, values[0]->GetRawValues().size());
    EXPECT_TRUE(values[0]->GetRawValues()[0].IsNull());
    }
