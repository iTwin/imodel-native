/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesDescriptorWithOnlySelectClassFieldsIncludedWhenExclusiveIncludePathsAreProvidedButEmpty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesDescriptorWithOnlySelectClassFieldsIncludedWhenExclusiveIncludePathsAreProvidedButEmpty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    ContentInstancesOfSpecificClassesSpecificationP contentSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentSpec->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relationshipAHasB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance));
    rule->AddSpecification(*contentSpec);
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    // empty path means only select class fields should be included
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create(), nullptr, std::make_shared<RelatedClassPathsList>(RelatedClassPathsList()))));
    ASSERT_TRUE(descriptor.IsValid());

    auto fields = descriptor->GetVisibleFields();
    ASSERT_EQ(fields.size(), 1);
    ASSERT_STREQ(fields[0]->GetLabel().c_str(), "PropA");
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesDescriptorWithCorrectNestedClassFieldsWhenExclusiveIncludePathsAreProvided, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesDescriptorWithCorrectNestedClassFieldsWhenExclusiveIncludePathsAreProvided)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *instanceB, *instanceC);

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    ContentInstancesOfSpecificClassesSpecificationP contentSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    RelatedPropertiesSpecificationP relatedSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relationshipAHasB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance);

    // add 2 nested related properties
    // A -> B -> C
    relatedSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relationshipBHasC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance));
    // A -> B -> A
    relatedSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relationshipAHasB->GetFullName(), RequiredRelationDirection_Backward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance));
    contentSpec->AddRelatedProperty(*relatedSpec);
    rule->AddSpecification(*contentSpec);
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    // include only A -> B -> C path
    std::shared_ptr<RelatedClassPathsList> exclusiveIncludePaths = std::make_shared<RelatedClassPathsList>(RelatedClassPathsList(
        {
        RelatedClassPath(
            {
            RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relationshipAHasB, "A_B"), true, SelectClassWithExcludes<ECClass>(*classB, "B")),
            RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relationshipBHasC, "B_C"), true, SelectClassWithExcludes<ECClass>(*classC, "C"))
            }),
        }));
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create(), nullptr, exclusiveIncludePaths)));
    ASSERT_TRUE(descriptor.IsValid());

    auto fields = descriptor->GetVisibleFields();
    ASSERT_EQ(fields.size(), 2);
    ASSERT_STREQ(fields[0]->GetLabel().c_str(), "PropA");
    auto nestedFields = fields[1]->AsNestedContentField()->GetFields();
    ASSERT_EQ(nestedFields.size(), 1);
    EXPECT_STREQ(nestedFields[0]->GetLabel().c_str(), "PropC");
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesDescriptorWithCorrectNestedClassFieldsWhenExclusiveIncludePathsAreProvidedAndRulesetUsesDeprecatedRelatedPropertiesSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesDescriptorWithCorrectNestedClassFieldsWhenExclusiveIncludePathsAreProvidedAndRulesetUsesDeprecatedRelatedPropertiesSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {});

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *instanceB, *instanceC);

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentRuleP rule = new ContentRule();
    ContentInstancesOfSpecificClassesSpecificationP contentSpec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    RelatedPropertiesSpecificationP relatedSpec = new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relationshipAHasB->GetFullName(), "", "*", RelationshipMeaning::SameInstance, true);

    // add 2 nested related properties
    // A -> B -> C
    relatedSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relationshipBHasC->GetFullName(), "", "*", RelationshipMeaning::SameInstance, true));
    // A -> B -> A
    relatedSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, relationshipAHasB->GetFullName(), "", "*", RelationshipMeaning::SameInstance, true));
    contentSpec->AddRelatedProperty(*relatedSpec);
    rule->AddSpecification(*contentSpec);
    ruleset->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*ruleset);

    // include only A -> B -> C path
    std::shared_ptr<RelatedClassPathsList> exclusiveIncludePaths = std::make_shared<RelatedClassPathsList>(RelatedClassPathsList(
        {
        RelatedClassPath(
            {
            RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relationshipAHasB, "A_B"), true, SelectClassWithExcludes<ECClass>(*classB, "B")),
            RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relationshipBHasC, "B_C"), true, SelectClassWithExcludes<ECClass>(*classC, "C"))
            }),
        }));
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        ruleset->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create(), nullptr, exclusiveIncludePaths)));
    ASSERT_TRUE(descriptor.IsValid());

    auto fields = descriptor->GetVisibleFields();
    ASSERT_EQ(fields.size(), 2);
    EXPECT_STREQ(fields[0]->GetLabel().c_str(), "PropA");
    EXPECT_STREQ(fields[1]->GetLabel().c_str(), "B");
    auto nestedFields = fields[1]->AsNestedContentField()->GetFields();
    ASSERT_EQ(nestedFields.size(), 2);
    EXPECT_STREQ(nestedFields[0]->GetLabel().c_str(), "PropB");
    EXPECT_STREQ(nestedFields[1]->GetLabel().c_str(), "C");
    nestedFields = nestedFields[1]->AsNestedContentField()->GetFields();
    ASSERT_EQ(nestedFields.size(), 1);
    EXPECT_STREQ(nestedFields[0]->GetLabel().c_str(), "PropC");
    }
