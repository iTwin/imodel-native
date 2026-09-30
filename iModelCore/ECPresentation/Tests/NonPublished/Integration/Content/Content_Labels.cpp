/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesRelatedInstanceInLabelOverrideCondition, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, UsesRelatedInstanceInLabelOverrideCondition)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("A label"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("B label"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\" ANDALSO bAlias.Property = \"B label\"", 1, "this.Property", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification(*new RelationshipStepSpecification(relationshipAHasB->GetFullName(), RequiredRelationDirection_Forward, classB->GetFullName())), "bAlias"));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("A label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_UsesRelatedInstanceInLabelOverrideExpression, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_UsesRelatedInstanceInLabelOverrideExpression)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [] (IECInstanceR instance) { instance.SetValue("Property", ECValue("A label")); });
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("B label"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\"", 1, "bAlias.Property", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relationshipAHasB->GetFullName(), classB->GetFullName(), "bAlias"));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("B label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_SetsDisplayLabelProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_SetsDisplayLabelProperty)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("Custom label"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreEqual, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreEqual)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("Custom label"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("Custom label"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\"", 1, "this.Property", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    int contentFlags = (int)ContentFlags::ShowLabels | (int)ContentFlags::MergeResults;
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, contentFlags, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreEqual, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreEqual)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("Custom label"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("Custom label"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    int contentFlags = (int)ContentFlags::ShowLabels | (int)ContentFlags::MergeResults;
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, contentFlags, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreDifferent, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreDifferent)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("Custom label 1"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("Custom label 2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\"", 1, "this.Property", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    int contentFlags = (int)ContentFlags::ShowLabels | (int)ContentFlags::MergeResults;
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, contentFlags, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(CommonStrings::LABEL_MULTIPLEINSTANCES, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreDifferent, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_SetsDisplayLabelPropertyWhenMergingRecordsAndLabelsAreDifferent)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("Custom label 1"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA,
        [](IECInstanceR instance){instance.SetValue("Property", ECValue("Custom label 2"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    int contentFlags = (int)ContentFlags::ShowLabels | (int)ContentFlags::MergeResults;
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, contentFlags, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(CommonStrings::LABEL_MULTIPLEINSTANCES, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsDisplayLabelPropertyWhenMergingRecordsAndClassesAreDifferent, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsDisplayLabelPropertyWhenMergingRecordsAndClassesAreDifferent)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    int contentFlags = (int)ContentFlags::ShowLabels | (int)ContentFlags::MergeResults;
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, contentFlags, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(CommonStrings::LABEL_MULTIPLEINSTANCES, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LabelOverride_DisplayLabelGetCreatedFromDifferentSpecifications, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LabelOverride_DisplayLabelGetCreatedFromDifferentSpecifications)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB,
        [](IECInstanceR instance){instance.SetValue("SharedProperty", ECValue("Test"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"B\"", 1, "this.SharedProperty", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false));
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); // no display label in the descriptor

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(3, content->GetDescriptor().GetVisibleFields().size()); // content created with a descriptor that has a display label

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ("Test", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_DisplayLabelGetCreatedFromDifferentSpecifications, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_DisplayLabelGetCreatedFromDifferentSpecifications)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB,
        [](IECInstanceR instance){instance.SetValue("SharedProperty", ECValue("Test"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "SharedProperty"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false));
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); // no display label in the descriptor

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(3, content->GetDescriptor().GetVisibleFields().size()); // content created with a descriptor that has a display label

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ("Test", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DisplayLabelGetsCreatedFromDifferentSpecificationsWhenCategorizingFields, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" category="TestCategory" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DisplayLabelGetsCreatedFromDifferentSpecificationsWhenCategorizingFields)
    {
    // set up the dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB,
        [](IECInstanceR instance){instance.SetValue("PropB", ECValue("test value"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "PropB"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_TRUE(nullptr != descriptor->GetDisplayLabelField());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    auto field1 = descriptor->GetDisplayLabelField();
    EXPECT_STREQ(ContentDescriptor::DisplayLabelField::NAME, field1->GetUniqueName().c_str());
    EXPECT_STREQ(DefaultCategorySupplier().CreateDefaultCategory()->GetName().c_str(), field1->GetCategory()->GetName().c_str());

    auto field2 = descriptor->GetVisibleFields()[0];
    EXPECT_STREQ("PropB", field2->GetLabel().c_str());
    EXPECT_STREQ("TestCategory", field2->GetCategory()->GetName().c_str());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    ContentSetItemCPtr record1 = contentSet.Get(0);
    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, record1->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    ContentSetItemCPtr record2 = contentSet.Get(1);
    EXPECT_STREQ("test value", record2->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_OverridesInstanceLabelOfSpecifiedClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_OverridesInstanceLabelOfSpecifiedClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property1", ECValue("InstanceB"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property1", ECValue("InstanceA"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property1,Property2"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceB).c_str(), contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("InstanceB", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_OverridesInstanceLabelOfSpecifiedClassesWithDifferentProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_OverridesInstanceLabelOfSpecifiedClassesWithDifferentProperties)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property1", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property2", ECValue("InstanceA"));});
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("Property1", ECValue("InstanceC"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Property1,Property2"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property2,Property1"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:A,B,C", GetSchema()->GetName().c_str()), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());
    EXPECT_STREQ("InstanceA", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("InstanceB", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(GetDefaultDisplayLabel(*sprocket).c_str(), contentSet.Get(2)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_AppliedByPriority, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_AppliedByPriority)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property1", ECValue("TestValue1"));
        instance.SetValue("Property2", ECValue("TestValue2"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "Property1"));
    rules->AddPresentationRule(*new InstanceLabelOverride(2, false, classA->GetFullName(), "Property2"));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "",  Utf8PrintfString("%s:A", GetSchema()->GetName().c_str()), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("TestValue2", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_WhenNoPropertiesSpecified_FallsBackToLabelOverrides, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_WhenNoPropertiesSpecified_FallsBackToLabelOverrides)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"LabelOverride\"", ""));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("LabelOverride", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_AssertsWhenNoClasesSpecified_FallsBackToLabelOverrides, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_AssertsWhenNoClasesSpecified_FallsBackToLabelOverrides)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("", 1, "\"LabelOverride\"", ""));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, "", ""));

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("LabelOverride", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetElementIdSuffix(IECInstanceCR element)
    {
    ECInstanceId id;
    ECInstanceId::FromString(id, element.GetInstanceId().c_str());
    uint64_t briefcaseId = id.GetValue() >> 40;
    uint64_t localId = id.GetValue() & (((uint64_t)1 << 40) - 1);
    return Utf8PrintfString("[%s-%s]", CommonTools::ToBase36String(briefcaseId).c_str(), CommonTools::ToBase36String(localId).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstanceLabelOverride_HandlesDefaultBisRulesCorrectlyForContent, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, InstanceLabelOverride_HandlesDefaultBisRulesCorrectlyForContent)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP geometricElementClass = GetClass("GeometricElement");
    ECClassCP customElementClass = GetClass("CustomElement");
    ECClassCP customGeometricElementClass = GetClass("CustomGeometricElement");

    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("UserLabel", ECValue("UserLabel"));
        });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("CodeValue", ECValue("CodeValue"));
        });
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customElementClass);
    IECInstancePtr geometricElement1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customGeometricElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("CodeValue", ECValue("CodeValue"));
        });
    IECInstancePtr geometricElement2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *customGeometricElementClass, [](IECInstanceR instance)
        {
        instance.SetValue("UserLabel", ECValue("UserLabel"));
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

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(6, contentSet.GetSize());

    EXPECT_STREQ("UserLabel", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("CodeValue", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(Utf8String("Custom Element ").append(GetElementIdSuffix(*element3)).c_str(), contentSet.Get(2)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ("CodeValue", contentSet.Get(3)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(Utf8String("UserLabel ").append(GetElementIdSuffix(*geometricElement2)).c_str(), contentSet.Get(4)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(Utf8String("Custom Geometric Element ").append(GetElementIdSuffix(*geometricElement3)).c_str(), contentSet.Get(5)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceLabelIsOverridenByParentClassProperty, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <ECNavigationProperty propertyName="A" relationshipName="ClassCUsesClassA" direction="Forward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassCUsesClassA" strength="referencing" strengthDirection="backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassC Uses ClassA" polymorphic="True">
            <Class class="ClassC" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassA is used by ClassC" polymorphic="True">
            <Class class="ClassA" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, RelatedInstanceLabelIsOverridenByParentClassProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassB_StringProperty"));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    ECRelationshipClassCP classCUsesClassA = GetClass("ClassCUsesClassA")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classCUsesClassA, *instanceC, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, classCUsesClassA->GetFullName(), classB->GetFullName()));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "BaseStringProperty"));
    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassB_StringProperty", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingBaseClassPolymorphicallyInstanceLabelOverrideAppliedToSpecifiedClass, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingBaseClassPolymorphicallyInstanceLabelOverrideAppliedToSpecifiedClass)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassB_StringProperty"));});
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassA_StringProperty"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "BaseStringProperty"));
    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("ClassB_StringProperty", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsNotSpecifiedLabelWhenUsingInstanceLabelOverrideWithEmptyStringPartsInCompositeSpecification, R"*(
    <ECEntityClass typeName="ClassA"/>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNotSpecifiedLabelWhenUsingInstanceLabelOverrideWithEmptyStringPartsInCompositeSpecification)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(),
        { new InstanceLabelOverrideCompositeValueSpecification(
            { new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideStringValueSpecification("")) })
        }));
    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_TRUE(descriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsNotSpecifiedLabelWhenUsingInstanceLabelOverrideWithoutAnyPartsInCompositeSpecification, R"*(
    <ECEntityClass typeName="ClassA"/>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNotSpecifiedLabelWhenUsingInstanceLabelOverrideWithoutAnyPartsInCompositeSpecification)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), { new InstanceLabelOverrideCompositeValueSpecification() }));
    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_TRUE(descriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsNotSpecifiedLabelWhenUsingInstanceLabelOverrideWithEmptyStringSpecification, R"*(
    <ECEntityClass typeName="ClassA"/>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNotSpecifiedLabelWhenUsingInstanceLabelOverrideWithEmptyStringSpecification)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), { new InstanceLabelOverrideStringValueSpecification("") }));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_TRUE(descriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsNotSpecifiedLabelWhenUsingInstanceLabelOverrideWithEmptyPropertySpecification, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNotSpecifiedLabelWhenUsingInstanceLabelOverrideWithEmptyPropertySpecification)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("BaseStringProperty", ECValue(""));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("BaseStringProperty") }));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_TRUE(descriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsPointValueLabelWhenUsingRelationshipPathSpecificationInstanceLabelOverrideWithPoint3dProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PointProperty" typeName="point3d" />
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
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPointValueLabelWhenUsingRelationshipPathSpecificationInstanceLabelOverrideWithPoint3dProperty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.SetValue("PointProperty", ECValue(DPoint3d::From(1, 2, 3)));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(),
        {
        new InstanceLabelOverridePropertyValueSpecification("PointProperty", RelationshipPathSpecification(
            {
            new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward),
            })),
        }));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_TRUE(descriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("X: 1.00; Y: 2.00; Z: 3.00", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsPointValueLabelWhenUsingInstanceLabelOverrideWithPoint3dProperty, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="PointProperty" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPointValueLabelWhenUsingInstanceLabelOverrideWithPoint3dProperty)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("PointProperty", ECValue(DPoint3d::From(1, 2, 3)));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("PointProperty") }));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_TRUE(descriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("X: 1.00; Y: 2.00; Z: 3.00", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsPointValueLabelWhenUsingInstanceLabelOverrideWithPoint2dProperty, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="PointProperty" typeName="point2d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPointValueLabelWhenUsingInstanceLabelOverrideWithPoint2dProperty)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("PointProperty", ECValue(DPoint2d::From(1, 2)));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), { new InstanceLabelOverridePropertyValueSpecification("PointProperty") }));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_TRUE(descriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("X: 1.00; Y: 2.00", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingBaseClassPolymorphicallyChildrenLabelsAreOverriden, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
        <ECProperty propertyName="ClassC_String" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingBaseClassPolymorphicallyChildrenLabelsAreOverriden)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassB_BaseStringProperty"));});
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassA_BaseStringProperty"));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance)
        {
        instance.SetValue("ClassC_String", ECValue("ClassC_StringProperty"));
        instance.SetValue("BaseStringProperty", ECValue("ClassC_BaseStringProperty"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "ClassC_String"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "BaseStringProperty"));
    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    EXPECT_STREQ("ClassB_BaseStringProperty", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("ClassA_BaseStringProperty", contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("ClassC_StringProperty", contentSet.Get(2)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelOverridesAppliedPolymorphically, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
        <ECProperty propertyName="ClassC_String" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelOverridesAppliedPolymorphically)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassC_BaseStringProperty"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classC->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "BaseStringProperty"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassC_BaseStringProperty", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelOverridesAppliedPolymorphically_MultipleInheritance, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelOverridesAppliedPolymorphically_MultipleInheritance)
    {
    // set up data set
    ECClassCP classA1 = GetClass("ClassA1");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("ClassC_CodeValue"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classC->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA1->GetFullName(), "CodeValue"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassC_CodeValue", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelOverridesAppliedPolymorphically_MultipleInheritance_TakesBaseClassProperty, R"*(
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
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelOverridesAppliedPolymorphically_MultipleInheritance_TakesBaseClassProperty)
    {
    // set up data set
    ECClassCP classA1 = GetClass("ClassA1");
    ECClassCP classB = GetClass("ClassB");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("ClassC_CodeValue"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classC->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classA1->GetFullName(), "CodeValue"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), "UserLabel"));
    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassC_CodeValue", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NavigationPropertyLabelIsOverridenByTargetClassProperty, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <ECNavigationProperty propertyName="A" relationshipName="ClassCUsesClassA" direction="Forward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassCUsesClassA" strength="referencing" strengthDirection="backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassC Uses ClassA" polymorphic="True">
            <Class class="ClassC" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassA is used by ClassC" polymorphic="True">
            <Class class="ClassA" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, NavigationPropertyLabelIsOverridenByTargetClassProperty)
    {
    // set up data set
    ECClassCP classB = GetClass("ClassB");
    ECClassCP classC = GetClass("ClassC");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassB_StringProperty"));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    ECRelationshipClassCP classCUsesClassA = GetClass("ClassCUsesClassA")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classCUsesClassA, *instanceC, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classC->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "BaseStringProperty"));
    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetDisplayValues();
    EXPECT_STREQ("ClassB_StringProperty", jsonValues[FIELD_NAME(classC, "A")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NavigationPropertyLabelIsOverridenPolymorphically, R"*(
    <ECEntityClass typeName="ClassA">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="BaseStringProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>ClassA</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>ClassB</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ClassD">
        <ECNavigationProperty propertyName="A" relationshipName="ClassDUsesClassA" direction="Forward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassDUsesClassA" strength="referencing" strengthDirection="backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassC Uses ClassA" polymorphic="True">
            <Class class="ClassD" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassA is used by ClassC" polymorphic="True">
            <Class class="ClassA" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, NavigationPropertyLabelIsOverridenPolymorphically)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classC = GetClass("ClassC");
    ECClassCP classD = GetClass("ClassD");
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("BaseStringProperty", ECValue("ClassC_StringProperty"));});
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    ECRelationshipClassCP classDUsesClassA = GetClass("ClassDUsesClassA")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDUsesClassA, *instanceD, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classD->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "BaseStringProperty"));
    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    RapidJsonValueCR jsonValues = contentSet.Get(0)->GetDisplayValues();
    EXPECT_STREQ("ClassC_StringProperty", jsonValues[FIELD_NAME(classD, "A")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelUsingInstanceLabelOverrideAndGetRelatedInstanceLabelExpression, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelUsingInstanceLabelOverrideAndGetRelatedInstanceLabelExpression)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassB_UserLabel"));});
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasClassB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "UserLabel"));
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Forward\", \"%s\")", classAHasClassB->GetFullName(), classB->GetFullName()), ""));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassB_UserLabel", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelUsingLabelOverrideAndGetRelatedInstanceLabelExpression, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelUsingLabelOverrideAndGetRelatedInstanceLabelExpression)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassB_UserLabel"));});
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasClassB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classB->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, "this.UserLabel", ""));
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Forward\", \"%s\")", classAHasClassB->GetFullName(), classB->GetFullName()), ""));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassB_UserLabel", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelBackwardUsingLabelOverrideAndGetRelatedInstanceLabelExpression, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelBackwardUsingLabelOverrideAndGetRelatedInstanceLabelExpression)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassB_UserLabel"));});
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasClassB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, "this.UserLabel", ""));
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classB->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Backward\", \"%s\")", classAHasClassB->GetFullName(), classA->GetFullName()), ""));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("ClassA_UserLabel", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelUsingGetRelatedInstanceLabelExpression_NoRelatedLabelOverrideRulesFound_ReturnsNotSpecifiedString, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelUsingGetRelatedInstanceLabelExpression_NoRelatedLabelOverrideRulesFound_ReturnsNotSpecifiedString)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassB_UserLabel"));});
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasClassB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Forward\", \"%s\")", classAHasClassB->GetFullName(), classB->GetFullName()), ""));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    // Expecting "Not specified" label
    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectingClassInstanceLabelUsingGetRelatedInstanceLabelExpression_NoRelatedInstanceFound_ReturnsNotSpecifiedString, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasClassB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ClassAHasClassB" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ClassAHasClassB (reversed)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectingClassInstanceLabelUsingGetRelatedInstanceLabelExpression_NoRelatedInstanceFound_ReturnsNotSpecifiedString)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP classAHasClassB = GetClass("ClassAHasClassB")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("UserLabel", ECValue("ClassA_UserLabel"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classB->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, "this.UserLabel", ""));
    rules->AddPresentationRule(*new LabelOverride(Utf8PrintfString("ThisNode.IsInstanceNode ANDALSO this.IsOfClass(\"%s\",\"%s\")", classA->GetName().c_str(), GetSchema()->GetName().c_str()),
        1, Utf8PrintfString("this.GetRelatedDisplayLabel(\"%s\", \"Forward\", \"%s\")", classAHasClassB->GetFullName(), classB->GetFullName()), ""));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

#ifdef wip_related_content_without_instances
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsEmptyRelatedContentForMultipleInstancesOfDifferentUnrelatedClassesWhenTheyDontHaveRelatedPropertyInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="C_D" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by" polymorphic="true">
            <Class class="D"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsEmptyRelatedContentForMultipleInstancesOfDifferentUnrelatedClassesWhenTheyDontHaveRelatedPropertyInstances)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP relCD = GetClass("C_D")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:%s,%s,%s",
        GetSchema()->GetName().c_str(), classA->GetName().c_str(), classB->GetName().c_str(), classC->GetName().c_str()), false, false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classC->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relCD->GetFullName(), RequiredRelationDirection_Forward)
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 3 content set items
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": []
        })",
        NESTED_CONTENT_FIELD_NAME(classC, classD)
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(1)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": []
        })",
        NESTED_CONTENT_FIELD_NAME(classC, classD)
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    recordJson = contentSet.Get(2)->AsJson();
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": []
        })",
        NESTED_CONTENT_FIELD_NAME(classC, classD)
    ).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsDisplayLabelOfSingleInstance, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="DisplayLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsDisplayLabelOfSingleInstance)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("DisplayLabel", ECValue("abc"));
        });

    // create key
    ECInstanceKey key(elementClass->GetId(), RulesEngineTestHelpers::GetInstanceKey(*element).GetId());

    // get label
    LabelDefinitionCPtr label = GetValidatedResponse(m_manager->GetDisplayLabel(AsyncECInstanceDisplayLabelRequestParams::Create(s_project->GetECDb(), key)));

    // verify
    EXPECT_STREQ("abc", label->GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsDisplayLabelOfMultipleInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="DisplayLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsDisplayLabelOfMultipleInstances)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("DisplayLabel", ECValue("abc"));
        });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.SetValue("DisplayLabel", ECValue("def"));
        });

    // create keys
    KeySetPtr keys = KeySet::Create(
        bvector<ECClassInstanceKey>{
        RulesEngineTestHelpers::GetInstanceKey(*element1),
        RulesEngineTestHelpers::GetInstanceKey(*element2)
        });

    // get label
    LabelDefinitionCPtr label = GetValidatedResponse(m_manager->GetDisplayLabel(AsyncKeySetDisplayLabelRequestParams::Create(s_project->GetECDb(), *keys)));

    // verify
    EXPECT_STREQ(CommonStrings::LABEL_MULTIPLEINSTANCES, label->GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsNotSpecifiedDisplayLabelForInstanceWithNonPrimitiveNameProperty, R"*(
    <ECEntityClass typeName="Element">
        <ECArrayProperty propertyName="Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsNotSpecifiedDisplayLabelForInstanceWithNonPrimitiveNameProperty)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("Name", 2);
        instance.SetValue("Name", ECValue("A"), 0);
        instance.SetValue("Name", ECValue("B"), 1);
        });

    // create key
    ECInstanceKey key(elementClass->GetId(), RulesEngineTestHelpers::GetInstanceKey(*element).GetId());

    // get label
    LabelDefinitionCPtr label = GetValidatedResponse(m_manager->GetDisplayLabel(AsyncECInstanceDisplayLabelRequestParams::Create(s_project->GetECDb(), key)));

    // verify
    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, label->GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsNotSpecifiedDisplayLabelForInstanceWithNonPrimitiveDisplayLabelProperty, R"*(
    <ECEntityClass typeName="Element">
        <ECArrayProperty propertyName="DisplayLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsNotSpecifiedDisplayLabelForInstanceWithNonPrimitiveDisplayLabelProperty)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("DisplayLabel", 2);
        instance.SetValue("DisplayLabel", ECValue("A"), 0);
        instance.SetValue("DisplayLabel", ECValue("B"), 1);
        });

    // create key
    ECInstanceKey key(elementClass->GetId(), RulesEngineTestHelpers::GetInstanceKey(*element).GetId());

    // get label
    LabelDefinitionCPtr label = GetValidatedResponse(m_manager->GetDisplayLabel(AsyncECInstanceDisplayLabelRequestParams::Create(s_project->GetECDb(), key)));

    // verify
    EXPECT_STREQ(CommonStrings::LABEL_NOTSPECIFIED, label->GetDisplayValue().c_str());
    }