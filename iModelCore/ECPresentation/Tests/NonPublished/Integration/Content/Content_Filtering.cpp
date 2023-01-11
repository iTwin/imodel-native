/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClassesSpecification_DescriptorInstanceFilter_UsesBaseClassProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecification_DescriptorInstanceFilter_UsesBaseClassProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, true));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetInstanceFilter(std::make_shared<InstanceFilterDefinition>("this.IntPropA = 2", *classA, bvector<RelatedClassPath>()));

    ContentCPtr content = GetVerifiedContent(*ovr);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{a2.get(), b2.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClassesSpecification_DescriptorInstanceFilter_UsesDerivedClassProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecification_DescriptorInstanceFilter_UsesDerivedClassProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(11)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(22)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, true));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetInstanceFilter(std::make_shared<InstanceFilterDefinition>("this.IntPropB = 22", *classB, bvector<RelatedClassPath>()));

    ContentCPtr content = GetVerifiedContent(*ovr);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{b2.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstancesSpecification_DescriptorInstanceFilter_UsesBaseClassProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstancesSpecification_DescriptorInstanceFilter_UsesBaseClassProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });

    // setup input
    KeySetPtr input = KeySet::Create({ a1.get(), a2.get(), b1.get(), b2.get() });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetInstanceFilter(std::make_shared<InstanceFilterDefinition>("this.IntPropA = 2", *classA, bvector<RelatedClassPath>()));

    ContentCPtr content = GetVerifiedContent(*ovr);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{a2.get(), b2.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstancesSpecification_DescriptorInstanceFilter_UsesDerivedClassProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstancesSpecification_DescriptorInstanceFilter_UsesDerivedClassProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(11)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(22)); });

    // setup input
    KeySetPtr input = KeySet::Create({ a1.get(), a2.get(), b1.get(), b2.get() });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetInstanceFilter(std::make_shared<InstanceFilterDefinition>("this.IntPropB = 22", *classB, bvector<RelatedClassPath>()));

    ContentCPtr content = GetVerifiedContent(*ovr);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{b2.get()}, *content, true);
    }


/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstancesSpecification_DescriptorInstanceFilter_UsesBaseClassProperty, R"*(
    <ECEntityClass typeName="X" />
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
    <ECRelationshipClass typeName="X_A" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="X"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_DescriptorInstanceFilter_UsesBaseClassProperty)
    {
    // set up data set
    ECClassCP classX = GetClass("X");
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relXA = GetClass("X_A")->GetRelationshipClassCP();
    IECInstancePtr x = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classX);
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relXA, *x, *a1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relXA, *x, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relXA, *x, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relXA, *x, *b2);

    // setup input
    KeySetPtr input = KeySet::Create({ x.get() });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentRelatedInstancesSpecification(1, false, "",
        { new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(relXA->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)) }));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetInstanceFilter(std::make_shared<InstanceFilterDefinition>("this.IntPropA = 2", *classA, bvector<RelatedClassPath>()));

    ContentCPtr content = GetVerifiedContent(*ovr);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{a2.get(), b2.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstancesSpecification_DescriptorInstanceFilter_UsesDerivedClassProperty, R"*(
    <ECEntityClass typeName="X" />
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
    <ECRelationshipClass typeName="X_A" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="X"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_DescriptorInstanceFilter_UsesDerivedClassProperty)
    {
    // set up data set
    ECClassCP classX = GetClass("X");
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relXA = GetClass("X_A")->GetRelationshipClassCP();
    IECInstancePtr x = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classX);
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(11)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(22)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relXA, *x, *a1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relXA, *x, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relXA, *x, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relXA, *x, *b2);

    // setup input
    KeySetPtr input = KeySet::Create({ x.get() });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentRelatedInstancesSpecification(1, false, "",
        { new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(relXA->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)) }));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetInstanceFilter(std::make_shared<InstanceFilterDefinition>("this.IntPropB = 22", *classB, bvector<RelatedClassPath>()));

    ContentCPtr content = GetVerifiedContent(*ovr);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{b2.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorInstanceFilter_UsesPropertyOfSpecifiedClassWhenSelectingFromOtherUnrelatedClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DescriptorInstanceFilter_UsesPropertyOfSpecifiedClassWhenSelectingFromOtherUnrelatedClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(11)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(22)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, false, "",
        bvector<MultiSchemaClass*> { new MultiSchemaClass(BeTest::GetNameOfCurrentTest(), false, bvector<Utf8String> { "A", "B" })}, bvector<MultiSchemaClass*>(), true));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetInstanceFilter(std::make_shared<InstanceFilterDefinition>("this.IntPropB = 22", *classB, bvector<RelatedClassPath>()));

    ContentCPtr content = GetVerifiedContent(*ovr);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{b2.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorInstanceFilter_UsesRelatedInstanceProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DescriptorInstanceFilter_UsesRelatedInstanceProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(11)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(22)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{a1, a2});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    RelatedClassPath relatedInstance = { RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "rel_alias_test"), true, SelectClass<ECClass>(*classB, "rel_class_B")) };
    ovr->SetInstanceFilter(std::make_shared<InstanceFilterDefinition>("rel_class_B.IntPropB = 22", *classA, bvector<RelatedClassPath>({ relatedInstance })));

    ContentCPtr content = GetVerifiedContent(*ovr);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{a2.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorInstanceFilter_UsesDerivedClassRelatedProperty, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="IntPropC" typeName="int"/>
    </ECEntityClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DescriptorInstanceFilter_UsesDerivedClassRelatedProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(11)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(22)); });
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("IntPropC", ECValue(111)); });
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) {instance.SetValue("IntPropC", ECValue(222)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, true));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    RelatedClassPath relatedInstance = { RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "rel_BC"), true, SelectClass<ECClass>(*classC, "rel_class_C")) };
    ovr->SetInstanceFilter(std::make_shared<InstanceFilterDefinition>("rel_class_C.IntPropC = 111", *classB, bvector<RelatedClassPath>({ relatedInstance })));

    ContentCPtr content = GetVerifiedContent(*ovr);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{b1.get()}, *content, true);
    }
