/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentIntegrationTests.h"

/*
             situation       | direct property               | direct categorized property     | related property                        | related property            | nested related property   | nested related property      | nested related categorized property
 option                      |                               |                                 | (same instance)                         | (related instance)          | (related + same instance) | (related + related instance) |
-----------------------------|-------------------------------|---------------------------------|-----------------------------------------|-----------------------------|---------------------------|------------------------------|-----------------------------------------
no category override         | `Selected Item(s)`            | `Selected Item(s)` -> Category  | `Selected Item(s)` -> RelatedClassLabel | RelatedClassLabel           | Class1 -> Class2          | Class1 -> Class2             | Class1 -> Category
`DefaultParent`              | `Selected Item(s)`            | `Selected Item(s)`              | `Selected Item(s)`                      | RelatedClassLabel           | Class1                    | Class1 -> Class2             | Class1
`Root`                       | `Selected Item(s)`            | `Selected Item(s)`              | `Selected Item(s)`                      | RelatedClassLabel           | Class1                    | Class1                       | Class1
`Id`                         | Custom                        | Custom                          | Custom                                  | Custom                      | Custom                    | Custom                       | Custom
`Id` (parent: DefaultParent) | `Selected Item(s)` -> Custom  | `Selected Item(s)` -> Custom    | `Selected Item(s)` -> Custom            | RelatedClassLabel -> Custom | Class1 -> Custom          | Class1 -> Class2 -> Custom   | Class1 -> Custom
`Id` (parent: Root)          | `Selected Item(s)` -> Custom  | `Selected Item(s)` -> Custom    | `Selected Item(s)` -> Custom            | RelatedClassLabel -> Custom | Class1 -> Custom          | Class1 -> Custom             | Class1 -> Custom
*/

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_NoCategory_NoOverride_GetsDefaultCategory, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_NoCategory_NoOverride_GetsDefaultCategory)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_NoCategory_WithNonExistingCustomCategoryOverride_GetsDefaultCategory, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_NoCategory_WithNonExistingCustomCategoryOverride_GetsDefaultCategory)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("doesnt-exist")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_NoCategory_WithDefaultParentCategoryOverride_GetsDefaultCategory, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_NoCategory_WithDefaultParentCategoryOverride_GetsDefaultCategory)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForDefaultParent()));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_NoCategory_WithRootCategoryOverride_GetsDefaultCategory, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_NoCategory_WithRootCategoryOverride_GetsDefaultCategory)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForRoot()));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_NoCategory_WithCustomCategoryOverride_GetsCorrectCategoryHierarchy, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_NoCategory_WithCustomCategoryOverride_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom"));
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { "custom" });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCorrectCategoryHierarchy, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForDefaultParent()));
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { "custom", DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCorrectCategoryHierarchy, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForRoot()));
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { "custom", DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_WithPropertyCategory_NoOverride_GetsECSchemaPropertyCategory, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_WithPropertyCategory_NoOverride_GetsECSchemaPropertyCategory)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { "TestCategory", DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_WithPropertyCategory_WithDefaultParentCategoryOverride_GetsDefaultCategory, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_WithPropertyCategory_WithDefaultParentCategoryOverride_GetsDefaultCategory)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForDefaultParent()));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { DefaultCategorySupplier().CreateDefaultCategory()->GetName() });

    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_WithPropertyCategory_WithRootCategoryOverride_GetsDefaultCategory, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_WithPropertyCategory_WithRootCategoryOverride_GetsDefaultCategory)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForRoot()));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_WithPropertyCategory_WithCustomCategoryOverride_GetsCorrectCategoryHierarchy, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_WithPropertyCategory_WithCustomCategoryOverride_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom"));
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { "custom" });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_WithPropertyCategory_WithCustomCategoryOverrideWithParent_GetsCorrectCategoryHierarchy, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_WithPropertyCategory_WithCustomCategoryOverrideWithParent_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForDefaultParent()));
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { "custom", DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_DirectProperty_WithPropertyCategory_WithCustomCategoryOverrideWithRootParent_GetsCorrectCategoryHierarchy, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_DirectProperty_WithPropertyCategory_WithCustomCategoryOverrideWithRootParent_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForRoot()));
    modifier->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0], { "custom", DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop") }, RelationshipMeaning::SameInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classB->GetName(), DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceRelatedProperty_DifferentIntermediateClasses_GetsParentCategory, R"*(
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
    <ECRelationshipClass typeName="AB" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="BA" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="CB" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceRelatedProperty_DifferentIntermediateClasses_GetsParentCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classB1 = GetClass("B1");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr instanceC1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA1, *instanceB1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB1, *instanceC1);

    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr instanceC2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA2, *instanceB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB2, *instanceC2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relAB->GetFullName(),
        classB->GetFullName(), "*", RelationshipMeaning::RelatedInstance, true));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relBC->GetFullName(),
        classC->GetFullName(), "*", RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classC->GetName(),
        classB1->GetName(),
        });
    EXPECT_EQ(1, descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classC->GetName(),
        classB2->GetName(),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsDefaultCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsDefaultCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForDefaultParent()) }, RelationshipMeaning::SameInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithRootCategoryOverride_GetsDefaultCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithRootCategoryOverride_GetsDefaultCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForRoot()) }, RelationshipMeaning::SameInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom"));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { "custom" });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCorrectCategoryHierarchy, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForDefaultParent()));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { "custom", DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCorrectCategoryHierarchy, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForRoot()));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { "custom", DefaultCategorySupplier().CreateDefaultCategory()->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop") }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForDefaultParent()) }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithRootCategoryOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithRootCategoryOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForRoot()) }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom"));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { "custom" });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCorrectCategoryHierarchy, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForDefaultParent()));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { "custom", classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCorrectCategoryHierarchy, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCorrectCategoryHierarchy)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForRoot()));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { "custom", classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::SameInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop") }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classC->GetName(),
        classB->GetName(),
        DefaultCategorySupplier().CreateDefaultCategory()->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::SameInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForDefaultParent()) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classB->GetName(),
        DefaultCategorySupplier().CreateDefaultCategory()->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithRootCategoryOverride_GetsRootRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithRootCategoryOverride_GetsRootRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::SameInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForRoot()) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        DefaultCategorySupplier().CreateDefaultCategory()->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom"));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::SameInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom"
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForDefaultParent()));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::SameInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom",
        classB->GetName(),
        DefaultCategorySupplier().CreateDefaultCategory()->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForRoot()));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::SameInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom",
        DefaultCategorySupplier().CreateDefaultCategory()->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop") }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classC->GetName(),
        classB->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForDefaultParent()) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithRootCategoryOverride_GetsRootRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithRootCategoryOverride_GetsRootRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForRoot()) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom"));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom"
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForDefaultParent()));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom",
        classB->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceAndSameInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForRoot()));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom",
        classB->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_NoOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop") }, RelationshipMeaning::RelatedInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classC->GetName(), classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceNestedRelatedProperty_DifferentIntermediateClasses_GetsRelatedClassCategory, R"*(
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
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D">
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
    <ECRelationshipClass typeName="C_D" modifier="None" strength="embedding">
        <Source multiplicity="(0..1)" roleLabel="cd" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="dc" polymorphic="true">
            <Class class="D"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceNestedRelatedProperty_DifferentIntermediateClasses_GetsRelatedClassCategory)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classB1 = GetClass("B1");
    ECClassCP classB2 = GetClass("B2");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relCD = GetClass("C_D")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c1, *d1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relCD, *c2, *d2);

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
        classB->GetFullName(), "*", RelationshipMeaning::RelatedInstance, true));
    modifier->GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relBC->GetFullName(),
        classC->GetFullName(), "*", RelationshipMeaning::RelatedInstance));
    modifier->GetRelatedProperties().back()->GetNestedRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relCD->GetFullName(),
        classD->GetFullName(), "*", RelationshipMeaning::RelatedInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(7, descriptor->GetCategories().size());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classD->GetName(),
        classC->GetName(),
        classB1->GetName(),
        });
    EXPECT_EQ(1, descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classD->GetName(),
        classC->GetName(),
        classB2->GetName(),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForDefaultParent()) }, RelationshipMeaning::RelatedInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classC->GetName(), classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithRootCategoryOverride_GetsRootRelatedClassCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithRootCategoryOverride_GetsRootRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForRoot()) }, RelationshipMeaning::RelatedInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classB->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverride_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom"));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::RelatedInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom"
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithParent_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForDefaultParent()));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::RelatedInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom",
        classC->GetName(),
        classB->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCustomCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceNestedRelatedProperty_NoCategory_WithCustomCategoryOverrideWithRootParent_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForRoot()));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::RelatedInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom",
        classB->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_NoOverride_GetsECSchemaPropertyCategory, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_NoOverride_GetsECSchemaPropertyCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop") }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "TestCategory",
        classB->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithDefaultParentCategoryOverride_GetsRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForDefaultParent()) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithRootCategoryOverride_GetsRootRelatedClassCategory, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithRootCategoryOverride_GetsRootRelatedClassCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForRoot()) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0], { classB->GetName() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithCustomCategoryOverride_GetsCustomCategory, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithCustomCategoryOverride_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom"));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom"
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithCustomCategoryOverrideWithParent_GetsCustomCategory, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithCustomCategoryOverrideWithParent_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForDefaultParent()));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom",
        classB->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithCustomCategoryOverrideWithRootParent_GetsCustomCategory, R"*(
    <PropertyCategory typeName="TestCategory" />
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" category="TestCategory" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="AB" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="BA" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BC" strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="BC" polymorphic="True">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="CB" polymorphic="True">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_NestedRelatedProperty_WithPropertyCategory_WithCustomCategoryOverrideWithRootParent_GetsCustomCategory)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("BC")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *instanceB, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("custom", "Custom", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForRoot()));
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward),
        }), {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("Prop", 1000, "", PropertyCategoryIdentifier::CreateForId("custom")) }, RelationshipMeaning::SameInstance));
    modifier->AddRelatedProperty(*relatedPropertiesSpec);
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        "custom",
        classB->GetName()
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceNestedRelatedProperty_WhenRelationshipPropertyExists_CategorizedUnderRelationshipCategory, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceNestedRelatedProperty_WhenRelationshipPropertyExists_CategorizedUnderRelationshipCategory)
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        relAB->GetName(),
        });
    EXPECT_EQ(1, descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0],
        {
        classB->GetName(),
        relAB->GetName(),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstanceNestedRelatedProperty_ForceCreateRelationshipCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstanceNestedRelatedProperty_ForceCreateRelationshipCategory)
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

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, false, false, false, {}, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classB->GetName(),
        relAB->GetName(),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_RelatedInstance_WhenRelationshipPropertyDoesNotExist_DoesNotCreateRelationshipCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
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
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_RelatedInstance_WhenRelationshipPropertyDoesNotExist_DoesNotCreateRelationshipCategory)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);

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

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classC->GetName(),
        classB->GetName(),
        });

    // check that relationship category is removed from descriptor and parent category
    ASSERT_EQ(3, descriptor->GetCategories().size());
    ASSERT_EQ(1, descriptor->GetCategories()[1]->GetChildCategories().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceNestedRelatedProperty_WhenRelationshipPropertyExists_CategorizedUnderRelationshipCategory, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceNestedRelatedProperty_WhenRelationshipPropertyExists_CategorizedUnderRelationshipCategory)
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

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::SameInstance, false, false, false, {new PropertySpecification("*")}));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        relAB->GetName(),
        DefaultCategorySupplier().CreateDefaultCategory()->GetName(),
        });
    EXPECT_EQ(1, descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[1]->AsNestedContentField()->GetFields()[0],
        {
        classB->GetName(),
        relAB->GetName(),
        DefaultCategorySupplier().CreateDefaultCategory()->GetName(),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategorization_SameInstanceNestedRelatedProperty_ForceCreateRelationshipCategory, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategorization_SameInstanceNestedRelatedProperty_ForceCreateRelationshipCategory)
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

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::SameInstance, false, false, false, {}, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ValidateFieldCategoriesHierarchy(*descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0],
        {
        classB->GetName(),
        relAB->GetName(),
        DefaultCategorySupplier().CreateDefaultCategory()->GetName(),
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_FieldsMerging_GetOneFieldWhenPropertyCategoriesAreEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_FieldsMerging_GetOneFieldWhenPropertyCategoriesAreEqual)
    {
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    auto spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), false, false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("my_category", "My Category"));
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("my_category")));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "UserLabel"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("my_category", descriptor->GetVisibleFields()[0]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("My Category", descriptor->GetVisibleFields()[0]->GetCategory()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_FieldsMerging_GetDifferentFieldsWhenPropertyCategoriesAreDifferent, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_FieldsMerging_GetDifferentFieldsWhenPropertyCategoriesAreDifferent)
    {
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier1 = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier1->AddPropertyCategory(*new PropertyCategorySpecification("my_category_1", "My Category 1"));
    modifier1->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("my_category_1")));
    rules->AddPresentationRule(*modifier1);

    ContentModifierP modifier2 = new ContentModifier(classB->GetSchema().GetName(), classB->GetName());
    modifier2->AddPropertyCategory(*new PropertyCategorySpecification("my_category_2", "My Category 2"));
    modifier2->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("my_category_2")));
    rules->AddPresentationRule(*modifier2);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_STREQ(FIELD_NAME(classA, "UserLabel"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("my_category_1", descriptor->GetVisibleFields()[0]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("My Category 1", descriptor->GetVisibleFields()[0]->GetCategory()->GetLabel().c_str());

    EXPECT_STREQ(FIELD_NAME(classB, "UserLabel"), descriptor->GetVisibleFields()[1]->GetUniqueName().c_str());
    EXPECT_STREQ("my_category_2", descriptor->GetVisibleFields()[1]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("My Category 2", descriptor->GetVisibleFields()[1]->GetCategory()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Polymorphism_FromBaseClassOnDerivedClass, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="DerivedElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Polymorphism_FromBaseClassOnDerivedClass)
    {
    ECClassCP baseClass = GetClass("Element");
    ECClassCP derivedClass = GetClass("DerivedElement");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", derivedClass->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(baseClass->GetSchema().GetName(), baseClass->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("my_category", "My Category"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("my_category")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME(baseClass, "UserLabel"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("my_category", descriptor->GetVisibleFields()[0]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("My Category", descriptor->GetVisibleFields()[0]->GetCategory()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="DerivedElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass)
    {
    ECClassCP baseClass = GetClass("Element");
    ECClassCP derivedClass = GetClass("DerivedElement");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(derivedClass->GetSchema().GetName(), derivedClass->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("my_category", "My Category"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("my_category")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME(baseClass, "UserLabel"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ(DefaultCategorySupplier().CreateDefaultCategory()->GetName().c_str(), descriptor->GetVisibleFields()[0]->GetCategory()->GetName().c_str());
    EXPECT_STREQ(FIELD_NAME_C(baseClass, "UserLabel", 2), descriptor->GetVisibleFields()[1]->GetUniqueName().c_str());
    EXPECT_STREQ("my_category", descriptor->GetVisibleFields()[1]->GetCategory()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("category1")));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("category2")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassA_UserLabel
    EXPECT_STREQ("category1", descriptor->GetVisibleFields()[0]->GetCategory()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Priorities_AppliesModifierOverSpecOverrideWhenPriorityIsHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Priorities_AppliesModifierOverSpecOverrideWhenPriorityIsHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("category1")));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", PropertyCategoryIdentifier::CreateForId("category2")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassA_UserLabel
    EXPECT_STREQ("category2", descriptor->GetVisibleFields()[0]->GetCategory()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("category1")) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("category2")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB
    EXPECT_EQ(1, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().size()); // ClassB_UserLabel
    EXPECT_STREQ("category1", descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetCategory()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityIsHigher, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityIsHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instanceA, *instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false);
    spec->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Category 1"));
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", nullptr, nullptr, nullptr) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Category 2"));
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", PropertyCategoryIdentifier::CreateForId("category2")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB
    EXPECT_EQ(1, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().size()); // ClassB_UserLabel
    EXPECT_STREQ("category2", descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetCategory()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty, R"*(
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty)
    {
    // set up the dataset
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsAspect")->GetRelationshipClassCP();

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("test_category", "Test Category"));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), aspectClass->GetFullName(),
        {new PropertySpecification("UserLabel", 1, "", PropertyCategoryIdentifier::CreateForId("test_category"))}, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_Aspect
    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsNestedContentField());
    ASSERT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("test_category", descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyCategoryOverride_CreatesNestedCustomCategories, R"*(
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECProperty propertyName="Prop1" typeName="string" />
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyCategoryOverride_CreatesNestedCustomCategories)
    {
    // set up the dataset
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsAspect")->GetRelationshipClassCP();

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category1", "Test Category 1"));
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category2", "Test Category 2", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForId("category1")));
    modifier->AddPropertyCategory(*new PropertyCategorySpecification("category3", "Test Category 3", "", 1000, false, nullptr, PropertyCategoryIdentifier::CreateForId("category1")));
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), aspectClass->GetFullName(),
        PropertySpecificationsList
            {
            new PropertySpecification("Prop1", 1, "", PropertyCategoryIdentifier::CreateForId("category2")),
            new PropertySpecification("Prop2", 1, "", PropertyCategoryIdentifier::CreateForId("category3")),
            },
        RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // ensure field has correct category
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_Aspect
    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsNestedContentField());
    ASSERT_EQ(2, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("category2", descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("category3", descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[1]->GetCategory()->GetName().c_str());

    // ensure categories are set up correctly
    ASSERT_EQ(5, descriptor->GetCategories().size());
    EXPECT_STREQ(DefaultCategorySupplier().CreateDefaultCategory()->GetName().c_str(), descriptor->GetCategories()[0]->GetName().c_str());
    EXPECT_STREQ(aspectClass->GetDisplayLabel().c_str(), descriptor->GetCategories()[1]->GetName().c_str());
    EXPECT_STREQ("category1", descriptor->GetCategories()[2]->GetName().c_str());
    EXPECT_STREQ("category2", descriptor->GetCategories()[3]->GetName().c_str());
    EXPECT_STREQ("category3", descriptor->GetCategories()[4]->GetName().c_str());

    ASSERT_EQ(2, descriptor->GetCategories()[2]->GetChildCategories().size());
    EXPECT_EQ(descriptor->GetCategories()[3], descriptor->GetCategories()[2]->GetChildCategories()[0]);
    EXPECT_EQ(descriptor->GetCategories()[4], descriptor->GetCategories()[2]->GetChildCategories()[1]);
    EXPECT_EQ(descriptor->GetCategories()[2].get(), descriptor->GetCategories()[3]->GetParentCategory());
    EXPECT_EQ(descriptor->GetCategories()[2].get(), descriptor->GetCategories()[4]->GetParentCategory());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DefaultPropertyCategoryOverride_UsesCustomDefaultPropertyCategory, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DefaultPropertyCategoryOverride_UsesCustomDefaultPropertyCategory)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));

    rules->AddPresentationRule(*new DefaultPropertyCategoryOverride(*new PropertyCategorySpecification("default", "Custom Category")));

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    // verify the descriptor has one category
    auto const& categories = descriptor->GetCategories();
    ASSERT_EQ(1, categories.size());
    EXPECT_STREQ("default", categories[0]->GetName().c_str());
    EXPECT_STREQ("Custom Category", categories[0]->GetLabel().c_str());

    // verify the field has correct category
    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    EXPECT_EQ(categories[0], fields[0]->GetCategory());
    }


/*---------------------------------------------------------------------------------**//**
* VSTS#177537
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CategorizesNestedContentFields,
    R"*(
    <PropertyCategory typeName="GeometryAttributes" />
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
        <ECProperty propertyName="CategorizedProp" typeName="double" category="GeometryAttributes" />
        <ECProperty propertyName="UncategorizedProp" typeName="double" />
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
TEST_F(RulesDrivenECPresentationManagerContentTests, CategorizesNestedContentFields)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");
    ECRelationshipClassCP elementHasAspectRel = GetClass("ElementHasAspect")->GetRelationshipClassCP();

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect, nullptr, true);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspectRel->GetFullName(),
        aspectClass->GetFullName(), "", RelationshipMeaning::RelatedInstance));

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields[0]->IsNestedContentField());
    EXPECT_STREQ(aspectClass->GetName().c_str(), fields[0]->GetCategory()->GetName().c_str());
    ASSERT_EQ(2, fields[0]->AsNestedContentField()->GetFields().size());

    EXPECT_STREQ(FIELD_NAME(aspectClass, "CategorizedProp"), fields[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("GeometryAttributes", fields[0]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetName().c_str());

    EXPECT_STREQ(FIELD_NAME(aspectClass, "UncategorizedProp"), fields[0]->AsNestedContentField()->GetFields()[1]->GetUniqueName().c_str());
    EXPECT_STREQ(aspectClass->GetDisplayLabel().c_str(), fields[0]->AsNestedContentField()->GetFields()[1]->GetCategory()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CategorizesFieldsWithSamePropertyCategoryAtDifferentLevelsWhenCategoriesComeFromSchema,
    R"*(
    <PropertyCategory typeName="GeometryAttributes" />
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CategorizedProp" typeName="double" category="GeometryAttributes" />
        <ECProperty propertyName="UncategorizedProp" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CategorizedAspectProp" typeName="double" category="GeometryAttributes" />
        <ECProperty propertyName="UncategorizedAspectProp" typeName="double" />
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
TEST_F(RulesDrivenECPresentationManagerContentTests, CategorizesFieldsWithSamePropertyCategoryAtDifferentLevelsWhenCategoriesComeFromSchema)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("Aspect");
    ECRelationshipClassCP elementHasAspectRel = GetClass("ElementHasAspect")->GetRelationshipClassCP();

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *element, *aspect, nullptr, true);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    rules->AddPresentationRule(*modifier);
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, elementHasAspectRel->GetFullName(),
        aspectClass->GetFullName(), "", RelationshipMeaning::RelatedInstance));

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(3, fields.size());

    // verify fields have correct hierarchies
    EXPECT_STREQ("CategorizedProp", fields[0]->GetLabel().c_str());
    EXPECT_STREQ("GeometryAttributes", fields[0]->GetCategory()->GetName().c_str());

    EXPECT_STREQ("UncategorizedProp", fields[1]->GetLabel().c_str());
    EXPECT_STREQ(DefaultCategorySupplier().CreateDefaultCategory()->GetName().c_str(), fields[1]->GetCategory()->GetName().c_str());

    ASSERT_TRUE(fields[2]->IsNestedContentField());
    EXPECT_STREQ(aspectClass->GetName().c_str(), fields[2]->GetCategory()->GetName().c_str());
    ASSERT_EQ(2, fields[2]->AsNestedContentField()->GetFields().size());

    EXPECT_STREQ("CategorizedAspectProp", fields[2]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_STREQ("GeometryAttributes", fields[2]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetName().c_str());

    EXPECT_STREQ("UncategorizedAspectProp", fields[2]->AsNestedContentField()->GetFields()[1]->GetLabel().c_str());
    EXPECT_STREQ(aspectClass->GetDisplayLabel().c_str(), fields[2]->AsNestedContentField()->GetFields()[1]->GetCategory()->GetName().c_str());

    /* Verify categories' hierarchy correctness. We expect:
                Selected Item
                    GeometryAttributes
       f[0]             CategorizedProp
       f[1]         UncategorizedProp
       f[2]     Aspect
                    GeometryAttributes
       f[2][0]          CategorizedAspectProp
       f[2][1]      UncategorizedAspectProp
    */
    EXPECT_EQ(fields[1]->GetCategory().get(), fields[0]->GetCategory()->GetParentCategory());
    EXPECT_EQ(nullptr, fields[1]->GetCategory()->GetParentCategory());
    EXPECT_EQ(nullptr, fields[2]->GetCategory()->GetParentCategory());
    EXPECT_EQ(fields[2]->GetCategory().get(), fields[2]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetParentCategory());
    EXPECT_EQ(fields[2]->GetCategory(), fields[2]->AsNestedContentField()->GetFields()[1]->GetCategory());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CategorizesFieldsWithSamePropertyCategoryAtDifferentLevelsWhenCategoriesComeFromRelatedClasses,
    R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="GeometricElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="GeometricElementProp" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="TypeElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="TypeElementProp" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="ReferencedElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="AspectProp" typeName="double" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementHasType" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementReferencesElement" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CategorizesFieldsWithSamePropertyCategoryAtDifferentLevelsWhenCategoriesComeFromRelatedClasses)
    {
    // set up data set
    ECClassCP geometricElementClass = GetClass("GeometricElement");
    ECClassCP typeElementClass = GetClass("TypeElement");
    ECClassCP referencedElementClass = GetClass("ReferencedElement");
    ECClassCP aspectClass = GetClass("Aspect");
    ECRelationshipClassCP elementHasAspectRel = GetClass("ElementHasAspect")->GetRelationshipClassCP();
    ECRelationshipClassCP elementHasTypeRel = GetClass("ElementHasType")->GetRelationshipClassCP();
    ECRelationshipClassCP elementReferencesElementRel = GetClass("ElementReferencesElement")->GetRelationshipClassCP();

    IECInstancePtr geometricElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *geometricElementClass);
    IECInstancePtr geometricElementApect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *geometricElement, *geometricElementApect);

    IECInstancePtr typeElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *typeElementClass);
    IECInstancePtr typeElementApect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *typeElement, *typeElementApect);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasTypeRel, *geometricElement, *typeElement);

    IECInstancePtr referencedElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *referencedElementClass);
    IECInstancePtr referencedElementApect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementHasAspectRel, *referencedElement, *referencedElementApect);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementReferencesElementRel, *geometricElement, *referencedElement, nullptr, true);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", geometricElementClass->GetFullName(), true, false));

    ContentModifierP modifier = new ContentModifier(geometricElementClass->GetSchema().GetName(), geometricElementClass->GetName());
    rules->AddPresentationRule(*modifier);

    // element -> aspect
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(elementHasAspectRel->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));

    // element -> type -> aspect
    auto typeRelatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(elementHasTypeRel->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true);
    typeRelatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(elementHasAspectRel->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));
    modifier->AddRelatedProperty(*typeRelatedPropertiesSpec);

    // element -> referenced element -> aspect
    auto referenceRelatedPropertiesSpec = new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(elementReferencesElementRel->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true);
    referenceRelatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(elementHasAspectRel->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::SameInstance, true));
    modifier->AddRelatedProperty(*referenceRelatedPropertiesSpec);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(4, fields.size());

    // verify fields have correct hierarchies
    size_t fieldIndex = 0;
    EXPECT_STREQ("GeometricElementProp", fields[fieldIndex]->GetLabel().c_str());
    EXPECT_STREQ(DefaultCategorySupplier().CreateDefaultCategory()->GetName().c_str(), fields[fieldIndex]->GetCategory()->GetName().c_str());

    ++fieldIndex;
    ASSERT_TRUE(fields[fieldIndex]->IsNestedContentField());
    EXPECT_STREQ(DefaultCategorySupplier().CreateDefaultCategory()->GetName().c_str(), fields[fieldIndex]->GetCategory()->GetName().c_str());
    ASSERT_EQ(1, fields[fieldIndex]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("AspectProp", fields[fieldIndex]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_STREQ(aspectClass->GetName().c_str(), fields[fieldIndex]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetName().c_str());

    ++fieldIndex;
    ASSERT_TRUE(fields[fieldIndex]->IsNestedContentField());
    EXPECT_STREQ(typeElementClass->GetName().c_str(), fields[fieldIndex]->GetCategory()->GetName().c_str());
    ASSERT_EQ(2, fields[fieldIndex]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("TypeElementProp", fields[fieldIndex]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_EQ(fields[fieldIndex]->GetCategory(), fields[fieldIndex]->AsNestedContentField()->GetFields()[0]->GetCategory());
    EXPECT_EQ(fields[fieldIndex]->GetCategory(), fields[fieldIndex]->AsNestedContentField()->GetFields()[1]->GetCategory());
    ASSERT_TRUE(fields[fieldIndex]->AsNestedContentField()->GetFields()[1]->IsNestedContentField());
    ASSERT_EQ(1, fields[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("AspectProp", fields[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_STREQ(aspectClass->GetName().c_str(), fields[fieldIndex]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetName().c_str());

    ++fieldIndex;
    ASSERT_TRUE(fields[fieldIndex]->IsNestedContentField());
    EXPECT_STREQ(referencedElementClass->GetName().c_str(), fields[fieldIndex]->GetCategory()->GetName().c_str());
    ASSERT_EQ(1, fields[fieldIndex]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ(referencedElementClass->GetName().c_str(), fields[fieldIndex]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetName().c_str());
    ASSERT_EQ(1, fields[fieldIndex]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("AspectProp", fields[fieldIndex]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    EXPECT_STREQ(aspectClass->GetName().c_str(), fields[fieldIndex]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetName().c_str());

    /* Verify categories' hierarchy correctness. We expect:
                Selected Item
       f[0]         - GeometricElementProp
       f[1]         - Aspect
       f[1][0]          - AspectProp
       f[2]     Type
       f[2][0]      - TypeElementProp
       f[2][1]      - Aspect
       f[2][1][0]       - AspectProp
       f[3]     Referenced Element
       f[3][0]      - Aspect
       f[3][0][0]       - AspectProp
    */
    EXPECT_EQ(nullptr, fields[0]->GetCategory()->GetParentCategory());

    EXPECT_EQ(nullptr, fields[1]->GetCategory()->GetParentCategory());
    EXPECT_EQ(fields[1]->GetCategory().get(), fields[1]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetParentCategory());

    EXPECT_EQ(nullptr, fields[2]->GetCategory()->GetParentCategory());
    EXPECT_EQ(fields[2]->GetCategory().get(), fields[2]->AsNestedContentField()->GetFields()[0]->GetCategory().get());
    EXPECT_EQ(fields[2]->GetCategory().get(), fields[2]->AsNestedContentField()->GetFields()[1]->GetCategory().get());
    EXPECT_EQ(fields[2]->GetCategory().get(), fields[2]->AsNestedContentField()->GetFields()[1]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetParentCategory());

    EXPECT_EQ(nullptr, fields[3]->GetCategory()->GetParentCategory());
    EXPECT_EQ(fields[3]->GetCategory().get(), fields[3]->AsNestedContentField()->GetFields()[0]->GetCategory().get());
    EXPECT_EQ(fields[3]->GetCategory().get(), fields[3]->AsNestedContentField()->GetFields()[0]->AsNestedContentField()->GetFields()[0]->GetCategory()->GetParentCategory());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CategorizesFieldsFromDifferentContentSpecifications, R"*(
    <PropertyCategory typeName="CategoryA" />
    <PropertyCategory typeName="CategoryB" />
    <ECEntityClass typeName="A">
        <ECProperty propertyName="UncategorizedProp" typeName="int" />
        <ECProperty propertyName="CategorizedProp" typeName="int" category="CategoryA" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="UncategorizedProp" typeName="int" />
        <ECProperty propertyName="CategorizedProp" typeName="int" category="CategoryB" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CategorizesFieldsFromDifferentContentSpecifications)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false));

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    // verify descriptor has both categories
    ASSERT_EQ(3, descriptor->GetCategories().size());
    EXPECT_STREQ(DefaultCategorySupplier().CreateDefaultCategory()->GetName().c_str(), descriptor->GetCategories()[0]->GetName().c_str());
    EXPECT_STREQ("CategoryA", descriptor->GetCategories()[1]->GetName().c_str());
    EXPECT_STREQ("CategoryB", descriptor->GetCategories()[2]->GetName().c_str());

    // verify fields have correct categories
    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(3, fields.size());
    EXPECT_STREQ(DefaultCategorySupplier().CreateDefaultCategory()->GetName().c_str(), fields[0]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("CategoryA", fields[1]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("CategoryB", fields[2]->GetCategory()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CategorizesCalculatedPropertiesWithDefaultCategory, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CategorizesCalculatedPropertiesWithDefaultCategory)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), classA->GetName());
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label", 1000, "123"));
    rules->AddPresentationRule(*modifier);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    // verify the field has correct category
    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    EXPECT_STREQ(DefaultCategorySupplier().CreateDefaultCategory()->GetName().c_str(), fields[0]->GetCategory()->GetName().c_str());
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RulesDrivenECPresentationManagerContentWithCustomCategorySupplierTests : RulesDrivenECPresentationManagerContentTests
    {
    TestCategorySupplier m_categorySupplier = TestCategorySupplier(ContentDescriptor::Category("CustomName", "Custom label", "Custom description", 0));

    virtual void _ConfigureManagerParams(ECPresentationManager::Params& params) override
        {
        RulesDrivenECPresentationManagerContentTests::_ConfigureManagerParams(params);
        params.SetCategorySupplier(&m_categorySupplier);
        }
    };

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesCustomPropertyCategorySupplierIfSet, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
        <ECProperty propertyName="Property3" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentWithCustomCategorySupplierTests, UsesCustomPropertyCategorySupplierIfSet)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", GetClass("A")->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // make sure all fields have custom category
    ContentDescriptor::Category const& category1 = *descriptor->GetVisibleFields()[0]->GetCategory();
    EXPECT_STREQ("Custom label", category1.GetLabel().c_str());
    EXPECT_STREQ("CustomName", category1.GetName().c_str());
    EXPECT_STREQ("Custom description", category1.GetDescription().c_str());

    ContentDescriptor::Category const& category2 = *descriptor->GetVisibleFields()[1]->GetCategory();
    EXPECT_STREQ("Custom label", category2.GetLabel().c_str());
    EXPECT_STREQ("CustomName", category2.GetName().c_str());
    EXPECT_STREQ("Custom description", category2.GetDescription().c_str());

    ContentDescriptor::Category const& category3 = *descriptor->GetVisibleFields()[2]->GetCategory();
    EXPECT_STREQ("Custom label", category3.GetLabel().c_str());
    EXPECT_STREQ("CustomName", category3.GetName().c_str());
    EXPECT_STREQ("Custom description", category3.GetDescription().c_str());
    }
