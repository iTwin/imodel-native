/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentIntegrationTests.h"

/*---------------------------------------------------------------------------------**//**
// @betest
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
void RulesDrivenECPresentationManagerContentTests::ValidateFieldCategoriesHierarchy(ContentDescriptor::Field const& field, bvector<Utf8String> const& expectedCategoriesHierarchyLeafToRoot)
    {
    auto category = field.GetCategory().get();
    for (Utf8StringCR expectedCategoryName : expectedCategoriesHierarchyLeafToRoot)
        {
        ASSERT_NE(nullptr, category);
        EXPECT_STREQ(expectedCategoryName.c_str(), category->GetName().c_str());
        category = category->GetParentCategory();
        }
    EXPECT_EQ(nullptr, category);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECPropertyCP> RulesDrivenECPresentationManagerContentTests::CreateNProperties(ECEntityClassR ecClass, int numberOfProperties)
    {
    bvector<ECPropertyCP> props;
    for (int i = 0; i < numberOfProperties; ++i)
        {
        PrimitiveECPropertyP prop = nullptr;
        ecClass.CreatePrimitiveProperty(prop, Utf8PrintfString("Prop%d", i + 1), PRIMITIVETYPE_Integer);
        props.push_back(prop);
        }
    return props;
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_ReturnsValidDescriptorBasedOnSelectedClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ReturnsValidDescriptorBasedOnSelectedClasses)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up input
    KeySetPtr input = KeySet::Create(bvector<ECClassInstanceKey>{ECClassInstanceKey(classA, ECInstanceId()), ECClassInstanceKey(classB, ECInstanceId())});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(2, descriptor->GetSelectClasses().size());

    EXPECT_EQ(classA, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());
    EXPECT_FALSE(descriptor->GetSelectClasses()[0].GetSelectClass().IsSelectPolymorphic());

    EXPECT_EQ(classB, &descriptor->GetSelectClasses()[1].GetSelectClass().GetClass());
    EXPECT_FALSE(descriptor->GetSelectClasses()[1].GetSelectClass().IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_SetsInputAndResultKeys, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_SetsInputAndResultKeys)
    {
    // set up dataset
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // set up selection
    KeySetPtr input = KeySet::Create(*element);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", (int)ContentFlags::KeysOnly | (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(element.get(), element.get()) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AllPropertiesOfOneSelectedNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AllPropertiesOfOneSelectedNode)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up input
    KeySetPtr input = KeySet::Create(*instance1);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(classA->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance1.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AcceptableClassNames_ReturnsInstanceOfDefinedClassName, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA1" typeName="string" />
        <ECProperty propertyName="PropertyA2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropertyB1" typeName="string" />
        <ECProperty propertyName="PropertyB2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AcceptableClassNames_ReturnsInstanceOfDefinedClassName)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "A", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(classA->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instanceA.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AcceptableSchemaName_WrongSchemaName_ContentIsNotValid, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AcceptableSchemaName_WrongSchemaName_ContentIsNotValid)
    {
    // insert instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetClass("A"));

    // set up input
    KeySetPtr input = KeySet::Create(*instance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "WrongSchemaName", "", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_FALSE(descriptor.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_ClassesAcceptablePolymorphically, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="BaseProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="DerivedProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ClassesAcceptablePolymorphically)
    {
    // insert some instances
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    IECInstancePtr baseInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass);
    IECInstancePtr derivedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{baseInstance, derivedInstance});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "Base", true));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{baseInstance.get(), derivedInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_SchemasAcceptablePolymorphically_BaseSchema, R"*(
    <ECEntityClass typeName="A" />
)*");
DEFINE_SCHEMA(SelectedNodeInstances_SchemasAcceptablePolymorphically, R"*(
    <ECSchemaReference name="SelectedNodeInstances_SchemasAcceptablePolymorphically_BaseSchema" version="01.00" alias="base" />
    <ECEntityClass typeName="B">
        <BaseClass>base:A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_SchemasAcceptablePolymorphically)
    {
    // set up the dataset
    ECClassCP classA = GetClass(Utf8PrintfString("%s_BaseSchema", BeTest::GetNameOfCurrentTest()).c_str(), "A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up input
    KeySetPtr input = KeySet::Create(*b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, classA->GetSchema().GetName(), "", true));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{b.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_SchemasAndClassesAcceptablePolymorphically_BaseSchema, R"*(
    <ECEntityClass typeName="A" />
)*");
DEFINE_SCHEMA(SelectedNodeInstances_SchemasAndClassesAcceptablePolymorphically, R"*(
    <ECSchemaReference name="SelectedNodeInstances_SchemasAndClassesAcceptablePolymorphically_BaseSchema" version="01.00" alias="base" />
    <ECEntityClass typeName="B">
        <BaseClass>base:A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_SchemasAndClassesAcceptablePolymorphically)
    {
    // set up the dataset
    ECClassCP classA = GetClass(Utf8PrintfString("%s_BaseSchema", BeTest::GetNameOfCurrentTest()).c_str(), "A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up input
    KeySetPtr input = KeySet::Create(*b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, classA->GetSchema().GetName(), classA->GetName(), true));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{b.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfTheSameClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfTheSameClass)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(classA->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance1.get(), instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfDifferentClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA1" typeName="string" />
        <ECProperty propertyName="PropertyA2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropertyB1" typeName="string" />
        <ECProperty propertyName="PropertyB2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_AllPropertiesOfMultipleSelectedNodesOfDifferentClasses)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance1.get(), instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_WithSortingFieldAndOrder, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_WithSortingFieldAndOrder)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("2"));});
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("1"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instanceB.get(), instanceA.get()}, *content, false);

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetSortingField(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "Property"));

    // get the content with descriptor override
    content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instanceA.get(), instanceB.get()}, *content, true);

    // change the order from ascending to descending
    ovr->SetSortDirection(SortDirection::Descending);

    // get the content with the changed sorting order
    content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted in descending order
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instanceB.get(), instanceA.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_SortByDisplayLabel, R"*(
    <ECEntityClass typeName="MyClass">
        <ECProperty propertyName="MyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_SortByDisplayLabel)
    {
    // insert some instances
    ECClassCP ecClass = GetClass("MyClass");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProperty", ECValue("a"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProperty", ECValue("b"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(0, false, ecClass->GetFullName(), "MyProperty"));

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::List, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    ASSERT_EQ(2, content->GetContentSet().GetSize());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetSortingField(descriptor->GetDisplayLabelField()->GetUniqueName().c_str());
    ovr->SetSortDirection(SortDirection::Ascending);

    // get the content with descriptor override
    content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    EXPECT_STREQ("a", content->GetContentSet().Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("b", content->GetContentSet().Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    // change the order from ascending to descending
    ovr->SetSortDirection(SortDirection::Descending);

    // get the content with the changed sorting order
    content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted in descending order
    ASSERT_EQ(2, content->GetContentSet().GetSize());
    EXPECT_STREQ("b", content->GetContentSet().Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("a", content->GetContentSet().Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_SortingByEnumProperty, R"*(
    <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="True" description="" displayLabel="TestEnum">
        <ECEnumerator value="1" displayLabel="Z" />
        <ECEnumerator value="2" displayLabel="M" />
        <ECEnumerator value="3" displayLabel="A" />
    </ECEnumeration>
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="TestEnum" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_SortingByEnumProperty)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(3));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetSortingField(FIELD_NAME(classA, "Property"));

    // get the content with descriptor override
    ContentCPtr content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are sorted
    // note: the sorting should be done by enum display values instead of enum value ids
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance2.get(), instance1.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_RemovesPropertyField, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_RemovesPropertyField)
    {
    ECClassCP classA = GetClass("A");

    // insert an instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up input
    KeySetPtr input = KeySet::Create(*instance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->RemoveRootField(*ovr->FindField(NamedContentFieldMatcher(FIELD_NAME(classA, "Property"))));

    // get the content with descriptor override
    ContentCPtr content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the Property field has been removed
    EXPECT_EQ(0, content->GetDescriptor().GetVisibleFields().size());
    EXPECT_TRUE(content->GetDescriptor().GetAllFields().end() == std::find_if(content->GetDescriptor().GetAllFields().begin(), content->GetDescriptor().GetAllFields().end(),
        [&](ContentDescriptor::Field const* field){return field->GetUniqueName().Equals(FIELD_NAME(classA, "Property"));}));
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_RemovesNavigationField, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
        <ECNavigationProperty propertyName="NavigationProperty" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A_Has_B" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="A_Has_B (reversed)" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_RemovesNavigationField)
    {
    ECClassCP classB = GetClass("B");

    // insert a sprocket instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->RemoveRootField(*ovr->FindField(NamedContentFieldMatcher(FIELD_NAME(classB, "NavigationProperty"))));

    // get the content with descriptor override
    ContentCPtr content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the NavigationProperty field has been removed
    EXPECT_EQ(1, content->GetDescriptor().GetVisibleFields().size());

    ASSERT_EQ(1, content->GetContentSet().GetSize());
    ContentSetItemCPtr record = content->GetContentSet().Get(0);
    rapidjson::Document json = record->AsJson();
    EXPECT_FALSE(json["Values"].HasMember(FIELD_NAME(classB, "NavigationProperty")));
    EXPECT_FALSE(json["DisplayValues"].HasMember(FIELD_NAME(classB, "NavigationProperty")));
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_WithFilters, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="DoubleProperty" typeName="double" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_WithFilters)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(2));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("DoubleProperty", ECValue(1.0));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2, instance3});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance1.get(), instance2.get(), instance3.get()}, *content);

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetFieldsFilterExpression(Utf8PrintfString("%s > 1 or %s < 0", FIELD_NAME(classA, "IntProperty"), FIELD_NAME(classA, "DoubleProperty")));

    // get the content with descriptor override
    content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are filtered
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_WithEscapedFilters, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_WithEscapedFilters)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("abc"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("%"));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("abc%def"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instance1, instance2, instance3});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance1.get(), instance2.get(), instance3.get()}, *content);

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetFieldsFilterExpression(Utf8String(FIELD_NAME(classA, "Property")).append(" LIKE \"%\\%%\""));

    // get the content with descriptor override
    content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are filtered
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance2.get(), instance3.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_FiltersByDisplayLabel, R"*(
    <ECEntityClass typeName="MyClass">
        <ECProperty propertyName="MyProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_FiltersByDisplayLabel)
    {
    // insert some instances
    ECClassCP ecClass = GetClass("MyClass");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProperty", ECValue("a"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance){instance.SetValue("MyProperty", ECValue("b"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(0, false, ecClass->GetFullName(), "MyProperty"));

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::List, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    ASSERT_EQ(2, content->GetContentSet().GetSize());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetFieldsFilterExpression(Utf8PrintfString("%s = \"b\"", descriptor->GetDisplayLabelField()->GetUniqueName().c_str()));

    // get the content with descriptor override
    content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are filtered
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    EXPECT_STREQ("b", content->GetContentSet().Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_FiltersByNavigationProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECNavigationProperty propertyName="A" relationshipName="A_To_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="false">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="false">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DescriptorOverride_FiltersByNavigationProperty)
    {
    // insert some instances
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP rel = GetClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Label", ECValue("1"));});
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a1, *b1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Label", ECValue("2"));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a2, *b2);

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    rules->AddPresentationRule(*new InstanceLabelOverride(0, false, classA->GetFullName(), "Label"));

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    ASSERT_EQ(3, content->GetContentSet().GetSize());

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetFieldsFilterExpression(Utf8PrintfString("%s = \"1\"", descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()));

    // get the content with descriptor override
    content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are filtered
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{b1.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_ReturnsValidDescriptorWhichDoesNotDependOnSelectedClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_ReturnsValidDescriptorWhichDoesNotDependOnSelectedClasses)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up input
    KeySetPtr input = KeySet::Create({ECClassInstanceKey(classB, ECInstanceId())});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(classA, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());
    EXPECT_TRUE(descriptor->GetSelectClasses()[0].GetSelectClass().IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_SetsInputAndResultKeys, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_SetsInputAndResultKeys)
    {
    // set up dataset
    ECClassCP elementClass = GetClass("Element");
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), "", (int)ContentFlags::KeysOnly | (int)ContentFlags::IncludeInputKeys, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(element.get(), element.get()) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_ClassNames_ReturnsInstanceOfDefinedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_ClassNames_ReturnsInstanceOfDefinedClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(classA->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instanceA.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_InstanceFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_InstanceFilter)
    {
    ECClassCP classA = GetClass("A");

    // insert some instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(2));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "this.Property=2", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(classA->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_HandleInstancesPolymorphically, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="BaseProperty1" typeName="string" />
        <ECProperty propertyName="BaseProperty2" typeName="string" />
        <ECProperty propertyName="BaseProperty3" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="DerivedProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_HandleInstancesPolymorphically)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    // insert some instances
    IECInstancePtr baseInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass);
    IECInstancePtr derivedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{baseInstance.get(), derivedInstance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_NoDerivedClasses, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="PropertyOnBase" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_NoDerivedClasses)
    {
    ECClassCP baseClass = GetClass("Base");

    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass, [] (IECInstanceR instance) { instance.SetValue("PropertyOnBase", ECValue("value")); });

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), false, true));

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_NoInstances, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="PropertyOnBase" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_NoInstances)
    {
    ECClassCP baseClass = GetClass("Base");

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), false, true));

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    EXPECT_FALSE(descriptor.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_OneDerivedClassWithInstance, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="PropertyOnBase" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="PropertyOnDerived" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_OneDerivedClassWithInstance)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass, [] (IECInstanceR instance) { instance.SetValue("PropertyOnDerived", ECValue("value")); });

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true, true));

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_WhileHandleInstancesPolymorphicallyIsFalse, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="PropertyOnBase" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="PropertyOnDerived" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_WhileHandleInstancesPolymorphicallyIsFalse)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass, [] (IECInstanceR instance) { instance.SetValue("PropertyOnDerived", ECValue("value")); });

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), false, true));

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    EXPECT_FALSE(descriptor.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_ParallelDerivedClassesWithInstances, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="PropertyOnBase" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived1">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="PropertyOnDerived1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived2">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="PropertyOnDerived2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_ParallelDerivedClassesWithInstances)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derived1Class = GetClass("Derived1");
    ECClassCP derived2Class = GetClass("Derived2");

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derived1Class, [] (IECInstanceR instance) { instance.SetValue("PropertyOnDerived1", ECValue("value1")); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derived2Class, [] (IECInstanceR instance) { instance.SetValue("PropertyOnDerived2", ECValue("value2")); });

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true, true));

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance1.get(), instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_NestedDerivedClassesWithInstances, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="PropertyOnBase" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived1">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="PropertyOnDerived1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived2">
        <BaseClass>Derived1</BaseClass>
        <ECProperty propertyName="PropertyOnDerived2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_NestedDerivedClassesWithInstances)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derived1Class = GetClass("Derived1");
    ECClassCP derived2Class = GetClass("Derived2");

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derived1Class, [] (IECInstanceR instance) { instance.SetValue("PropertyOnDerived1", ECValue("value1")); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derived2Class, [] (IECInstanceR instance)
        {
        instance.SetValue("PropertyOnDerived1", ECValue("value2"));
        instance.SetValue("PropertyOnDerived2", ECValue("value3"));
        });

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true, true));

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance1.get(), instance2.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_FiltersOutClassesWithoutInstances, R"*(
    <ECEntityClass typeName="Base1">
        <ECProperty propertyName="PropertyOnBase1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived1">
        <BaseClass>Base1</BaseClass>
        <ECProperty propertyName="PropertyOnDerived1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived2">
        <BaseClass>Derived1</BaseClass>
        <ECProperty propertyName="PropertyOnDerived2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Base2">
        <ECProperty propertyName="PropertyOnBase2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_FiltersOutClassesWithoutInstances)
    {
    ECClassCP derived1Class = GetClass("Derived1");

    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derived1Class, [] (IECInstanceR instance) { instance.SetValue("PropertyOnDerived1", ECValue("value1")); });

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:Base1,Base2", GetSchema()->GetName().c_str()), true, true));

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_ConsidersInstanceFilterWhenFilteringClasses, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="PropertyOnBase" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived1">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="PropertyOnDerived1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived2">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="PropertyOnDerived2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_HandlePropertiesPolymorphically_ConsidersInstanceFilterWhenFilteringClasses)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derived1Class = GetClass("Derived1");
    ECClassCP derived2Class = GetClass("Derived2");

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derived1Class, [] (IECInstanceR instance) { instance.SetValue("PropertyOnBase", ECValue("value1")); });
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derived2Class, [] (IECInstanceR instance) { instance.SetValue("PropertyOnBase", ECValue("value2")); });

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "this.PropertyOnBase = \"value1\"", baseClass->GetFullName(), true, true));

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    rules->AddPresentationRule(*rule);
    m_locater->AddRuleSet(*rules);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{instance1.get()}, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="A_Has_B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="A_Has_B (reversed)" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_Has_Bs"  strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="A has B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="A has B (reversed)" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipAHasBs = GetClass("A_Has_Bs")->GetRelationshipClassCP();

    // insert some instances with relationships
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("CustomID")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasBs, *instanceA, *instanceB2);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "", classB->GetFullName()));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult(*instanceA, *instanceB1),
        InstanceInputAndResult(*instanceA, *instanceB2),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance_BackwardsDirection, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="A_Has_B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="A_Has_B (reversed)" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelatedClassNames_ReturnsRelatedInstance_BackwardsDirection)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, "", classA->GetFullName()));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(classA->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*instanceB, *instanceA) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsRelatedInstancesPolymorphically, R"*(
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
        <ECProperty propertyName="PropC" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="PropD" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsRelatedInstancesPolymorphically)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP rel = GetClass("A_Has_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) { instance.SetValue("PropC", ECValue("C")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *c);

    // set up input
    KeySetPtr input = KeySet::Create(*a);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({new RepeatableRelationshipStepSpecification(
        rel->GetFullName(), RequiredRelationDirection_Forward
    )})}));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ("PropC", descriptor->GetVisibleFields().front()->AsPropertiesField()->GetProperties().front().GetProperty().GetName().c_str());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*a, *c) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_RelationshipClassNames_ReturnsInvalidContentWhenRelationshipDoesNotExist, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="A_Has_B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="A_Has_B (reversed)" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelationshipClassNames_ReturnsInvalidContentWhenRelationshipDoesNotExist)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "DoesNotExist:DoesNotExist", ""));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_FALSE(descriptor.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_RelationshipClassNames, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropertyB" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="A_Has_B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="A_Has_B (reversed)" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_Has_B_2"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="A_Has_B_2" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="A_Has_B_2 (reversed)" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelationshipClassNames)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipAHasB2 = GetClass("A_Has_B_2")->GetRelationshipClassCP();

    // insert some instances with relationships
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB2, *instanceA, *instanceB2);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, relationshipAHasB->GetFullName(), ""));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*instanceA, *instanceB1) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_RelationshipClassNames_BackwardsDirection, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="A_Has_B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="A_Has_B (reversed)" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RelationshipClassNames_BackwardsDirection)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB= GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, relationshipAHasB->GetFullName(), ""));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(classA->GetPropertyCount(), descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*instanceB, *instanceA) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsSingleStepRelatedInstances, R"*(
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
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsSingleStepRelatedInstances)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP modelContainsElementsRelationship = GetClass("ModelContainsElements")->GetRelationshipClassCP();

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *modelContainsElementsRelationship, *model, *element);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(
        modelContainsElementsRelationship->GetFullName(), RequiredRelationDirection_Forward
    ))}));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    KeySetPtr input = KeySet::Create(*model);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*model, *element) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiStepRelatedInstances, R"*(
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
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiStepRelatedInstances)
    {
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP elementOwnsChildElementsRelationship = GetClass("ElementOwnsChildElements")->GetRelationshipClassCP();

    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *e1, *e2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *e2, *e3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElementsRelationship, *e3, *e4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(
        elementOwnsChildElementsRelationship->GetFullName(), RequiredRelationDirection_Backward, elementClass->GetFullName(), 3
    ))}));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    KeySetPtr input = KeySet::Create(*e4);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*e4, *e1) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipSingleStepRelatedInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipSingleStepRelatedInstances)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Backward),
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Backward),
        })}));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*c);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*c, *a) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipMultiStepRelatedInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipMultiStepRelatedInstances)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBB = GetClass("B_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b1, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b2, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Backward),
        new RepeatableRelationshipStepSpecification(relationshipBB->GetFullName(), RequiredRelationDirection_Backward, "", 2),
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Backward),
        })}));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*c);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*c, *a) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipInFront, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipInFront)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAA = GetClass("A_To_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a2, *a3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a3, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        })}));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*a1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult(*a1, *c1),
        InstanceInputAndResult(*a1, *c2),
        InstanceInputAndResult(*a1, *c3),
        InstanceInputAndResult(*a1, *c4),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipInTheMiddle, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipInTheMiddle)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBB = GetClass("B_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b1, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b2, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *b2, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBB->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        })}));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*a);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult(*a, *c1),
        InstanceInputAndResult(*a, *c2),
        InstanceInputAndResult(*a, *c3),
        InstanceInputAndResult(*a, *c4),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipAtTheEnd, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipAtTheEnd)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipCC = GetClass("C_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipCC->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        })}));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*a);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult(*a, *c1),
        InstanceInputAndResult(*a, *c2),
        InstanceInputAndResult(*a, *c3),
        InstanceInputAndResult(*a, *c4),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipsInMultiplePlaces, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsMultiRelationshipRecursivelyRelatedInstancesWithRecursiveRelationshipsInMultiplePlaces)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAA = GetClass("A_To_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipCC = GetClass("C_To_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a2, *a3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a3, *b3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a3, *a4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a4, *b4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBC, *b4, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c1, *c3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipCC, *c3, *c4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipCC->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        })}));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*a1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult(*a1, *c1),
        InstanceInputAndResult(*a1, *c2),
        InstanceInputAndResult(*a1, *c3),
        InstanceInputAndResult(*a1, *c4),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsEmptyResultsWithRecursiveRelationship, R"*(
    <ECEntityClass typeName="A" />
    <ECRelationshipClass typeName="A_To_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsEmptyResultsWithRecursiveRelationship)
    {
    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP relationshipAA = GetClass("A_To_A")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        })}));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*a);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsEmptyResultsWithRecursiveRelationshipAtTheEnd, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsEmptyResultsWithRecursiveRelationshipAtTheEnd)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBC = GetClass("B_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipCC = GetClass("C_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *b);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", { new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBC->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipCC->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        }) }));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*a);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsRecursivelyRelatedInstancesWithRecursiveRelationshipWithAbstractTargetAtTheEnd, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsRecursivelyRelatedInstancesWithRecursiveRelationshipWithAbstractTargetAtTheEnd)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAB = GetClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBB = GetClass("B_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAB, *a, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *c1, *c2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBB, *c1, *c3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAB->GetFullName(), RequiredRelationDirection_Forward),
        new RepeatableRelationshipStepSpecification(relationshipBB->GetFullName(), RequiredRelationDirection_Forward, classB->GetFullName(), 0),
        })}));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*a);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(),
        rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Graphics, (int)ContentFlags::KeysOnly | (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult(*a, *c1),
        InstanceInputAndResult(*a, *c2),
        InstanceInputAndResult(*a, *c3),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_SetsInputAndResultKeysWhenSameResultIsAssociatedWithManyInputs, R"*(
    <ECEntityClass typeName="A" />
    <ECRelationshipClass typeName="A_To_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_SetsInputAndResultKeysWhenSameResultIsAssociatedWithManyInputs)
    {
    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP relationshipAA = GetClass("A_To_A")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a2, *a3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a3, *a4);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", { new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        }) }));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{ a1, a2 });
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult({ a1.get() }, *a2),
        InstanceInputAndResult({ a1.get(), a2.get() }, *a3),
        InstanceInputAndResult({ a1.get(), a2.get() }, *a4),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsEmptyResultsWithInvalidRecursiveRelationship, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="B_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsEmptyResultsWithInvalidRecursiveRelationship)
    {
    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP relationshipBB = GetClass("B_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", { new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipBB->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        }) }));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*a);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsResultsWithRecursiveRelationshipWhenElementsPointToEachOther, R"*(
    <ECEntityClass typeName="A" />
    <ECRelationshipClass typeName="A_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_ReturnsResultsWithRecursiveRelationshipWhenElementsPointToEachOther)
    {
    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP relationshipAA = GetClass("A_A")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAA, *a2, *a1);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", { new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relationshipAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
        }) }));
    rules->AddPresentationRule(*rule);

    // request
    KeySetPtr input = KeySet::Create(*a1);
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", (int)ContentFlags::KeysOnly | (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult(*a1, *a1),
        InstanceInputAndResult(*a1, *a2),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_InstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_InstanceFilter)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB1"));});
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceB2"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB2);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "this.Property=\"InstanceB2\"", RequiredRelationDirection_Both, "", classB->GetFullName()));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*instanceA, *instanceB2) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_Recursive, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_Recursive)
    {
    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP relationshipClass = GetClass("A_Has_A")->GetRelationshipClassCP();

    // insert some  instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(2));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(3));});
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(4));});
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance1, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance2, *instance4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *instance4, *instance5);

    /*
    Relationship hierarchy:
            1
            |
            2
          /   \
         3     4
               |
               5
    */

    // set up input
    KeySetPtr input = KeySet::Create(*instance2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Both, "", classA->GetFullName()));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        // note: tracking input keys doesn't work when using deprecated recursion specification
        InstanceInputAndResult(/* *instance2 */ *instance1, *instance1),
        InstanceInputAndResult(/* *instance2 */ *instance3, *instance3),
        InstanceInputAndResult(/* *instance2 */ *instance4, *instance4),
        InstanceInputAndResult(/* *instance2 */ *instance5, *instance5),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_RecursiveWithMultipleRelationships, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropertyB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropertyC" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
        <ECRelationshipClass typeName="B_Has_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RecursiveWithMultipleRelationships)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasA = GetClass("A_Has_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_Has_C")->GetRelationshipClassCP();

    // insert some  instances
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropertyA", ECValue(1));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropertyA", ECValue(2));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropertyA", ECValue(3));});
    IECInstancePtr instance4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropertyA", ECValue(4));});
    IECInstancePtr instance5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("PropertyA", ECValue(5));});
    IECInstancePtr instance6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropertyB", ECValue(6));});
    IECInstancePtr instance7 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("PropertyB", ECValue(7));});
    IECInstancePtr instance8 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance){instance.SetValue("PropertyC", ECValue(8));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasA, *instance1, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasA, *instance2, *instance3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasA, *instance2, *instance4);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasA, *instance4, *instance5);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instance5, *instance6);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instance5, *instance7);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *instance7, *instance8);

    /*
    Relationship hierarchy:
            1
            |       <-- A_Has_A
            2
          /   \     <-- A_Has_A
         3     4
               |    <-- A_Has_A
               5
              / \   <-- A_Has_B
             6   7
                 |  <-- B_Has_C
                 8
    */

    // set up input
    KeySetPtr input = KeySet::Create(*instance1);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Forward,
        Utf8PrintfString("%s:A_Has_A,A_Has_B,B_Has_C", GetSchema()->GetName().c_str()), Utf8PrintfString("%s:A,B,C", GetSchema()->GetName().c_str())));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        // note: tracking input keys doesn't work when using deprecated recursion specification
        InstanceInputAndResult(/* *instance1 */ *instance2, *instance2),
        InstanceInputAndResult(/* *instance1 */ *instance3, *instance3),
        InstanceInputAndResult(/* *instance1 */ *instance4, *instance4),
        InstanceInputAndResult(/* *instance1 */ *instance5, *instance5),
        InstanceInputAndResult(/* *instance1 */ *instance6, *instance6),
        InstanceInputAndResult(/* *instance1 */ *instance7, *instance7),
        InstanceInputAndResult(/* *instance1 */ *instance8, *instance8),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_ContentRelatedInstances_RecursiveWithMultipleSelectClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DEPRECATED_ContentRelatedInstances_RecursiveWithMultipleSelectClasses)
    {
    ECEntityClassCP elementClass = GetClass("Element")->GetEntityClassCP();
    ECEntityClassCP elementAClass = GetClass("ElementA")->GetEntityClassCP();
    ECEntityClassCP elementBClass = GetClass("ElementB")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClass = GetRelationshipClass("ElementOwnsChildElements")->GetRelationshipClassCP();

    // insert some  instances
    IECInstancePtr parentA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass, [](IECInstanceR instance){instance.SetValue("ElementProperty", ECValue(1));});
    IECInstancePtr childA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementProperty", ECValue(2));});
    IECInstancePtr parentB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass, [](IECInstanceR instance){instance.SetValue("ElementProperty", ECValue(3));});
    IECInstancePtr childB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR instance){instance.SetValue("ElementProperty", ECValue(4));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *parentA, *childA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *parentB, *childB);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{parentA, parentB});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection_Forward,
        relationshipClass->GetFullName(), elementClass->GetFullName()));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    RulesEngineTestHelpers::ValidateContentSet(
        {
        // note: tracking input keys doesn't work when using deprecated recursion specification
        InstanceInputAndResult(/* *parentA */ *childA, *childA),
        InstanceInputAndResult(/* *parentB */ *childB, *childB),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
 * @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_CalculatedPropertiesValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>

)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_CalculatedPropertiesValue)
    {
    ECClassCP classA = GetClass("A");

    // insert some instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property", ECValue("Test"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    ContentSpecificationP specification = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    specification->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label1", 1000, "\"Value\""));
    specification->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1100, "1+2"));
    specification->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label3", 1200, "this.Property"));
    rule->AddSpecification(*specification);
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(4, descriptor->GetVisibleFields().size());

    ASSERT_TRUE(descriptor->GetVisibleFields()[1]->IsCalculatedPropertyField());
    ASSERT_TRUE(descriptor->GetVisibleFields()[2]->IsCalculatedPropertyField());
    ASSERT_TRUE(descriptor->GetVisibleFields()[3]->IsCalculatedPropertyField());

    EXPECT_STREQ("label1", descriptor->GetVisibleFields()[1]->AsCalculatedPropertyField()->GetLabel().c_str());
    EXPECT_STREQ("label2", descriptor->GetVisibleFields()[2]->AsCalculatedPropertyField()->GetLabel().c_str());
    EXPECT_STREQ("label3", descriptor->GetVisibleFields()[3]->AsCalculatedPropertyField()->GetLabel().c_str());

    EXPECT_EQ(1000, descriptor->GetVisibleFields()[1]->GetPriority());
    EXPECT_EQ(1100, descriptor->GetVisibleFields()[2]->GetPriority());
    EXPECT_EQ(1200, descriptor->GetVisibleFields()[3]->GetPriority());

    EXPECT_STREQ("\"Value\"", descriptor->GetVisibleFields()[1]->AsCalculatedPropertyField()->GetValueExpression().c_str());
    EXPECT_STREQ("1+2", descriptor->GetVisibleFields()[2]->AsCalculatedPropertyField()->GetValueExpression().c_str());
    EXPECT_STREQ("this.Property", descriptor->GetVisibleFields()[3]->AsCalculatedPropertyField()->GetValueExpression().c_str());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];

    EXPECT_STREQ("Value", jsonValues["CalculatedProperty_0"].GetString());
    EXPECT_STREQ("3", jsonValues["CalculatedProperty_1"].GetString());
    EXPECT_STREQ("Test", jsonValues["CalculatedProperty_2"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CalculatedPropertiesSpecificationAppliedForBaseClassAndDerived_CreatesOneField, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropertyB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="PropertyC" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CalculatedPropertiesSpecificationAppliedForBaseClassAndDerived_CreatesOneField)
    {
    // set up the dataset
    ECEntityClassCP classA = GetClass("A")->GetEntityClassCP();
    ECEntityClassCP classB = GetClass("B")->GetEntityClassCP();
    ECEntityClassCP classC = GetClass("C")->GetEntityClassCP();
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("PropertyB", ECValue(1000));});
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) { instance.SetValue("PropertyB", ECValue(2000));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceC});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName().c_str(), "B");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 900, "this.PropertyB"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("1000", jsonValues["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classA, "PropertyA")].IsNull());
    EXPECT_EQ(1000, jsonValues[FIELD_NAME(classB, "PropertyB")].GetInt());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classC, "PropertyC")].IsNull());

    rapidjson::Document jsonDoc1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues1 = jsonDoc1["Values"];
    EXPECT_STREQ("2000", jsonValues1["CalculatedProperty_0"].GetString());
    EXPECT_TRUE(jsonValues1[FIELD_NAME(classA, "PropertyA")].IsNull());
    EXPECT_EQ(2000, jsonValues1[FIELD_NAME(classB, "PropertyB")].GetInt());
    EXPECT_FALSE(jsonValues1[FIELD_NAME(classC, "PropertyC")].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentSerialization, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="StringProperty" typeName="string" />
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="BoolProperty" typeName="bool" />
        <ECProperty propertyName="DoubleProperty" typeName="double" />
        <ECProperty propertyName="LongProperty" typeName="long" />
        <ECProperty propertyName="DateProperty" typeName="dateTime" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, ContentSerialization)
    {
    ECClassCP classA = GetClass("A");
    // insert instance
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StringProperty", ECValue("Test"));
        instance.SetValue("IntProperty", ECValue(9));
        instance.SetValue("BoolProperty", ECValue(true));
        instance.SetValue("DoubleProperty", ECValue(7.0));
        instance.SetValue("LongProperty", ECValue((int64_t)123));
        instance.SetValue("DateProperty", ECValue(DateTime(2017, 5, 30)));
        });

    // set up input
    KeySetPtr input = KeySet::Create(*instance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));
    rules->AddPresentationRule(*rule);

    // request for content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    rapidjson::Document contentJson = content->AsJson();
    EXPECT_TRUE(contentJson.IsObject());

    // validate descriptor
    ASSERT_TRUE(contentJson.HasMember("Descriptor"));
    RapidJsonValueCR descriptorJson = contentJson["Descriptor"];
    ASSERT_TRUE(descriptorJson.HasMember("Fields"));
    RapidJsonValueCR fields = descriptorJson["Fields"];
    ASSERT_TRUE(fields.IsArray());
    ASSERT_EQ(6, fields.Size());

    // validate content set
    ASSERT_TRUE(contentJson.HasMember("ContentSet"));
    ASSERT_TRUE(contentJson["ContentSet"].IsArray());
    ASSERT_EQ(1, contentJson["ContentSet"].Size());
    RapidJsonValueCR contentSetItem = contentJson["ContentSet"][0];
    EXPECT_TRUE(contentSetItem.IsObject());

    ASSERT_TRUE(contentSetItem.HasMember("PrimaryKeys"));
    RapidJsonValueCR primaryKeys = contentSetItem["PrimaryKeys"];
    ASSERT_TRUE(primaryKeys.IsArray() && 1 == primaryKeys.Size());
    EXPECT_STREQ(instance->GetClass().GetId().ToString().c_str(), primaryKeys[0]["ECClassId"].GetString());
    EXPECT_STREQ(instance->GetInstanceId().c_str(), primaryKeys[0]["ECInstanceId"].GetString());

    ASSERT_TRUE(contentSetItem.HasMember("Values"));
    RapidJsonValueCR value = contentSetItem["Values"];

    for(rapidjson::Value::ConstValueIterator field = fields.Begin(); field != fields.End(); field++)
        {
        Utf8String name = (*field)["Name"].GetString();
        ASSERT_TRUE(value.HasMember(name.c_str()));

        if (name.Equals(FIELD_NAME(classA, "StringProperty")))
            {
            EXPECT_STREQ("Test", value[name.c_str()].GetString());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("string", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(classA, "IntProperty")))
            {
            EXPECT_EQ(9, value[name.c_str()].GetInt());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("int", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(classA, "BoolProperty")))
            {
            EXPECT_TRUE(value[name.c_str()].GetBool());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("boolean", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(classA, "DoubleProperty")))
            {
            EXPECT_DOUBLE_EQ(7.0, value[name.c_str()].GetDouble());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("double", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(classA, "LongProperty")))
            {
            EXPECT_EQ(123, value[name.c_str()].GetInt64());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("long", (*field)["Type"]["TypeName"].GetString());
            }
        else if (name.Equals(FIELD_NAME(classA, "DateProperty")))
            {
            EXPECT_STREQ("2017-05-30T00:00:00.000", value[name.c_str()].GetString());
            EXPECT_STREQ("Primitive", (*field)["Type"]["ValueFormat"].GetString());
            EXPECT_STREQ("dateTime", (*field)["Type"]["TypeName"].GetString());
            }
        else
            FAIL();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HandlesOnlyOneContentSpecificationWhenAllSpecificationsHaveOnlyIfNotHandled, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="BHasC" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, HandlesOnlyOneContentSpecificationWhenAllSpecificationsHaveOnlyIfNotHandled)
    {
    // set up the dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP bcRelationship = GetClass("BHasC")->GetRelationshipClassCP();

    //expected returned instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *bcRelationship, *b, *c);

    // set up input
    KeySetPtr inputA = KeySet::Create(*a);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    // seperate specs
    SelectedNodeInstancesSpecificationP specA = new SelectedNodeInstancesSpecification(2, true, BeTest::GetNameOfCurrentTest(), "A", false);
    rule->AddSpecification(*specA);

    ContentInstancesOfSpecificClassesSpecificationP specB = new ContentInstancesOfSpecificClassesSpecification(1, true, "",
        bvector<MultiSchemaClass*> { new MultiSchemaClass(BeTest::GetNameOfCurrentTest(), false, bvector<Utf8String> { "B" })}, bvector<MultiSchemaClass*>(), false);
    rule->AddSpecification(*specB);

    ContentRelatedInstancesSpecificationP specC = new ContentRelatedInstancesSpecification(1, true, "",
        bvector<RepeatableRelationshipPathSpecification*> { new RepeatableRelationshipPathSpecification(
            *new RepeatableRelationshipStepSpecification(
                bcRelationship->GetName(),
                RequiredRelationDirection_Forward))});
    rule->AddSpecification(*specC);

    SelectedNodeInstancesSpecificationP specD = new SelectedNodeInstancesSpecification(1, true, "", "", false);
    rule->AddSpecification(*specD);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *inputA)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    ASSERT_EQ(classA->GetId(), descriptor->GetSelectClasses()[0].GetSelectClass().GetClass().GetId());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);

    ASSERT_TRUE(content.IsValid());
    ASSERT_EQ(1, content->GetContentSet().GetSize());
    RulesEngineTestHelpers::ValidateContentSetItem(*a, *content->GetContentSet()[0], *descriptor);
    }

#ifdef wip_field_instance_keys
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPropertyValueInstanceKeys)
    {
    // set up the dataset
    ECRelationshipClassCP widgetHasGadgetsRelationship = GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget);

    ECClassInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);
    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID,IntProperty", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(5, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID, Widget.IntProperty

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldPropertyIdentifier fp0(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp1(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp2(*descriptor->GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp3(*descriptor->GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> widgetMyIdFieldInstanceKeys = record->GetPropertyValueKeys(fp3);
    ASSERT_EQ(1, widgetMyIdFieldInstanceKeys.size());
    EXPECT_EQ(widgetKey, widgetMyIdFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp4(*descriptor->GetVisibleFields()[4]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> widgetIntpropertyFieldInstanceKeys = record->GetPropertyValueKeys(fp4);
    ASSERT_EQ(1, widgetIntpropertyFieldInstanceKeys.size());
    EXPECT_EQ(widgetKey, widgetIntpropertyFieldInstanceKeys[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPropertyValueInstanceKeysWhenColumnsAreMerged)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);
    ECClassInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);
    ECClassInstanceKey sprocketKey = RulesEngineTestHelpers::GetInstanceKey(*sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget,Sprocket", false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID + Sprocket.MyID, Gadget.Description + Sprocket.MyID, Gadget.Widget, Sprocket.Gadget

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    ContentSetItem::FieldPropertyIdentifier fp00(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 0);
    ContentSetItem::FieldPropertyIdentifier fp01(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 1);
    ContentSetItem::FieldPropertyIdentifier fp10(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);
    ContentSetItem::FieldPropertyIdentifier fp11(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 1);
    ContentSetItem::FieldPropertyIdentifier fp20(*descriptor->GetVisibleFields()[2]->AsPropertiesField(), 0);
    ContentSetItem::FieldPropertyIdentifier fp30(*descriptor->GetVisibleFields()[3]->AsPropertiesField(), 0);

    ContentSetItemCPtr record0 = contentSet.Get(0); // gadget

    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record0->GetPropertyValueKeys(fp00);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);

    bvector<ECClassInstanceKey> sprocketMyidFieldInstanceKeys = record0->GetPropertyValueKeys(fp01);
    EXPECT_TRUE(sprocketMyidFieldInstanceKeys.empty());

    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record0->GetPropertyValueKeys(fp10);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    bvector<ECClassInstanceKey> sprocketDescriptionFieldInstanceKeys = record0->GetPropertyValueKeys(fp11);
    EXPECT_TRUE(sprocketDescriptionFieldInstanceKeys.empty());

    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record0->GetPropertyValueKeys(fp20);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    bvector<ECClassInstanceKey> sprocketGadgetFieldInstanceKeys = record0->GetPropertyValueKeys(fp30);
    EXPECT_TRUE(sprocketGadgetFieldInstanceKeys.empty());

    ContentSetItemCPtr record1 = contentSet.Get(1); // sprocket

    gadgetMyidFieldInstanceKeys = record1->GetPropertyValueKeys(fp00);
    EXPECT_TRUE(gadgetMyidFieldInstanceKeys.empty());

    sprocketMyidFieldInstanceKeys = record1->GetPropertyValueKeys(fp01);
    ASSERT_EQ(1, sprocketMyidFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketMyidFieldInstanceKeys[0]);

    gadgetDescriptionFieldInstanceKeys = record1->GetPropertyValueKeys(fp10);
    EXPECT_TRUE(gadgetDescriptionFieldInstanceKeys.empty());

    sprocketDescriptionFieldInstanceKeys = record1->GetPropertyValueKeys(fp11);
    ASSERT_EQ(1, sprocketDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketDescriptionFieldInstanceKeys[0]);

    gadgetWidgetFieldInstanceKeys = record1->GetPropertyValueKeys(fp20);
    EXPECT_TRUE(gadgetWidgetFieldInstanceKeys.empty());

    sprocketGadgetFieldInstanceKeys = record1->GetPropertyValueKeys(fp30);
    ASSERT_EQ(1, sprocketGadgetFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketGadgetFieldInstanceKeys[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPropertyValueInstanceKeysWhenRowsAreMerged)
    {
    // set up the dataset
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    ECClassInstanceKey gadgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*gadget1);
    ECClassInstanceKey gadgetKey2 = RulesEngineTestHelpers::GetInstanceKey(*gadget2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldPropertyIdentifier fp0(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(2, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetMyidFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetMyidFieldInstanceKeys[1]);

    ContentSetItem::FieldPropertyIdentifier fp1(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(2, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetDescriptionFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetDescriptionFieldInstanceKeys[1]);

    ContentSetItem::FieldPropertyIdentifier fp2(*content->GetDescriptor().GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(2, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetWidgetFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetWidgetFieldInstanceKeys[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsPropertyValueInstanceKeysWhenRowsAndColumnsAreMerged)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);
    ECClassInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);
    ECClassInstanceKey sprocketKey = RulesEngineTestHelpers::GetInstanceKey(*sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget,Sprocket", false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID + Sprocket.MyID, Gadget.Description + Sprocket.MyID, Gadget.Widget, Sprocket.Gadget

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldPropertyIdentifier fp00(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp00);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp01(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 1);
    bvector<ECClassInstanceKey> sprocketMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp01);
    ASSERT_EQ(1, sprocketMyidFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketMyidFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp10(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp10);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp11(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 1);
    bvector<ECClassInstanceKey> sprocketDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp11);
    ASSERT_EQ(1, sprocketDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp20(*content->GetDescriptor().GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp20);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp30(*content->GetDescriptor().GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> sprocketGadgetFieldInstanceKeys = record->GetPropertyValueKeys(fp30);
    ASSERT_EQ(1, sprocketGadgetFieldInstanceKeys.size());
    EXPECT_EQ(sprocketKey, sprocketGadgetFieldInstanceKeys[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectsRelatedPropertyValuesWhenSelectingFromMultipleClasses)
    {
    // set up the dataset
    ECRelationshipClassCP rel = GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Widget"));
        });
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        });
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Sprocket"));
        instance.SetValue("Gadget", ECValue(RulesEngineTestHelpers::GetInstanceKey(*gadget).GetId(), rel));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Sprocket", false, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); // 7 Widget properties (2 of them merged with Sprocket MyID and Description), Sprocket.Gadget, Gadget.MyID

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    ContentSetItem::FieldPropertyIdentifier fp(*descriptor->GetVisibleFields()[8]->AsPropertiesField(), 0);

    ContentSetItemCPtr widgetRecord = contentSet.Get(0);
    bvector<ECClassInstanceKey> widgetKeys = widgetRecord->GetPropertyValueKeys(fp);
    EXPECT_TRUE(widgetKeys.empty());
    EXPECT_TRUE(widgetRecord->AsJson()["Values"][fp.GetFieldName().c_str()].IsNull());

    ContentSetItemCPtr sprocketRecord = contentSet.Get(1);
    bvector<ECClassInstanceKey> sprocketKeys = sprocketRecord->GetPropertyValueKeys(fp);
    ASSERT_EQ(1, sprocketKeys.size());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadget), sprocketKeys[0]);
    EXPECT_STREQ("Gadget", sprocketRecord->AsJson()["Values"][fp.GetFieldName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectsRelatedPropertyValuesWhenSelectingFromMultipleClassesAndFieldsAreMerged)
    {
    ECRelationshipClassCP rel_gs = GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_wg = GetClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP();

    // set up the dataset
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        });
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Widget"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_wg, *widget, *gadget1);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Sprocket"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_gs, *gadget2, *sprocket);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Sprocket", false, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Both,
        "RulesEngineTest:GadgetHasSprockets,WidgetHasGadget", "RulesEngineTest:Gadget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(9, descriptor->GetVisibleFields().size()); // 7 Widget properties (2 of them merged with Sprocket MyID and Description), Gadget.MyID, Sprocket.Gadget

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItem::FieldPropertyIdentifier fp(*content->GetDescriptor().GetVisibleFields()[7]->AsPropertiesField(), 0);
    ContentSetItemCPtr record = contentSet.Get(0);
    bvector<ECClassInstanceKey> keys = record->GetPropertyValueKeys(fp);
    ASSERT_EQ(2, keys.size());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadget1), keys[0]);
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*gadget2), keys[1]);
    EXPECT_STREQ("Gadget", record->AsJson()["Values"][fp.GetFieldName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectsNullRelatedPropertyValuesWhenSelectingFromMultipleClassesWithNoRelationships)
    {
    // set up the dataset
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Widget"));
        });
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        });
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Sprocket"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Widget,Gadget,Sprocket", false, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(10, descriptor->GetVisibleFields().size()); // 7 Widget properties (2 of them merged with MyID and Description of Gadget and Sprocket), Gadget.Widget, Sprocket.Gadget, Gadget.MyID

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    ContentSetItem::FieldPropertyIdentifier fp(*descriptor->GetVisibleFields()[9]->AsPropertiesField(), 0);

    ContentSetItemCPtr widgetRecord = contentSet.Get(0);
    bvector<ECClassInstanceKey> widgetKeys = widgetRecord->GetPropertyValueKeys(fp);
    EXPECT_TRUE(widgetKeys.empty());

    ContentSetItemCPtr gadgetRecord = contentSet.Get(1);
    bvector<ECClassInstanceKey> gadgetKeys = gadgetRecord->GetPropertyValueKeys(fp);
    EXPECT_TRUE(gadgetKeys.empty());

    ContentSetItemCPtr sprocketRecord = contentSet.Get(2);
    bvector<ECClassInstanceKey> sprocketKeys = sprocketRecord->GetPropertyValueKeys(fp);
    ASSERT_EQ(1, sprocketKeys.size());
    EXPECT_EQ(ECClassInstanceKey(m_gadgetClass, ECInstanceId()), sprocketKeys[0]);
    EXPECT_TRUE(sprocketRecord->AsJson()["Values"][fp.GetFieldName().c_str()].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectsRelatedPropertyValuesWhenSelectingFromDerivedRelatedInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="BaseRelatedClass">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="RelatedProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="DerivedRelatedClass">
        <BaseClass>BaseRelatedClass</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasRelatedInstances" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="is of" polymorphic="true">
            <Class class="Element" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="classifies" polymorphic="true">
            <Class class="BaseRelatedClass"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectsRelatedPropertyValuesWhenSelectingFromDerivedRelatedInstances)
    {
    ECClassCP elementClass = GetClass("Element");
    ECClassCP baseRelatedClass = GetClass("BaseRelatedClass");
    ECClassCP derivedRelatedClass = GetClass("DerivedRelatedClass");
    ECRelationshipClassCP rel = GetRelationshipClass("ElementHasRelatedInstances");

    // set up the dataset
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr relatedInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedRelatedClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element1, *relatedInstance1);
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr relatedInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseRelatedClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element2, *relatedInstance2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        rel->GetFullName(), baseRelatedClass->GetFullName(), "RelatedProperty", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(s_project->GetECDb(), ContentDisplayType::PropertyPane, 0, *KeySet::Create(), nullptr,
        options.GetJson()));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size()); // Element.ElementProperty + <BaseRelatedClass.RelatedProperty, DerivedRelatedClass.RelatedProperty>

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItem::FieldPropertyIdentifier fp(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);

    ContentSetItemCPtr record = contentSet.Get(0);
    bvector<ECClassInstanceKey> keys = record->GetPropertyValueKeys(fp);
    ASSERT_EQ(2, keys.size());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*relatedInstance1), keys[0]);
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*relatedInstance2), keys[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNullPropertyValueInstanceKeyWhenThereIsNoRelatedInstance)
    {
    // set up the dataset
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    ECClassInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldPropertyIdentifier fp0(*descriptor->GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(1, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetMyidFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp1(*descriptor->GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(1, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetDescriptionFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp2(*descriptor->GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(1, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey, gadgetWidgetFieldInstanceKeys[0]);

    ContentSetItem::FieldPropertyIdentifier fp3(*descriptor->GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> widgetMyIdFieldInstanceKeys = record->GetPropertyValueKeys(fp3);
    ASSERT_EQ(1, widgetMyIdFieldInstanceKeys.size());
    EXPECT_FALSE(widgetMyIdFieldInstanceKeys[0].IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNullPropertyValueInstanceKeyWhenThereIsNoRelatedInstance_MergedValuesCase)
    {
    // set up the dataset
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    ECClassInstanceKey gadgetKey1 = RulesEngineTestHelpers::GetInstanceKey(*gadget1);
    ECClassInstanceKey gadgetKey2 = RulesEngineTestHelpers::GetInstanceKey(*gadget2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", "RulesEngineTest:Gadget", false, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID", RelationshipMeaning::RelatedInstance));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(s_project->GetECDb(),
        nullptr, 0, *KeySet::Create(), nullptr, options.GetJson()));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size()); // Gadget.MyID, Gadget.Description, Gadget.Widget, Widget.MyID

    // set the "merge results" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);

    ContentSetItem::FieldPropertyIdentifier fp0(*content->GetDescriptor().GetVisibleFields()[0]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetMyidFieldInstanceKeys = record->GetPropertyValueKeys(fp0);
    ASSERT_EQ(2, gadgetMyidFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetMyidFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetMyidFieldInstanceKeys[1]);

    ContentSetItem::FieldPropertyIdentifier fp1(*content->GetDescriptor().GetVisibleFields()[1]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetDescriptionFieldInstanceKeys = record->GetPropertyValueKeys(fp1);
    ASSERT_EQ(2, gadgetDescriptionFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetDescriptionFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetDescriptionFieldInstanceKeys[1]);

    ContentSetItem::FieldPropertyIdentifier fp2(*content->GetDescriptor().GetVisibleFields()[2]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> gadgetWidgetFieldInstanceKeys = record->GetPropertyValueKeys(fp2);
    ASSERT_EQ(2, gadgetWidgetFieldInstanceKeys.size());
    EXPECT_EQ(gadgetKey1, gadgetWidgetFieldInstanceKeys[0]);
    EXPECT_EQ(gadgetKey2, gadgetWidgetFieldInstanceKeys[1]);

    ContentSetItem::FieldPropertyIdentifier fp3(*content->GetDescriptor().GetVisibleFields()[3]->AsPropertiesField(), 0);
    bvector<ECClassInstanceKey> widgetMyIdFieldInstanceKeys = record->GetPropertyValueKeys(fp3);
    ASSERT_EQ(2, widgetMyIdFieldInstanceKeys.size());
    EXPECT_FALSE(widgetMyIdFieldInstanceKeys[0].IsValid());
    EXPECT_FALSE(widgetMyIdFieldInstanceKeys[1].IsValid());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsDisplayLabelProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsDisplayLabelProperty)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("Custom label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ("B label", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesRelatedInstanceInFilterExpression, R"*(
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
    <ECEntityClass typeName="Element" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, UsesRelatedInstanceInFilterExpression)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP rel = GetClass("ModelContainsElements")->GetRelationshipClassCP();

    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR inst){inst.SetValue("ModelName", ECValue("a"));});
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model1, *element1);

    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR inst){inst.SetValue("ModelName", ECValue("b"));});
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model2, *element2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "model.ModelName = \"a\"", elementClass->GetFullName(), false, false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Backward, modelClass->GetFullName())), "model"));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*element1), contentSet[0]->GetKeys().front());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesMultiRelationshipRelatedInstanceInFilterExpression, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="ModelName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="ElementA" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementA" />
    <ECRelationshipClass typeName="A_To_B" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="refers" polymorphic="true">
            <Class class="ElementA"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is refered by" polymorphic="true">
            <Class class="ElementB" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ElementB" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, UsesMultiRelationshipRelatedInstanceInFilterExpression)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementAClass = GetClass("ElementA");
    ECClassCP elementBClass = GetClass("ElementB");
    ECRelationshipClassCP rel1 = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = GetClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR inst) {inst.SetValue("ModelName", ECValue("a")); });
    IECInstancePtr elementA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr elementB1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel1, *model1, *elementA1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel2, *elementA1, *elementB1);

    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR inst) {inst.SetValue("ModelName", ECValue("b")); });
    IECInstancePtr elementA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr elementB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel1, *model2, *elementA2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel2, *elementA2, *elementB2);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "model.ModelName = \"a\"", elementBClass->GetFullName(), false, false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({
        new RelationshipStepSpecification(rel2->GetFullName(), RequiredRelationDirection_Backward),
        new RelationshipStepSpecification(rel1->GetFullName(), RequiredRelationDirection_Backward)
        }), "model"));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*elementB1), contentSet[0]->GetKeys().front());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HandlesManyEndOfTheRelationshipInRelatedInstanceSpecification, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementName" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Model" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, HandlesManyEndOfTheRelationshipInRelatedInstanceSpecification)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP rel = GetClass("ModelContainsElements")->GetRelationshipClassCP();

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR inst) {inst.SetValue("ElementName", ECValue("a")); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR inst) {inst.SetValue("ElementName", ECValue("a")); });
    IECInstancePtr element3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [](IECInstanceR inst) {inst.SetValue("ElementName", ECValue("b")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element3);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "element.ElementName = \"a\"", modelClass->GetFullName(), false, false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({
        new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward),
        }), "element"));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*model), contentSet[0]->GetKeys().front());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesContentWithRelatedInstancesDerivingFromMultipleBaseClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ISubModeledElement" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.1.0">
                <AppliesToEntityClass>Element</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementA">
        <BaseClass>Element</BaseClass>
        <BaseClass>ISubModeledElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementB">
        <BaseClass>Element</BaseClass>
        <BaseClass>ISubModeledElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="LinkElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Model">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ModelA">
        <BaseClass>Model</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ModelB">
        <BaseClass>Model</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ModelModelsElement" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is contained by" polymorphic="true">
            <Class class="ISubModeledElement" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementHasLinks" strength="referencing" modifier="None">
        <Source multiplicity="(1..*)" roleLabel="has" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
            <Class class="LinkElement"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentWithRelatedInstancesDerivingFromMultipleBaseClasses)
    {
    ECClassCP modelClass = GetClass("Model");
    ECClassCP modelAClass = GetClass("ModelA");
    ECClassCP mixinClass = GetClass("ISubModeledElement");
    ECClassCP elementAClass = GetClass("ElementA");
    ECClassCP elementBClass = GetClass("ElementB");
    ECClassCP linkElementClass = GetClass("LinkElement");
    ECRelationshipClassCP rel_ModelContainsElements = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_ModelModelsElement = GetClass("ModelModelsElement")->GetRelationshipClassCP();
    ECRelationshipClassCP rel_ElementHasLinks = GetClass("ElementHasLinks")->GetRelationshipClassCP();

    IECInstancePtr modelA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelAClass);
    IECInstancePtr modeledElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr containedElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    IECInstancePtr linkElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *linkElementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_ModelContainsElements, *modelA, *containedElement);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_ModelModelsElement, *modelA, *modeledElement);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel_ElementHasLinks, *modeledElement, *linkElement);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementBClass->GetFullName(), false, false);
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({
        new RelationshipStepSpecification(rel_ModelContainsElements->GetFullName(), RequiredRelationDirection_Backward, modelClass->GetFullName()),
        new RelationshipStepSpecification(rel_ModelModelsElement->GetFullName(), RequiredRelationDirection_Forward, mixinClass->GetFullName()),
        new RelationshipStepSpecification(rel_ElementHasLinks->GetFullName(), RequiredRelationDirection_Forward, linkElementClass->GetFullName()),
        }), "link1", true));
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({
        new RelationshipStepSpecification(rel_ModelContainsElements->GetFullName(), RequiredRelationDirection_Backward, modelClass->GetFullName()),
        new RelationshipStepSpecification(rel_ModelModelsElement->GetFullName(), RequiredRelationDirection_Forward, mixinClass->GetFullName()),
        new RelationshipStepSpecification(rel_ElementHasLinks->GetFullName(), RequiredRelationDirection_Forward, linkElementClass->GetFullName()),
        }), "link2", true));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(RulesEngineTestHelpers::GetInstanceKey(*containedElement), contentSet[0]->GetKeys().front());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(CommonStrings::RULESENGINE_MULTIPLEINSTANCES, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(CommonStrings::RULESENGINE_MULTIPLEINSTANCES, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_STREQ(CommonStrings::RULESENGINE_MULTIPLEINSTANCES, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsClassWhenMergingRecordsAndClassesAreEqual, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsClassWhenMergingRecordsAndClassesAreEqual)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(classA, contentSet.Get(0)->GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsNullClassWhenMergingRecordsAndClassesDifferent, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsNullClassWhenMergingRecordsAndClassesDifferent)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    EXPECT_EQ(nullptr, contentSet.Get(0)->GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsPointPropertyContent, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsPointPropertyContent)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(DPoint3d::From(1, 2, 3)));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecificationP spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true));
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();

    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"(
        {
        "%s": {"x": 1.0, "y": 2.0, "z": 3.0}
        })", FIELD_NAME(classA, "Property")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);

    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"(
        {
        "%s": "X: 1.00 Y: 2.00 Z: 3.00"
        })", FIELD_NAME(classA, "Property")).c_str());
    EXPECT_EQ(expectedDisplayValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RecordFromDifferrentSpecificationsGetMerged, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, RecordFromDifferrentSpecificationsGetMerged)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("SharedProperty", ECValue("Test A")); });
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("SharedProperty", ECValue("Test B")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification *specB = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false);
    ContentInstancesOfSpecificClassesSpecification *specA = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*specB);
    rule->AddSpecification(*specA);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

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
    Utf8PrintfString expectedValue(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);
    EXPECT_STREQ(expectedValue.c_str(), recordJson["DisplayValues"][content->GetDescriptor().GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    EXPECT_TRUE(contentSet.Get(0)->IsMerged(content->GetDescriptor().GetVisibleFields()[0]->GetUniqueName()));
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); // no display label in the descriptor

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(3, content->GetDescriptor().GetVisibleFields().size()); // content created with a descriptor that has a display label

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ("Test", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size()); // no display label in the descriptor

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(3, content->GetDescriptor().GetVisibleFields().size()); // content created with a descriptor that has a display label

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ("Test", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, contentSet.Get(1)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClassesWithExcludedClassInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E">
        <BaseClass>D</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesWithExcludedClassInstances)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECClassCP classE = GetClass("E");

    //expected returned instances
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    //expected filtered out instances
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);

    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, false, "",
        bvector<MultiSchemaClass*> {new MultiSchemaClass(classA->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classA->GetName().c_str() })},
        bvector<MultiSchemaClass*> {
            new MultiSchemaClass(classB->GetSchema().GetName().c_str(), false, bvector<Utf8String> { classB->GetName().c_str() }),
            new MultiSchemaClass(classD->GetSchema().GetName().c_str(), true, bvector<Utf8String> { classD->GetName().c_str() })
        }, true));
    rules->AddPresentationRule(*rule);


    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::ShowLabels, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    auto selectedClasses = descriptor->GetSelectClasses();
    ASSERT_EQ(2, selectedClasses.size());
    ASSERT_EQ(a->GetClass().GetId(), selectedClasses[0].GetSelectClass().GetClass().GetId());
    ASSERT_EQ(c->GetClass().GetId(), selectedClasses[1].GetSelectClass().GetClass().GetId());
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
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, record1->GetDisplayLabelDefinition().GetDisplayValue().c_str());

    ContentSetItemCPtr record2 = contentSet.Get(1);
    EXPECT_STREQ("test value", record2->GetDisplayLabelDefinition().GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentDescriptorIsCachedWhenParametersEqual, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsCachedWhenParametersEqual)
    {
    ECClassCP classA = GetClass("A");

    // set up input
    KeySetPtr input = KeySet::Create(bvector<ECClassInstanceKey>{
        ECClassInstanceKey(classA, ECInstanceId((uint64_t)1)),
        ECClassInstanceKey(classA, ECInstanceId((uint64_t)2))
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));

    // verify the two objects are equal
    EXPECT_EQ(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentDescriptorIsNotCachedWhenParametersDifferent_Connection, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_Connection)
    {
    // set up a different connection
    ECDbTestProject project2;
    RulesEngineTestHelpers::ImportSchema(project2.Create("ContentDescriptorIsNotCachedWhenParametersDifferent_Connection"), [&](ECSchemaR schema)
        {
        ASSERT_STREQ(schema.GetName().c_str(), GetSchema()->GetName().c_str());

        ECEntityClassP classA = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(classA, "A"));
        ASSERT_TRUE(nullptr != classA);
        });

    IConnectionPtr connection2 = m_manager->GetConnections().CreateConnection(project2.GetECDb());

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", GetClass("A")->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(project2.GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    // verify the two objects are not equal
    EXPECT_NE(descriptor1, descriptor2);

    CloseConnection(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentDescriptorIsNotCachedWhenParametersDifferent_ContentDisplayType, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_ContentDisplayType)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", GetClass("A")->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Graphics, 0, *KeySet::Create())));
    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), ContentDisplayType::Grid, 0, *KeySet::Create())));

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_Provider, R"*(
    <ECEntityClass typeName="ClassA" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_Provider)
    {
    // set up selection 1
    SelectionInfoCPtr selection1 = SelectionInfo::Create("A", false);

    // set up selection 2
    SelectionInfoCPtr selection2 = SelectionInfo::Create("B", false);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", GetClass("ClassA")->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(), selection1.get())));
    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(), selection2.get())));

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_SubSelection, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_SubSelection)
    {
    // set up selection 1
    SelectionInfoCPtr selection1 = SelectionInfo::Create("", false);

    // set up selection 2
    SelectionInfoCPtr selection2 = SelectionInfo::Create("", true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", GetClass("A")->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create(), selection1.get())));
    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create(), selection2.get())));

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_Keys, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_SelectionInfo_Keys)
    {
    ECClassCP classA = GetClass("A");

    // set up input 1
    KeySetPtr input1 = KeySet::Create(bvector<ECClassInstanceKey>{
        ECClassInstanceKey(classA, ECInstanceId((uint64_t)1)),
        ECClassInstanceKey(classA, ECInstanceId((uint64_t)2))
        });

    // set up input 2
    KeySetPtr input2 = KeySet::Create(bvector<ECClassInstanceKey>{
        ECClassInstanceKey(classA, ECInstanceId((uint64_t)3)),
        ECClassInstanceKey(classA, ECInstanceId((uint64_t)4))
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input1)));
    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input2)));

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentDescriptorIsNotCachedWhenParametersDifferent_RulesetId, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsNotCachedWhenParametersDifferent_RulesetId)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set 1
    PresentationRuleSetPtr rules1 = PresentationRuleSet::CreateInstance("ContentDescriptorIsNotCachedWhenParametersDifferent_RulesetId_1");
    m_locater->AddRuleSet(*rules1);
    ContentRuleP rule1 = new ContentRule("", 1, false);
    rule1->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules1->AddPresentationRule(*rule1);

    // create the rule set 2
    PresentationRuleSetPtr rules2 = PresentationRuleSet::CreateInstance("ContentDescriptorIsNotCachedWhenParametersDifferent_RulesetId_2");
    m_locater->AddRuleSet(*rules2);
    ContentRuleP rule2 = new ContentRule("", 1, false);
    rule2->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules2->AddPresentationRule(*rule2);

    // request
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules1->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules2->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    // verify the two objects are equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentDescriptorIsRemovedFromCacheAfterConnectionClose, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentDescriptorIsRemovedFromCacheAfterConnectionClose)
    {
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", GetClass("A")->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor1 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    // simulate re-opening
    CloseConnection(*m_manager->GetConnections().GetConnection(s_project->GetECDb()));
    m_manager->GetConnections().CreateConnection(s_project->GetECDb());

    // request again
    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    // verify the two objects aren't equal
    EXPECT_NE(descriptor1, descriptor2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentModifierAppliesPropertyHidingOverride, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Description" typeName="string" />
        <ECProperty propertyName="MyID" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentModifierAppliesPropertyHidingOverride)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {
        instance.SetValue("Description", ECValue("TestDescription"));
        instance.SetValue("MyID", ECValue("TestID"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "A");
    rules->AddPresentationRule(*modifier);
    modifier->AddPropertyOverride(*new PropertySpecification("Description", 1000, "", nullptr, false));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // A.MyID

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestID", values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClassesSpecification_ContentModifierAppliesCalculatedPropertiesSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecification_ContentModifierAppliesCalculatedPropertiesSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("SharedProperty", ECValue("InstanceA"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("SharedProperty", ECValue("InstanceB"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "B");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.SharedProperty"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(4, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_STREQ("InstanceB", jsonValues2["CalculatedProperty_0"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClassesSpecification_ContentModifierOnBaseClassPropertyIsAppliedToOnlyOneChildClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B" modifier="Abstract">
        <ECProperty propertyName="Property" typeName="string" />
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_CAnd_D" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_CAnd_D" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_CAnd_D"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="references" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="True" abstractConstraint="B">
            <Class class="C" />
            <Class class="D" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecification_ContentModifierOnBaseClassPropertyIsAppliedToOnlyOneChildClass)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP rel = GetClass("A_Has_CAnd_D")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    ECInstanceId instanceAId = RulesEngineTestHelpers::GetInstanceKey(*instanceA).GetId();
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [&](IECInstanceR instance) { instance.SetValue("Property", ECValue("C")); instance.SetValue("A", ECValue(instanceAId, rel)); });
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, [&](IECInstanceR instance) { instance.SetValue("Property", ECValue("D")); instance.SetValue("A", ECValue(instanceAId, rel)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:C,D", GetSchema()->GetName().c_str()), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName().c_str(), "D");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.Property"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("C", jsonValues[FIELD_NAME(classB, "Property")].GetString());
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), jsonValues[FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), jsonValues[FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")]["ECInstanceId"].GetString());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_STREQ("D", jsonValues2[FIELD_NAME(classB, "Property")].GetString());
    EXPECT_STREQ("D", jsonValues2["CalculatedProperty_0"].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), jsonValues[FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), jsonValues[FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClassesSpecification_ContentModifierIsAppliedToOnlyOneChildClassPolymorphically, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived1">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Derived2">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClassesSpecification_ContentModifierIsAppliedToOnlyOneChildClassPolymorphically)
    {
    // set up data set
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derived1Class = GetClass("Derived1")->GetEntityClassCP();
    ECClassCP derived2Class = GetClass("Derived2")->GetEntityClassCP();
    IECInstancePtr derived1Instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derived1Class,
        [](IECInstanceR instance) { instance.SetValue("Property", ECValue("Derived1")); });
    IECInstancePtr derived2Instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derived2Class,
        [](IECInstanceR instance) { instance.SetValue("Property", ECValue("Derived2")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "Derived2");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.Property"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("Derived1", jsonValues[FIELD_NAME(baseClass, "Property")].GetString());
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_STREQ("Derived2", jsonValues2[FIELD_NAME(baseClass, "Property")].GetString());
    EXPECT_STREQ("Derived2", jsonValues2["CalculatedProperty_0"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstancesSpecification_ContentModifierAppliesCalculatedPropertiesSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyB" typeName="string" />
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_ContentModifierAppliesCalculatedPropertiesSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("SharedProperty", ECValue("InstanceA"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("SharedProperty", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, relationshipAHasB->GetFullName(), classA->GetFullName()));
    rules->AddPresentationRule(*rule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "A");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.SharedProperty"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("InstanceA", jsonValues[FIELD_NAME(classA, "SharedProperty")].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classA, "PropertyA")].IsNull());
    EXPECT_STREQ("InstanceA", jsonValues["CalculatedProperty_0"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstancesSpecification_GetsRelatedValueThroughCalculatedProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="SharedProperty" typeName="string" />
        <ECProperty propertyName="PropertyB" typeName="string" />
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
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_GetsRelatedValueThroughCalculatedProperty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert some instances
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("SharedProperty", ECValue("InstanceA"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance){instance.SetValue("SharedProperty", ECValue("InstanceB"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Backward, relationshipAHasB->GetFullName(), classA->GetFullName()));
    rules->AddPresentationRule(*rule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "A");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 1200, Utf8PrintfString("this.GetRelatedValue(\"%s\", \"Forward\", \"%s\", \"SharedProperty\")", relationshipAHasB->GetFullName(), classB->GetFullName())));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("InstanceA", jsonValues[FIELD_NAME(classA, "SharedProperty")].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classA, "PropertyA")].IsNull());
    EXPECT_STREQ("InstanceB", jsonValues["CalculatedProperty_0"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstancesSpecification_ContentModifierAppliesCalculatedPropertiesSpecificationPolymorphically, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="PropertyC" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="references" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_ContentModifierAppliesCalculatedPropertiesSpecificationPolymorphically)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // set up data set
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR instance) { instance.SetValue("PropertyC", ECValue(1000)); });
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceD);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, relationshipAHasB->GetFullName(), classB->GetFullName()));
    rules->AddPresentationRule(*rule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "C");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 900, "this.PropertyC"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(3, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), jsonValues[FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), jsonValues[FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    EXPECT_TRUE(jsonValues[FIELD_NAME(classC, "PropertyC")].IsNull());
    EXPECT_TRUE(jsonValues["CalculatedProperty_0"].IsNull());

    rapidjson::Document jsonDoc1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues1 = jsonDoc1["Values"];
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), jsonValues1[FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), jsonValues1[FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    EXPECT_EQ(1000, jsonValues1[FIELD_NAME(classC, "PropertyC")].GetInt());
    EXPECT_STREQ("1000", jsonValues1["CalculatedProperty_0"].GetString());

    rapidjson::Document jsonDoc2 = contentSet.Get(2)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), jsonValues2[FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), jsonValues2[FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    EXPECT_TRUE(jsonValues2[FIELD_NAME(classC, "PropertyC")].IsNull());
    EXPECT_TRUE(jsonValues2["CalculatedProperty_0"].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstancesSpecification_ContentModifierDoesNotApplyCalculatedPropertyForNonExistingClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropertyB" typeName="int" />
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="references" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstancesSpecification_ContentModifierDoesNotApplyCalculatedPropertyForNonExistingClass)
    {
    ECEntityClassCP classA = GetClass("A")->GetEntityClassCP();
    ECEntityClassCP classB = GetClass("B")->GetEntityClassCP();
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // insert instances
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceE = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) { instance.SetValue("PropertyB", ECValue(12345)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceD, *instanceE);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceD);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, relationshipAHasB->GetFullName(), classB->GetFullName()));
    rules->AddPresentationRule(*rule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "NonExistingClass"); // There is no such class in database
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 900, "this.PropertyB"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));

    //Calculated property is not applied
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_EQ(12345, jsonValues[FIELD_NAME(classB, "PropertyB")].GetInt());
    EXPECT_EQ(instanceD->GetClass().GetId().ToString(), jsonValues[FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceD->GetInstanceId(), jsonValues[FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_ContentModifierAppliesCalculatedPropertiesSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ContentModifierAppliesCalculatedPropertiesSpecification)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // insert some instances
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("InstanceA"));});

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", false));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "A");
    rules->AddPresentationRule(*modifier);
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("label2", 1200, "this.Property"));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document jsonDoc = contentSet.Get(0)->AsJson();
    RapidJsonValueCR jsonValues = jsonDoc["Values"];
    EXPECT_STREQ("InstanceA", jsonValues[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "Property")].GetString());
    EXPECT_STREQ("InstanceA", jsonValues["CalculatedProperty_0"].GetString());

    rapidjson::Document jsonDoc2 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR jsonValues2 = jsonDoc2["Values"];
    EXPECT_TRUE(jsonValues2[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "Property")].IsNull());
    EXPECT_TRUE(jsonValues2["CalculatedProperty_0"].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsCorrectEnumValues, R"*(
    <ECEnumeration typeName="IntegerEnum" backingTypeName="int" isStrict="True" description="" displayLabel="IntegerEnum">
        <ECEnumerator value="1" displayLabel="Z" />
        <ECEnumerator value="2" displayLabel="M" />
        <ECEnumerator value="3" displayLabel="A" />
    </ECEnumeration>
    <ECEnumeration typeName="StringEnum" backingTypeName="string" isStrict="True" description="" displayLabel="StringEnum">
        <ECEnumerator value="One" displayLabel="3" />
        <ECEnumerator value="Two" displayLabel="2" />
        <ECEnumerator value="Three" displayLabel="1" />
    </ECEnumeration>
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntEnum" typeName="IntegerEnum" />
        <ECProperty propertyName="StrEnum" typeName="StringEnum" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsCorrectEnumValues)
    {
    // set up data set
    ECClassCP classA = GetClass("A")->GetEntityClassCP();
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("IntEnum", ECValue(2));
        instance.SetValue("StrEnum", ECValue("Three"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values = recordJson["Values"];
    RapidJsonValueCR dispalyValues = recordJson["DisplayValues"];
    EXPECT_EQ(2, values[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetInt());
    EXPECT_STREQ("M", dispalyValues[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    EXPECT_STREQ("Three", values[descriptor->GetVisibleFields()[1]->GetUniqueName().c_str()].GetString());
    EXPECT_STREQ("1", dispalyValues[descriptor->GetVisibleFields()[1]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_AppliesWithContentInstancesOfSpecificClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_AppliesWithContentInstancesOfSpecificClasses)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("Test"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true));

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

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("Test", values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_AppliesWithSelectedNodeInstances, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_AppliesWithSelectedNodeInstances)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("Test"));});

    // set up input
    KeySetPtr input = KeySet::Create(*instance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification(1, false, "", "", false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("Test", values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_AppliesWithContentRelatedInstances, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="references" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="True">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_AppliesWithContentRelatedInstances)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("Test"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentRelatedInstancesSpecification* spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, "", classA->GetFullName());
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("Test", values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesHigherPriorityDisplayOverride, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesHigherPriorityDisplayOverride)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("Test")); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true));
    spec->AddPropertyOverride(*new PropertySpecification("Property", 900, "", nullptr, false));

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

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("Test", values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesHigherPriorityHideOverride, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesHigherPriorityHideOverride)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property1", ECValue("Test"));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property1", 900, "", nullptr, true));
    spec->AddPropertyOverride(*new PropertySpecification("Property2", 900, "", nullptr, true));
    spec->AddPropertyOverride(*new PropertySpecification("Property2", 1000, "", nullptr, false));

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

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("Test", values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesByOrderOfDefinitionIfPrioritiesEqual, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property1" typeName="int" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesByOrderOfDefinitionIfPrioritiesEqual)
    {
    ECClassCP classA = GetClass("A");

    // set up the dataset
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property1", ECValue(1));});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property2", 1000, "", nullptr, false));
    spec->AddPropertyOverride(*new PropertySpecification("Property2", 1000, "", nullptr, true));
    spec->AddPropertyOverride(*new PropertySpecification("Property1", 1000, "", nullptr, true));

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

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_EQ(1, values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesSpecificationOverrideIfPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesSpecificationOverrideIfPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", nullptr, false));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", nullptr, true));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(0, descriptor->GetVisibleFields().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesModifierOverSpecificationOverrideIfPriorityHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesModifierOverSpecificationOverrideIfPriorityHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", nullptr, false));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", nullptr, true));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME(classA, "UserLabel"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECRelationshipClassCP rel = GetClass("ClassAHasClassB")->GetRelationshipClassCP();

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), false, false);
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", nullptr, false) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", nullptr, true));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // ClassB_UserLabel
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher)
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
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", nullptr, false) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", nullptr, true));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassB_UserLabel, ClassB_ClassA_UserLabel
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Polymorphism_AppliesDisplayFromBaseClassOnDerivedClass, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="BaseProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="DerivedProperty" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Polymorphism_AppliesDisplayFromBaseClassOnDerivedClass)
    {
    // set up the dataset
    ECClassCP derivedClass = GetClass("Derived");
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass, [](IECInstanceR instance) {
        instance.SetValue("BaseProperty", ECValue(10));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", derivedClass->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1500, "", nullptr, true)); // base class specification
    spec->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "", nullptr, false));
    spec->AddPropertyOverride(*new PropertySpecification("DerivedProperty", 1000, "", nullptr, false));

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

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_EQ(10, values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_Polymorphism_AppliesHideFromBaseClassOnDerivedClass, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="BaseProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="DerivedProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_Polymorphism_AppliesHideFromBaseClassOnDerivedClass)
    {
    // set up the dataset
    ECClassCP derivedClass = GetClass("Derived");
    IECInstancePtr instanceF = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass, [](IECInstanceR instance)
        {
        instance.SetValue("BaseProperty", ECValue(123));
        instance.SetValue("DerivedProperty", ECValue(456));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", derivedClass->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "Base");
    rules->AddPresentationRule(*modifier);
    modifier->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "", nullptr, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassE_IntProperty, ClassF_PropertyF

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(123, values[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetInt());
    EXPECT_EQ(456, values[descriptor->GetVisibleFields()[1]->GetUniqueName().c_str()].GetInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_EvaluateECExpression, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_EvaluateECExpression)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property1", ECValue("TestValue1"));
        instance.SetValue("Property2", ECValue("TestValue2"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property1", 1000, "", nullptr, "2>1"));

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

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestValue1", values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_EvaluateECExpression_DoNotHideOtherProperties, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_EvaluateECExpression_DoNotHideOtherProperties)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property1", ECValue("TestValue1"));
        instance.SetValue("Property2", ECValue("TestValue2"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property1", 1000, "", nullptr, "false", nullptr, nullptr, true));

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

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestValue2", values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_ECExpressionIsNotEvaluatedIntoBoolean, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_ECExpressionIsNotEvaluatedIntoBoolean)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property1", ECValue("TestValue1"));
        instance.SetValue("Property2", ECValue("TestValue2"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property1", 1000, "", nullptr, "abc"));

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

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR values1 = recordJson["Values"];
    EXPECT_STREQ("TestValue1", values1[descriptor->GetVisibleFields()[0]->GetUniqueName().c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyDisplayOverride_UseRulesetVariableInECExpression, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="Property1" typeName="string" />
        <ECProperty propertyName="Property2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyDisplayOverride_UseRulesetVariableInECExpression)
    {
    // set up data set
    ECClassCP classA = GetClass("ClassA");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Property1", ECValue("TestValue1"));
        instance.SetValue("Property2", ECValue("TestValue2"));
        });

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    spec->AddPropertyOverride(*new PropertySpecification("Property1", 1000, "", nullptr, "GetVariableBoolValue(\"show_property\")"));

    // validate descriptor
    RulesetVariables variables({ RulesetVariableEntry("show_property", false) });
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), variables, nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(0, descriptor->GetVisibleFields().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyRendererOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="PropertyOnA" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyRendererOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyOverride(*new PropertySpecification("PropertyOnA", 1, "", nullptr, nullptr, new CustomRendererSpecification("Specification Renderer")));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("PropertyOnA", 1, "", nullptr, nullptr, new CustomRendererSpecification("Modifier Renderer")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    ASSERT_NE(nullptr, descriptor->GetVisibleFields()[0]->GetRenderer());
    EXPECT_STREQ("Specification Renderer", descriptor->GetVisibleFields()[0]->GetRenderer()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyRendererOverride_Priorities_AppliesModifierOverSpecificationOverrideWhenPriorityHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="PropertyOnA" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyRendererOverride_Priorities_AppliesModifierOverSpecificationOverrideWhenPriorityHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyOverride(*new PropertySpecification("PropertyOnA", 1, "", nullptr, nullptr, new CustomRendererSpecification("Specification Renderer")));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("PropertyOnA", 2, "", nullptr, nullptr, new CustomRendererSpecification("Modifier Renderer")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    ASSERT_NE(nullptr, descriptor->GetVisibleFields()[0]->GetRenderer());
    EXPECT_STREQ("Modifier Renderer", descriptor->GetVisibleFields()[0]->GetRenderer()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyRendererOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="PropertyOnAOrB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="PropertyOnAOrB" typeName="string" />
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyRendererOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual)
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
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        {new PropertySpecification("PropertyOnAOrB", 1, "", nullptr, nullptr, new CustomRendererSpecification("Related Property Renderer"))}, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("PropertyOnAOrB", 1, "", nullptr, nullptr, new CustomRendererSpecification("Modifier Renderer")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_PropertyOnAOrB, ClassA_ClassB
    ASSERT_EQ(1, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().size()); // ClassB_PropertyOnAOrB
    ASSERT_NE(nullptr, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetRenderer());
    EXPECT_STREQ("Related Property Renderer", descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetRenderer()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyRendererOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="PropertyOnAOrB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <ECProperty propertyName="PropertyOnAOrB" typeName="string" />
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyRendererOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher)
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
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        {new PropertySpecification("PropertyOnAOrB", 1, "", nullptr, nullptr, new CustomRendererSpecification("Related Property Renderer"))}, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("PropertyOnAOrB", 2, "", nullptr, nullptr, new CustomRendererSpecification("Modifier Renderer")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_PropertyOnAOrB, ClassA_ClassB
    ASSERT_EQ(1, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().size()); // ClassB_PropertyOnAOrB
    ASSERT_NE(nullptr, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetRenderer());
    EXPECT_STREQ("Modifier Renderer", descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetRenderer()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyRendererOverride_FieldsMerging_GetOneFieldWhenPropertiesAndRenderersAreSimilar, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyRendererOverride_FieldsMerging_GetOneFieldWhenPropertiesAndRenderersAreSimilar)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true, new CustomRendererSpecification("TestRenderer")));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ASSERT_NE(nullptr, descriptor->GetVisibleFields()[0]->GetRenderer());
    EXPECT_STREQ("TestRenderer", descriptor->GetVisibleFields()[0]->GetRenderer()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyRendererOverride_FieldsMerging_GetDifferentFieldsWhenPropertiesAndRenderersAreDifferent, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyRendererOverride_FieldsMerging_GetDifferentFieldsWhenPropertiesAndRenderersAreDifferent)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true, new CustomRendererSpecification("TestRenderer1")));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "B");
    modifier->AddPropertyOverride(*new PropertySpecification("Property", 1500, "", nullptr, true, new CustomRendererSpecification("TestRenderer2")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    ASSERT_NE(nullptr, descriptor->GetVisibleFields()[0]->GetRenderer());
    EXPECT_STREQ("TestRenderer1", descriptor->GetVisibleFields()[0]->GetRenderer()->GetName().c_str());

    ASSERT_NE(nullptr, descriptor->GetVisibleFields()[1]->GetRenderer());
    EXPECT_STREQ("TestRenderer2", descriptor->GetVisibleFields()[1]->GetRenderer()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyRendererOverride_Polymorphism_AppliesFromBaseClassOnDerivedClass, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="BaseProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyRendererOverride_Polymorphism_AppliesFromBaseClassOnDerivedClass)
    {
    // set up the dataset
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");
    IECInstancePtr derivedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass);
    IECInstancePtr baseInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{derivedInstance, baseInstance});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "", nullptr, true, nullptr, nullptr));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier("RulesEngineTest", "Base");
    modifier->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "", nullptr, nullptr, new CustomRendererSpecification("TestRenderer")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ASSERT_NE(nullptr, descriptor->GetVisibleFields()[0]->GetRenderer());
    EXPECT_STREQ("TestRenderer", descriptor->GetVisibleFields()[0]->GetRenderer()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyRendererOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyRendererOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass)
    {
    // set up the dataset
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceC, instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("PropertyA", 1000, "", nullptr, true, nullptr, nullptr));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "B");
    modifier->AddPropertyOverride(*new PropertySpecification("PropertyA", 1500, "", nullptr, nullptr, new CustomRendererSpecification("TestRenderer")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_EQ(nullptr, descriptor->GetVisibleFields()[0]->GetRenderer());

    ASSERT_NE(nullptr, descriptor->GetVisibleFields()[1]->GetRenderer());
    EXPECT_STREQ("TestRenderer", descriptor->GetVisibleFields()[1]->GetRenderer()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyRendererOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyRendererOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty)
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
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), aspectClass->GetFullName(),
        {new PropertySpecification("UserLabel", 1, "", nullptr, nullptr, new CustomRendererSpecification("test renderer"))}, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_Aspect
    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsNestedContentField());
    ASSERT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ASSERT_NE(nullptr, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetRenderer());
    EXPECT_STREQ("test renderer", descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetRenderer()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("Custom Editor 1")));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("Custom Editor 2")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("Custom Editor 1", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Priorities_AppliesModifierOverSpecificationOverrideWhenPriorityHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Priorities_AppliesModifierOverSpecificationOverrideWhenPriorityHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("Custom Editor 1")));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("Custom Editor 2")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("Custom Editor 2", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual)
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
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("Custom Editor 1")) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("Custom Editor 2")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB
    ASSERT_EQ(1, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().size()); // ClassB_UserLabel
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetEditor());
    EXPECT_STREQ("Custom Editor 1", descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityHigher)
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
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("Custom Editor 1")) }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("Custom Editor 2")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB
    ASSERT_EQ(1, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().size()); // ClassB_UserLabel
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetEditor());
    EXPECT_STREQ("Custom Editor 2", descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_FieldsMerging_GetOneFieldWhenPropertiesAndEditorsAreSimilar, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_FieldsMerging_GetOneFieldWhenPropertiesAndEditorsAreSimilar)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true, nullptr, new PropertyEditorSpecification("TestEditor")));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("TestEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_FieldsMerging_GetDifferentFieldsWhenPropertiesAndEditorsAreDifferent, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_FieldsMerging_GetDifferentFieldsWhenPropertiesAndEditorsAreDifferent)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true, nullptr, new PropertyEditorSpecification("EditorA")));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "B");
    modifier->AddPropertyOverride(*new PropertySpecification("Property", 1500, "", nullptr, true, nullptr, new PropertyEditorSpecification("EditorB")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("EditorA", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[1]->GetEditor());
    EXPECT_STREQ("EditorB", descriptor->GetVisibleFields()[1]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Polymorphism_AppliesFromBaseClassOnDerivedClass, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="BaseProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Polymorphism_AppliesFromBaseClassOnDerivedClass)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    // set up the dataset
    IECInstancePtr derivedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass);
    IECInstancePtr baseInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{derivedInstance, baseInstance});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "", nullptr, true, nullptr));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "Base");
    modifier->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("TestEditor")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->GetEditor());
    EXPECT_STREQ("TestEditor", descriptor->GetVisibleFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="BaseProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    // set up the dataset
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceC, instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "", nullptr, true, nullptr));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "B");
    modifier->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1500, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("TestEditor")));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_TRUE(nullptr == descriptor->GetVisibleFields()[0]->GetEditor());

    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[1]->GetEditor());
    EXPECT_STREQ("TestEditor", descriptor->GetVisibleFields()[1]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyEditorOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyEditorOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty)
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
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), aspectClass->GetFullName(),
        {new PropertySpecification("UserLabel", 1, "", nullptr, nullptr, nullptr, new PropertyEditorSpecification("test editor"))}, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_Aspect
    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsNestedContentField());
    ASSERT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    ASSERT_TRUE(nullptr != descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetEditor());
    EXPECT_STREQ("test editor", descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetEditor()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_FieldsMerging_GetOneFieldWhenPropertyLabelsAreEqual, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_FieldsMerging_GetOneFieldWhenPropertyLabelsAreEqual)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "Custom Property Label", nullptr, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "Property"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("Custom Property Label", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_FieldsMerging_GetDifferentFieldsWhenPropertyLabelsAreDifferent, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_FieldsMerging_GetDifferentFieldsWhenPropertyLabelsAreDifferent)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // set up the dataset
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("Property", 1000, "", nullptr, true));

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "A");
    modifier->AddPropertyOverride(*new PropertySpecification("Property", 1000, "Custom A Property Label", nullptr, true));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_STREQ(FIELD_NAME(classA, "Property"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("Custom A Property Label", descriptor->GetVisibleFields()[0]->GetLabel().c_str());

    EXPECT_STREQ(FIELD_NAME(classB, "Property"), descriptor->GetVisibleFields()[1]->GetUniqueName().c_str());
    EXPECT_STREQ("Property", descriptor->GetVisibleFields()[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Polymorphism_FromBaseClassOnDerivedClass, R"*(
    <ECEntityClass typeName="Base">
        <ECProperty propertyName="BaseProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Derived">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Polymorphism_FromBaseClassOnDerivedClass)
    {
    ECClassCP baseClass = GetClass("Base");
    ECClassCP derivedClass = GetClass("Derived");

    // set up the dataset
    IECInstancePtr derivedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClass);
    IECInstancePtr baseInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *baseClass);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{derivedInstance, baseInstance});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "", nullptr, true));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "Base");
    modifier->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "Custom Base Property Label", nullptr, nullptr));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());
    EXPECT_STREQ(FIELD_NAME(baseClass, "BaseProperty"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("Custom Base Property Label", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="BaseProperty" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Polymorphism_DoesNotApplyFromDerivedClassOnBaseClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    // set up the dataset
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceC, instanceB, instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "", nullptr, true));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(GetSchema()->GetName(), "B");
    modifier->AddPropertyOverride(*new PropertySpecification("BaseProperty", 1000, "Custom B Property Label", nullptr, nullptr));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_STREQ(FIELD_NAME(classA, "BaseProperty"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("BaseProperty", descriptor->GetVisibleFields()[0]->GetLabel().c_str());

    EXPECT_STREQ(FIELD_NAME_C(classA, "BaseProperty", 2), descriptor->GetVisibleFields()[1]->GetUniqueName().c_str());
    EXPECT_STREQ("Custom B Property Label", descriptor->GetVisibleFields()[1]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Priorities_AppliesSpecificationOverrideWhenPrioritiesEqual)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "Custom Label 1"));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "Custom Label 2"));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassA_UserLabel
    EXPECT_STREQ("Custom Label 1", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Priorities_AppliesModifierOverSpecOverrideWhenPriorityIsHigher, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="UserLabel" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Priorities_AppliesModifierOverSpecOverrideWhenPriorityIsHigher)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    spec->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "Custom Label 1"));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "Custom Label 2"));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size()); // ClassA_UserLabel
    EXPECT_STREQ("Custom Label 2", descriptor->GetVisibleFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Priorities_AppliesRelatedPropertyOverrideWhenPrioritiesEqual)
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
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "Custom A Label 1") }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 1, "Custom A Label 2"));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB
    ASSERT_EQ(1, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().size()); // ClassB_UserLabel
    EXPECT_STREQ("Custom A Label 1", descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityIsHigher, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_Priorities_AppliesModifierOverRelatedPropertyOverrideWhenPriorityIsHigher)
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
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, rel->GetFullName(), classA->GetFullName(),
        { new PropertySpecification("UserLabel", 1, "Custom A Label 1") }, RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    ContentModifierP modifier = new ContentModifier(classA->GetSchema().GetName(), classA->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("UserLabel", 2, "Custom A Label 2"));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // ClassA_UserLabel, ClassA_ClassB
    ASSERT_EQ(1, descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().size()); // ClassB_UserLabel
    EXPECT_STREQ("Custom A Label 2", descriptor->GetVisibleFields().back()->AsNestedContentField()->AsRelatedContentField()->GetFields().back()->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyLabelOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty, R"*(
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
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyLabelOverride_AppliesRelatedPropertyOverrideOnNestedContentProperty)
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
    modifier->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, rel->GetFullName(), aspectClass->GetFullName(),
        {new PropertySpecification("UserLabel", 1, "Custom Label")}, RelationshipMeaning::RelatedInstance));
    rules->AddPresentationRule(*modifier);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size()); // Element_Aspect
    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsNestedContentField());
    ASSERT_EQ(1, descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields().size());
    EXPECT_STREQ("Custom Label", descriptor->GetVisibleFields()[0]->AsNestedContentField()->GetFields()[0]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyIsReadOnlyOverride, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyIsReadOnlyOverride)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", nullptr, nullptr, nullptr, nullptr, false, true));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    ASSERT_TRUE(descriptor->GetVisibleFields()[0]->IsReadOnly());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(PropertyPriorityOverride, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, PropertyPriorityOverride)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);
    spec->AddPropertyOverride(*new PropertySpecification("Prop", 1000, "", nullptr, nullptr, nullptr, nullptr, false, nullptr, 20));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    ASSERT_EQ(20, descriptor->GetVisibleFields()[0]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstance_GetNavigationPropertyValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
        <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetNavigationPropertyValue)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // set up the dataset
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("InstanceA"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\"", 1, "this.Property", ""));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    EXPECT_STREQ("InstanceA", recordJson["DisplayValues"][FIELD_NAME(classB, "A")].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#919256
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetNavigationPropertyValueWhenThereIsPropertyWithItsKeyFieldName, R"*(
    <ECEntityClass typeName="Element">
        <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
        <ECProperty propertyName="Model_Id" typeName="string" />
    </ECEntityClass>
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
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetNavigationPropertyValueWhenThereIsPropertyWithItsKeyFieldName)
    {
    // set up the dataset
    ECRelationshipClassCP rel = GetClass("ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP elementClass = GetClass("Element");
    ECClassCP modelClass = GetClass("Model");
    IECInstancePtr modelInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr elementInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass, [&modelInstance, rel](IECInstanceR instance)
        {
        ECInstanceId modelId;
        ECInstanceId::FromString(modelId, modelInstance->GetInstanceId().c_str());
        instance.SetValue("Model", ECValue(modelId, rel));
        instance.SetValue("Model_Id", ECValue("Test"));
        });

    // set up input
    KeySetPtr input = KeySet::Create(*elementInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_EQ(2, descriptor->GetVisibleFields().size()); // Model, Model_Id

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_STREQ(GetDefaultDisplayLabel(*modelInstance).c_str(), displayValues[FIELD_NAME(elementClass, "Model")].GetString());
    EXPECT_STREQ("Test", displayValues[FIELD_NAME(elementClass, "Model_Id")].GetString());

    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(modelInstance->GetClass().GetId().ToString(), values[FIELD_NAME(elementClass, "Model")]["ECClassId"].GetString());
    EXPECT_EQ(modelInstance->GetInstanceId(), values[FIELD_NAME(elementClass, "Model")]["ECInstanceId"].GetString());
    EXPECT_STREQ("Test", values[FIELD_NAME(elementClass, "Model_Id")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstance_GetNavigationPropertyValue_InstanceLabelOverride, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
        <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetNavigationPropertyValue_InstanceLabelOverride)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();

    // set up the dataset
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("InstanceA"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    EXPECT_STREQ("InstanceA", recordJson["DisplayValues"][FIELD_NAME(classB, "A")].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstance_GetOneFieldForSimilarNavigationProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B" modifier="Abstract">
        <ECProperty propertyName="Property" typeName="string" />
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_C_And_D" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_C_And_D" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_C_And_D"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="references" polymorphic="True">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="True" abstractConstraint="B">
            <Class class="C" />
            <Class class="D" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetOneFieldForSimilarNavigationProperties)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");
    ECRelationshipClassCP classAHasCAndD = GetClass("A_Has_C_And_D")->GetRelationshipClassCP();

    // set up the dataset
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasCAndD, *instanceA, *instanceC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasCAndD, *instanceA, *instanceD);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceC, instanceD});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    EXPECT_EQ(GetDefaultDisplayLabel(*instanceA), recordJson["DisplayValues"][FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson["Values"][FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson["Values"][FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")]["ECInstanceId"].GetString());

    rapidjson::Document recordJson1 = contentSet.Get(1)->AsJson();
    EXPECT_EQ(GetDefaultDisplayLabel(*instanceA), recordJson1["DisplayValues"][FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson1["Values"][FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson1["Values"][FIELD_NAME((bvector<ECClassCP>{classC, classD}), "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstance_GetDifferentFieldsForDifferentNavigationProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropertyA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropertyB" typeName="string" />
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropertyC" typeName="string" />
        <ECNavigationProperty propertyName="B" relationshipName="B_Has_C" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="False">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="False">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="False">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetDifferentFieldsForDifferentNavigationProperties)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipBHasC = GetClass("B_Has_C")->GetRelationshipClassCP();

    // set up the dataset
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipBHasC, *instanceB, *instanceC);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceC});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_EQ(4, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    EXPECT_EQ(GetDefaultDisplayLabel(*instanceA), displayValues[FIELD_NAME(classB, "A")].GetString());
    EXPECT_TRUE(displayValues[FIELD_NAME(classC, "B")].IsNull());
    RapidJsonValueCR values = recordJson["Values"];
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), values[FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), values[FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    EXPECT_TRUE(values[FIELD_NAME(classC, "B")].IsNull());

    rapidjson::Document recordJson1 = contentSet.Get(1)->AsJson();
    RapidJsonValueCR displayValues1 = recordJson1["DisplayValues"];
    EXPECT_EQ(GetDefaultDisplayLabel(*instanceB), displayValues1[FIELD_NAME(classC, "B")].GetString());
    EXPECT_TRUE(displayValues1[FIELD_NAME(classB, "A")].IsNull());
    RapidJsonValueCR values1 = recordJson1["Values"];
    EXPECT_EQ(instanceB->GetClass().GetId().ToString(), values1[FIELD_NAME(classC, "B")]["ECClassId"].GetString());
    EXPECT_EQ(instanceB->GetInstanceId(), values1[FIELD_NAME(classC, "B")]["ECInstanceId"].GetString());
    EXPECT_TRUE(values1[FIELD_NAME(classB, "A")].IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstance_GetCorrectValuesWhenNavigationPropertiesPointsToDifferentClassesButAreInTheSameField, R"*(
    <ECEntityClass typeName="ClassA">
        <ECProperty propertyName="MyID" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="BaseOfBAndC" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="MyID" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
        <BaseClass>BaseOfBAndC</BaseClass>
        <ECNavigationProperty propertyName="A" relationshipName="ClassAHasBAndC" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassC">
        <BaseClass>BaseOfBAndC</BaseClass>
        <ECNavigationProperty propertyName="A" relationshipName="ClassAHasBAndC" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassAHasBAndC"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="ClassA Has B and C" polymorphic="True">
            <Class class="ClassA" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B and C Classes Have A" polymorphic="True" abstractConstraint="BaseOfBAndC">
            <Class class="ClassB" />
            <Class class="ClassC" />
        </Target>
    </ECRelationshipClass>

    <ECEntityClass typeName="ClassA2Base">
        <ECProperty propertyName="PropertyA2Base" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB2">
        <ECNavigationProperty propertyName="A" relationshipName="ClassA2BaseHasB2" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ClassA2BaseHasB2"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="ClassA2Base Has ClassB2" polymorphic="False">
            <Class class="ClassA2Base" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="ClassB2 Has ClassA2Base" polymorphic="False">
            <Class class="ClassB2" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetCorrectValuesWhenNavigationPropertiesPointsToDifferentClassesButAreInTheSameField)
    {
    // set up the dataset
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    ECClassCP classA2Base = GetClass("ClassA2Base");
    ECClassCP classB2 = GetClass("ClassB2");
    ECRelationshipClassCP classAHasBAndC = GetClass("ClassAHasBAndC")->GetRelationshipClassCP();
    ECRelationshipClassCP classA2HasB2 = GetClass("ClassA2BaseHasB2")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceA2Base = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA2Base);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceB2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classAHasBAndC, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classA2HasB2, *instanceA2Base, *instanceB2);

    /*
    A ---ClassAHasBAndC---> B
    A2Base ---ClassA2BaseHasB2---> B2
    */

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceB, instanceB2});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    EXPECT_EQ(GetDefaultDisplayLabel(*instanceA), recordJson["DisplayValues"][FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson["Values"][FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson["Values"][FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")]["ECInstanceId"].GetString());

    rapidjson::Document recordJson1 = contentSet.Get(1)->AsJson();
    EXPECT_EQ(GetDefaultDisplayLabel(*instanceA2Base), recordJson1["DisplayValues"][FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")].GetString());
    EXPECT_EQ(instanceA2Base->GetClass().GetId().ToString(), recordJson1["Values"][FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA2Base->GetInstanceId(), recordJson1["Values"][FIELD_NAME((bvector<ECClassCP>{classB, classB2}), "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstance_GetDerivedClassLabelWhenNavigationPropertyPointsToDerivedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
      <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECNavigationProperty propertyName="NavigationProperty" relationshipName="A_Has_C" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_C"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="A Has C" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="C Has A" polymorphic="False">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetDerivedClassLabelWhenNavigationPropertyPointsToDerivedClass)
    {
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relationshipAHasC = GetClass("A_Has_C")->GetRelationshipClassCP();
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasC, *instanceB, *instanceC);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    EXPECT_EQ(GetDefaultDisplayLabel(*instanceB), recordJson["DisplayValues"][FIELD_NAME(classC, "NavigationProperty")].GetString());
    EXPECT_EQ(instanceB->GetClass().GetId().ToString(), recordJson["Values"][FIELD_NAME(classC, "NavigationProperty")]["ECClassId"].GetString());
    EXPECT_EQ(instanceB->GetInstanceId(), recordJson["Values"][FIELD_NAME(classC, "NavigationProperty")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstance_GetCorrectNavigationPropertiesValuesWhenRelatedPropertiesSpecificationIsApplied, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="A Has B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B Has A" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetCorrectNavigationPropertiesValuesWhenRelatedPropertiesSpecificationIsApplied)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("InstanceA"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\"", 1, "this.Property", ""));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, relationshipAHasB->GetFullName(),
        classA->GetFullName(), "*", RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // validate content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    EXPECT_STREQ("InstanceA", recordJson["DisplayValues"][FIELD_NAME(classB, "A")].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstance_GetCorrectNavigationPropertiesValuesWhenRelatedPropertiesSpecificationIsApplied_InstanceLabelOverride, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="A Has B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B Has A" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstance_GetCorrectNavigationPropertiesValuesWhenRelatedPropertiesSpecificationIsApplied_InstanceLabelOverride)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("InstanceA"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, relationshipAHasB->GetFullName(),
        classA->GetFullName(), "*", RelationshipMeaning::RelatedInstance));
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // validate content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    EXPECT_STREQ("InstanceA", recordJson["DisplayValues"][FIELD_NAME(classB, "A")].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_GetRelatedInstanceNavigationPropertyValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="A Has B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B Has A" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_GetRelatedInstanceNavigationPropertyValue)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("InstanceA"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"A\"", 1, "this.Property", ""));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentRelatedInstancesSpecificationP spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, relationshipAHasB->GetFullName(), "");
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    EXPECT_STREQ("InstanceA", recordJson["DisplayValues"][FIELD_NAME(classB, "A")].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_GetRelatedInstanceNavigationPropertyValue_InstanceLabelOverride, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
        <ECNavigationProperty propertyName="A" relationshipName="A_Has_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B"  strength="referencing" strengthDirection="forward" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="A Has B" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(1..*)" roleLabel="B Has A" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_GetRelatedInstanceNavigationPropertyValue_InstanceLabelOverride)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECRelationshipClassCP relationshipAHasB = GetClass("A_Has_B")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Property", ECValue("InstanceA"));});
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipAHasB, *instanceA, *instanceB);

    // set up input
    KeySetPtr input = KeySet::Create(*instanceA);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));

    ContentRuleP contentRule = new ContentRule("", 1, false);
    ContentRelatedInstancesSpecificationP spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Both, relationshipAHasB->GetFullName(), "");
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // validate content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    EXPECT_STREQ("InstanceA", recordJson["DisplayValues"][FIELD_NAME(classB, "A")].GetString());
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson["Values"][FIELD_NAME(classB, "A")]["ECInstanceId"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDerivedClassNavigationPropertyWhenSelectingFromBaseClassAndDerivedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECNavigationProperty propertyName="A" relationshipName="C_A" direction="Forward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="C_A" strength="referencing" strengthDirection="backward" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="references" polymorphic="True">
            <Class class="C" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is referenced by" polymorphic="True">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDerivedClassNavigationPropertyWhenSelectingFromBaseClassAndDerivedClass)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP rel = GetClass("C_A")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instanceA, *instanceC);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SetUpDefaultLabelRule(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:B,C", GetSchema()->GetName().c_str()), false, false));
    rules->AddPresentationRule(*contentRule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // validate content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    EXPECT_TRUE(recordJson1["Values"][FIELD_NAME(classC, "A")].IsNull());
    EXPECT_TRUE(recordJson1["DisplayValues"][FIELD_NAME(classC, "A")].IsNull());

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    EXPECT_EQ(instanceA->GetClass().GetId().ToString(), recordJson2["Values"][FIELD_NAME(classC, "A")]["ECClassId"].GetString());
    EXPECT_EQ(instanceA->GetInstanceId(), recordJson2["Values"][FIELD_NAME(classC, "A")]["ECInstanceId"].GetString());
    EXPECT_STREQ(GetDefaultDisplayLabel(*instanceA).c_str(), recordJson2["DisplayValues"][FIELD_NAME(classC, "A")].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPrimitiveArrayPropertyValue, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPrimitiveArrayPropertyValue)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(3), 0);
        instance.SetValue("ArrayProperty", ECValue(4), 1);
        instance.SetValue("ArrayProperty", ECValue(5), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Array",
        "TypeName": "int[]",
        "MemberType": {
            "ValueFormat": "Primitive",
            "TypeName": "int"
            }
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME(ecClass, "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": [3, 4, 5]
        })", FIELD_NAME(ecClass, "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsPointsArrayPropertyValue, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="PointsArrayProperty" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsPointsArrayPropertyValue)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("PointsArrayProperty", 2);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(0, 0, 0)), 0);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(1, 1, 1)), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("PointsArrayProperty", 3);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(3, 3, 3)), 0);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(4, 4, 4)), 1);
        instance.SetValue("PointsArrayProperty", ECValue(DPoint3d::From(5, 5, 5)), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Array",
        "TypeName": "point3d[]",
        "MemberType": {
            "ValueFormat": "Primitive",
            "TypeName": "point3d"
            }
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": [
            {"x": 0, "y": 0, "z": 0},
            {"x": 1, "y": 1, "z": 1}
        ]})", FIELD_NAME(ecClass, "PointsArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    rapidjson::Document expectedDisplayValues1;
    expectedDisplayValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": ["X: 0.00 Y: 0.00 Z: 0.00", "X: 1.00 Y: 1.00 Z: 1.00"]
        })", FIELD_NAME(ecClass, "PointsArrayProperty")).c_str());
    EXPECT_EQ(expectedDisplayValues1, recordJson1["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["DisplayValues"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": [
            {"x": 3, "y": 3, "z": 3},
            {"x": 4, "y": 4, "z": 4},
            {"x": 5, "y": 5, "z": 5}
        ]})", FIELD_NAME(ecClass, "PointsArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    rapidjson::Document expectedDisplayValues2;
    expectedDisplayValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": ["X: 3.00 Y: 3.00 Z: 3.00", "X: 4.00 Y: 4.00 Z: 4.00", "X: 5.00 Y: 5.00 Z: 5.00"]
        })", FIELD_NAME(ecClass, "PointsArrayProperty")).c_str());
    EXPECT_EQ(expectedDisplayValues2, recordJson2["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesDescriptorsWithSimilarNavigationProperties, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="ChildElement" relationshipName="ElementContainsElements" direction="Backward"/>
    </ECEntityClass>
    <ECEntityClass typeName="DerivedElement">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesDescriptorsWithSimilarNavigationProperties)
    {
    // set up data set
    ECClassCP element = GetClass("Element");
    ECClassCP derivedElement = GetClass("DerivedElement");

    IECInstancePtr elementInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *element);
    IECInstancePtr derivedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedElement);

    // set up input
    KeySetPtr elementInput = KeySet::Create(*elementInstance);
    KeySetPtr derivedInput = KeySet::Create(*derivedInstance);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP contentRule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecification* spec = new SelectedNodeInstancesSpecification();
    contentRule->AddSpecification(*spec);
    rules->AddPresentationRule(*contentRule);

    // validate descriptors
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *elementInput)));
    ASSERT_TRUE(descriptor.IsValid());

    ContentDescriptorCPtr descriptor2 = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *derivedInput)));
    ASSERT_TRUE(descriptor2.IsValid());

    // merge descriptors
    ContentDescriptorPtr mergedDescriptor = ContentDescriptor::Create(*descriptor);
    mergedDescriptor->MergeWith(*descriptor2);
    ASSERT_TRUE(mergedDescriptor.IsValid());
    EXPECT_EQ(mergedDescriptor->GetAllFields().size(), descriptor->GetAllFields().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntMergePropertiesOfSameNameAndTypeWhenValueKindDoesntMatch, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntMergePropertiesOfSameNameAndTypeWhenValueKindDoesntMatch)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("Prop", ECValue(1));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.AddArrayElements("Prop", 2);
        instance.SetValue("Prop", ECValue(2), 0);
        instance.SetValue("Prop", ECValue(3), 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": 1,
        "%s": null
        })", FIELD_NAME(classA, "Prop"), FIELD_NAME(classB, "Prop")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": null,
        "%s": [2, 3]
        })", FIELD_NAME(classA, "Prop"), FIELD_NAME(classB, "Prop")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyFieldsOfDifferentClasses, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyFieldsOfDifferentClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(3), 0);
        instance.SetValue("ArrayProperty", ECValue(4), 1);
        instance.SetValue("ArrayProperty", ECValue(5), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": [3, 4, 5]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues, record->GetValues())
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record->GetValues());
    EXPECT_FALSE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        instance.SetValue("ArrayProperty", ECValue(3), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenSomeValuesAreNull, R"*(
    <ECEntityClass typeName="MyClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyFieldsAndRowsOfDifferentClassesWhenSomeValuesAreNull)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 1);
        instance.SetValue("ArrayProperty", ECValue(1), 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyValueWhenValuesEqual, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyValueWhenValuesEqual)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME(ecClass, "ArrayProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(record->IsMerged(FIELD_NAME(ecClass, "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyValueWhenArraySizesAreDifferent, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyValueWhenArraySizesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        instance.SetValue("ArrayProperty", ECValue(3), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyValueWhenValuesInArrayAreDifferent, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyValueWhenValuesInArrayAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(1), 0);
        instance.SetValue("ArrayProperty", ECValue(2), 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesPrimitiveArrayPropertyValueWhenClassesAreDifferent, R"*(
    <ECEntityClass typeName="ClassA">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesPrimitiveArrayPropertyValueWhenClassesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.AddArrayElements("ArrayProperty", 2);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME(classA, "ArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME(classA, "ArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(classA, "ArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsEnumsArrayPropertyValue, R"*(
    <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="True" displayLabel="My Enum">
        <ECEnumerator value="1" displayLabel="One" />
        <ECEnumerator value="2" displayLabel="Two" />
        <ECEnumerator value="3" displayLabel="Three" />
    </ECEnumeration>
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="EnumsArray" typeName="MyEnum" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsEnumsArrayPropertyValue)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("EnumsArray", 2);
        instance.SetValue("EnumsArray", ECValue(2), 0);
        instance.SetValue("EnumsArray", ECValue(1), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("EnumsArray", 3);
        instance.SetValue("EnumsArray", ECValue(1), 0);
        instance.SetValue("EnumsArray", ECValue(2), 1);
        instance.SetValue("EnumsArray", ECValue(3), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Array",
        "TypeName": "MyEnum[]",
        "MemberType": {
            "ValueFormat": "Primitive",
            "TypeName": "MyEnum"
            }
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": [2, 1]
        })", FIELD_NAME(ecClass, "EnumsArray")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);
    rapidjson::Document expectedDisplayValues1;
    expectedDisplayValues1.Parse(Utf8PrintfString(R"(
        {
        "%s": ["Two", "One"]
        })", FIELD_NAME(ecClass, "EnumsArray")).c_str());
    EXPECT_EQ(expectedDisplayValues1, recordJson1["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["DisplayValues"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": [1, 2, 3]
        })", FIELD_NAME(ecClass, "EnumsArray")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    rapidjson::Document expectedDisplayValues2;
    expectedDisplayValues2.Parse(Utf8PrintfString(R"(
        {
        "%s": ["One", "Two", "Three"]
        })", FIELD_NAME(ecClass, "EnumsArray")).c_str());
    EXPECT_EQ(expectedDisplayValues2, recordJson2["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsStructPropertyValue, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsStructPropertyValue)
    {
    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
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
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 456,
           "StringProperty": "def"
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyFieldsOfDifferentClasses, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyFieldsOfDifferentClasses)
    {
    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 456,
           "StringProperty": "def"
           }
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")).c_str());
    EXPECT_EQ(expectedValues, record->GetValues())
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record->GetValues());
    EXPECT_FALSE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyValuesWhenValuesAreEqual, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyValuesWhenValuesAreEqual)
    {
    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": 123,
           "StringProperty": "abc"
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(record->IsMerged(FIELD_NAME(ecClass, "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyValuesWhenValuesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyValuesWhenValuesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(456));
        instance.SetValue("StructProperty.StringProperty", ECValue("def"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "StructProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "StructProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructPropertyValueWhenClassesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="ClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructPropertyValueWhenClassesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    EXPECT_TRUE(record->GetValues()[FIELD_NAME(classA, "StructProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME(classA, "StructProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(classA, "StructProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsStructArrayPropertyValue, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsStructArrayPropertyValue)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
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

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
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
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

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
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })", FIELD_NAME(ecClass, "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 456,
           "StringProperty": "def"
           },{
           "IntProperty": 789,
           "StringProperty": "ghi"
           }]
        })", FIELD_NAME(ecClass, "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyFieldsOfDifferentClasses, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyFieldsOfDifferentClasses)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [structClass](IECInstanceR instance)
        {
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

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

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
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 456,
           "StringProperty": "def"
           },{
           "IntProperty": 789,
           "StringProperty": "ghi"
           }]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesEqual)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })", FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues, record->GetValues())
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record->GetValues());
    EXPECT_FALSE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyFieldsAndRowsOfDifferentClassesWhenValuesDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(456));
        structInstance->SetValue("StringProperty", ECValue("def"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(record->GetValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyValuesWhenValuesAreEqual, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyValuesWhenValuesAreEqual)
    {
    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [{
           "IntProperty": 123,
           "StringProperty": "abc"
           }]
        })", FIELD_NAME(ecClass, "StructArrayProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    EXPECT_FALSE(record->IsMerged(FIELD_NAME(ecClass, "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyValuesWhenArraySizesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyValuesWhenArraySizesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 2);

        IECInstancePtr structInstance1 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance1->SetValue("IntProperty", ECValue(123));
        structInstance1->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue1;
        structValue1.SetStruct(structInstance1.get());
        instance.SetValue("StructArrayProperty", structValue1, 0);

        IECInstancePtr structInstance2 = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance2->SetValue("IntProperty", ECValue(456));
        structInstance2->SetValue("StringProperty", ECValue("def"));
        ECValue structValue2;
        structValue2.SetStruct(structInstance2.get());
        instance.SetValue("StructArrayProperty", structValue2, 1);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "StructArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "StructArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyValuesWhenArrayValuesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyValuesWhenArrayValuesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);

        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(456));
        structInstance->SetValue("StringProperty", ECValue("def"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    rapidjson::Document recordJson = record->AsJson();
    EXPECT_TRUE(recordJson["Values"][FIELD_NAME(ecClass, "StructArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), recordJson["DisplayValues"][FIELD_NAME(ecClass, "StructArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(ecClass, "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(MergesStructArrayPropertyValueWhenClassesAreDifferent, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="ClassA">
        <ECStructArrayProperty propertyName="StructArrayProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB">
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, MergesStructArrayPropertyValueWhenClassesAreDifferent)
    {
    Utf8PrintfString varies_string(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    // set up data set
    ECClassCP structClass = GetClass("MyStruct");
    ECClassCP classA = GetClass("ClassA");
    ECClassCP classB = GetClass("ClassB");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [structClass](IECInstanceR instance)
        {
        instance.AddArrayElements("StructArrayProperty", 1);
        IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("IntProperty", ECValue(123));
        structInstance->SetValue("StringProperty", ECValue("abc"));
        ECValue structValue;
        structValue.SetStruct(structInstance.get());
        instance.SetValue("StructArrayProperty", structValue, 0);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList({classA, classB}), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentDescriptorPtr mergingDescriptor = ContentDescriptor::Create(*descriptor);
    mergingDescriptor->AddContentFlag(ContentFlags::MergeResults);

    // request for content
    ContentCPtr content = GetVerifiedContent(*mergingDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    ContentSetItemCPtr record = contentSet.Get(0);
    EXPECT_TRUE(record->GetValues()[FIELD_NAME(classA, "StructArrayProperty")].IsNull());
    EXPECT_STREQ(varies_string.c_str(), record->GetDisplayValues()[FIELD_NAME(classA, "StructArrayProperty")].GetString());
    EXPECT_TRUE(record->IsMerged(FIELD_NAME(classA, "StructArrayProperty")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsStructWithArrayPropertyValue, R"*(
    <ECStructClass typeName="MyStruct">
        <ECArrayProperty propertyName="IntProperty" typeName="int" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsStructWithArrayPropertyValue)
    {
    // set up data set
    // unused - ECClassCP structClass = GetClass("MyStruct");
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("StructProperty.IntProperty", 2);
        instance.SetValue("StructProperty.IntProperty", ECValue(1), 0);
        instance.SetValue("StructProperty.IntProperty", ECValue(2), 1);
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.AddArrayElements("StructProperty.IntProperty", 3);
        instance.SetValue("StructProperty.IntProperty", ECValue(4), 0);
        instance.SetValue("StructProperty.IntProperty", ECValue(5), 1);
        instance.SetValue("StructProperty.IntProperty", ECValue(6), 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());
    rapidjson::Document expectedFieldType;
    expectedFieldType.Parse(Utf8PrintfString(R"({
        "ValueFormat": "Struct",
        "TypeName": "MyStruct",
        "Members": [{
            "Name": "IntProperty",
            "Label": "IntProperty",
            "Type": {
                "ValueFormat": "Array",
                "TypeName": "int[]",
                "MemberType": {
                    "ValueFormat": "Primitive",
                    "TypeName": "int"
                    }
                }
            }]
        })").c_str());
    rapidjson::Document actualFieldType = descriptor->GetVisibleFields()[0]->GetTypeDescription().AsJson();
    EXPECT_EQ(expectedFieldType, actualFieldType)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedFieldType) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actualFieldType);

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    rapidjson::Document recordJson1 = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues1;
    expectedValues1.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": [1, 2]
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues1, recordJson1["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues1) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson1["Values"]);

    rapidjson::Document recordJson2 = contentSet.Get(1)->AsJson();
    rapidjson::Document expectedValues2;
    expectedValues2.Parse(Utf8PrintfString(R"({
        "%s": {
           "IntProperty": [4, 5, 6]
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues2, recordJson2["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues2) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson2["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(LoadsStructWithNullArrayAndStructPropertyValues, R"*(
    <ECStructClass typeName="MyStructA">
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECStructClass typeName="MyStructB">
        <ECArrayProperty propertyName="IntProperties" typeName="int" />
        <ECStructProperty propertyName="StructProperty" typeName="MyStructA" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStructB" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, LoadsStructWithNullArrayAndStructPropertyValues)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        instance.SetValue("StructProperty.StringProperty", ECValue("test"));
        });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    rapidjson::Document recordJson = contentSet.Get(0)->AsJson();
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": {
           "StringProperty": "test",
           "StructProperty": null,
           "IntProperties": null
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RequestingDescriptorWithClassIdsAllowsUsingSelectedNodeECExpressionSymbol, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, RequestingDescriptorWithClassIdsAllowsUsingSelectedNodeECExpressionSymbol)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classC = GetClass("C");

    // set up input
    KeySetPtr input = KeySet::Create({ECClassInstanceKey(classC, ECInstanceId())});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule(Utf8PrintfString("SelectedNode.ECInstance.IsOfClass(\"B\", \"%s\") ANDALSO SelectedNode.ClassName = \"C\"", GetSchema()->GetName().c_str()), 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(classA, &descriptor->GetSelectClasses()[0].GetSelectClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RequestingDescriptorWithClassIdsAndUsingSelectedNodeECInstanceSymbolFailsGracefully, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, RequestingDescriptorWithClassIdsAndUsingSelectedNodeECInstanceSymbolFailsGracefully)
    {
    ECClassCP classA = GetClass("A");

    // set up input
    NavNodeKeyList keys;
    KeySetPtr input = KeySet::Create({ECClassInstanceKey(classA, ECInstanceId())});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("SelectedNode.ECInstance.Property = 123", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetDifferentFieldsIfPropertiesHaveDifferentKindOfQuantities, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="LengthProperty" typeName="int" kindOfQuantity="Length" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="LengthProperty" typeName="int" kindOfQuantity="LengthSmall" />
    </ECEntityClass>
    <KindOfQuantity typeName="LengthSmall" displayLabel="Small length" persistenceUnit="MM" relativeError="0" defaultPresentationUnit="IN" presentationUnits="MM;CM"/>
    <KindOfQuantity typeName="Length" displayLabel="Length" persistenceUnit="M" relativeError="0" defaultPresentationUnit="M" presentationUnits="CM"/>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetDifferentFieldsIfPropertiesHaveDifferentKindOfQuantities)
    {
    // set up input
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    SelectedNodeInstancesSpecificationP spec = new SelectedNodeInstancesSpecification();
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    EXPECT_STREQ(FIELD_NAME(classA, "LengthProperty"), descriptor->GetVisibleFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ(FIELD_NAME(classB, "LengthProperty"), descriptor->GetVisibleFields()[1]->GetUniqueName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysForSelectedECClassGroupingNode_HierarchyIsCached, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_HierarchyIsCached)
    {
    ECClassCP classA = GetClass("A");

    // prepare dataset
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);
    ECClassInstanceKey instance2Key = RulesEngineTestHelpers::GetInstanceKey(*instance2);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", classA->GetFullName(), false));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(2, childNodes.GetSize());

    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], instance2Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysForSelectedECClassGroupingNode_HierarchyCacheIsClear, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_HierarchyCacheIsClear)
    {
    ECClassCP classA = GetClass("A");

    // prepare dataset
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);
    ECClassInstanceKey instance2Key = RulesEngineTestHelpers::GetInstanceKey(*instance2);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", classA->GetFullName(), false));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(2, childNodes.GetSize());

    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();
    // clear cache
    m_locater->InvalidateCache(rules->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*rules);

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], instance2Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysForSelectedECClassGroupingNode_SelectedNodeIsNotRootNode, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_SelectedNodeIsNotRootNode)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");

    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);
    ECClassInstanceKey instance2Key = RulesEngineTestHelpers::GetInstanceKey(*instance2);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rules
    // Class B rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classB->GetFullName(), false));
    // Class C rule
    ChildNodeRuleP childRule = new ChildNodeRule("ParentNode.ClassName = \"B\"", 1, false, RuleTargetTree::TargetTree_MainTree);
    rules->AddPresentationRule(*childRule);
    InstanceNodesOfSpecificClassesSpecification* childRuleSpec = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, false, "", classC->GetFullName(), false);
    childRule->AddSpecification(*childRuleSpec);
    // Class A rule
    ChildNodeRuleP nestedChildRule = new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree);
    nestedChildRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", classA->GetFullName(), false));
    childRuleSpec->AddNestedRule(*nestedChildRule);

    /* Create the following hierarchy:
            + B
            +--+ C
            |  +--+ A ECClassGroupingNode   <- selected node
            |     +--- instance1
            |     +--- instance2
    */

    // cache hierarchy
    // get B node
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(1, rootNodes.GetSize());

    // get C node
    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(1, childNodes.GetSize());

    // get A class grouping node
    childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), childNodes[0].get())));
    ASSERT_EQ(1, childNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, childNodes[0]->GetType().c_str());

    // save A class grouping node key
    NavNodeKeyCPtr selectedNode = childNodes[0]->GetKey();
    // clear cache
    m_locater->InvalidateCache(rules->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*rules);

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], instance2Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysForSelectedECClassGroupingNode_NewInstanceIsInsertedAndHierarchyCacheIsClear, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_NewInstanceIsInsertedAndHierarchyCacheIsClear)
    {
    ECClassCP classA = GetClass("A");

    // prepare dataset
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", classA->GetFullName(), false));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(1, childNodes.GetSize());

    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();
    // clear cache
    m_locater->InvalidateCache(rules->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*rules);

    // insert another instance
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    ECClassInstanceKey instance2Key = RulesEngineTestHelpers::GetInstanceKey(*instance2);

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], instance2Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysForSelectedECClassGroupingNode_OneInstanceIsDeletedAndHierarchyCacheIsClear, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECClassGroupingNode_OneInstanceIsDeletedAndHierarchyCacheIsClear)
    {
    ECClassCP classA = GetClass("A");

    // prepare dataset
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, false, false, "", classA->GetFullName(), false));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(2, childNodes.GetSize());

    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();
    // clear cache
    m_locater->InvalidateCache(rules->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*rules);

    // delete one instance
    RulesEngineTestHelpers::DeleteInstance(s_project->GetECDb(), *instance2);

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysForSelectedDisplayLabelGroupingNode_HierarchyIsCached, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedDisplayLabelGroupingNode_HierarchyIsCached)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});

    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);
    ECClassInstanceKey instance2Key = RulesEngineTestHelpers::GetInstanceKey(*instance2);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, rootNodes[0]->GetType().c_str()); // A display grouping node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str()); // B instance

    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], instance2Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysForSelectedDisplayLabelGroupingNode_HierarchyCacheIsEmpty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedDisplayLabelGroupingNode_HierarchyCacheIsEmpty)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});

    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);
    ECClassInstanceKey instance2Key = RulesEngineTestHelpers::GetInstanceKey(*instance2);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, rootNodes[0]->GetType().c_str()); // A display grouping node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str()); // B instance

    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    // clear caches
    m_locater->InvalidateCache(rules->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*rules);

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], instance2Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysWhenDisplayLabelGroupingNodeIsCreated, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysWhenDisplayLabelGroupingNodeIsCreated)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instanceA1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});

    ECClassInstanceKey instanceA1Key = RulesEngineTestHelpers::GetInstanceKey(*instanceA1);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[0]->GetType().c_str()); // A node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str()); // B instance

    // save A node key
    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    // clear caches
    m_locater->InvalidateCache(rules->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*rules);

    // insert another A DisplayGroupingNode is created
    IECInstancePtr instanceA2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instanceA1Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysWhenDisplayLabelGroupingNodeIsRemoved, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysWhenDisplayLabelGroupingNodeIsRemoved)
    {
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    // prepare dataset
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue("A"));});

    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Property"));
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, true, false, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(2, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, rootNodes[0]->GetType().c_str()); // A display grouping node
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, rootNodes[1]->GetType().c_str()); // B instance

    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    // clear caches
    m_locater->InvalidateCache(rules->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*rules);

    // delete one A instance
    RulesEngineTestHelpers::DeleteInstance(s_project->GetECDb(), *instance2);

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysForSelectedECPropertyGroupingNode_HierarchyIsCached, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECPropertyGroupingNode_HierarchyIsCached)
    {
    ECClassCP classA = GetClass("A");

    // prepare dataset
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});

    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);
    ECClassInstanceKey instance2Key = RulesEngineTestHelpers::GetInstanceKey(*instance2);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false));
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    rules->AddPresentationRule(*groupingRule);
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "Property"));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], instance2Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetKeysForSelectedECPropertyGroupingNode_HierarchyCacheIsEmpty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Property" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, GetKeysForSelectedECPropertyGroupingNode_HierarchyCacheIsEmpty)
    {
    ECClassCP classA = GetClass("A");

    // prepare dataset
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Property", ECValue(5));});

    ECClassInstanceKey instance1Key = RulesEngineTestHelpers::GetInstanceKey(*instance1);
    ECClassInstanceKey instance2Key = RulesEngineTestHelpers::GetInstanceKey(*instance2);

    // create a ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    // set up content rule
    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));

    // set up navigation rule
    RootNodeRuleP rootRule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    rules->AddPresentationRule(*rootRule);
    rootRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, true, false, false, false, false, false, "", Utf8PrintfString("%s:A,B", GetSchema()->GetName().c_str()), false));
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, GetSchema()->GetName(), "A", "", "", "");
    rules->AddPresentationRule(*groupingRule);
    groupingRule->AddGroup(*new PropertyGroup("", "", false, "Property"));

    // cache hierarchy
    NavNodesContainer rootNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables())));
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, rootNodes[0]->GetType().c_str());

    NavNodesContainer childNodes = GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), rootNodes[0].get())));
    ASSERT_EQ(2, childNodes.GetSize());

    // save display label grouping node key
    NavNodeKeyCPtr selectedNode = rootNodes[0]->GetKey();

    // clear caches
    m_locater->InvalidateCache(rules->GetRuleSetId().c_str());
    m_locater->AddRuleSet(*rules);

    // validate content descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*selectedNode))));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    ASSERT_EQ(1, contentSet[0]->GetKeys().size());
    EXPECT_EQ(contentSet[0]->GetKeys()[0], instance1Key);
    ASSERT_EQ(1, contentSet[1]->GetKeys().size());
    EXPECT_EQ(contentSet[1]->GetKeys()[0], instance2Key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntLoadCompositeContentIfInstanceDoesntHaveCompositeProperty, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClassA">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClassB">
        <ECProperty propertyName="BIntProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntLoadCompositeContentIfInstanceDoesntHaveCompositeProperty)
    {
    // set up data set
    ECClassCP classA = GetClass("MyClassA");
    ECClassCP classB = GetClass("MyClassB");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{instanceA, instanceB});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntTakeContentTwiceForTheSameInstance, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="StructProperty" typeName="int" />
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward" description="The parent element that owns this element.">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesntTakeContentTwiceForTheSameInstance)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECRelationshipClassCP elementOwnsChildElement = GetClass("ElementOwnsChildElements")->GetRelationshipClassCP();
    IECInstancePtr rootElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr childElement = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementOwnsChildElement, *rootElement, *childElement);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, true, "", RequiredRelationDirection::RequiredRelationDirection_Forward,
        elementOwnsChildElement->GetFullName(), elementClass->GetFullName()));
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{rootElement, childElement});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(2, descriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // expect 2 content set items
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classCUsesClassA, *instanceB, *instanceC);

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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_TRUE(modifiedDescriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_TRUE(modifiedDescriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_TRUE(modifiedDescriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_TRUE(modifiedDescriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_TRUE(modifiedDescriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("X: 1.00 Y: 2.00 Z: 3.00", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_TRUE(modifiedDescriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("X: 1.00 Y: 2.00 Z: 3.00", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_TRUE(modifiedDescriptor->GetDisplayLabelField() != nullptr);

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ("X: 1.00 Y: 2.00", contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "BaseStringProperty"));
    rules->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "ClassC_String"));
    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(2, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(2, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classCUsesClassA, *instanceB, *instanceC);

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
    ECRelationshipClassCP classCUsesClassA = GetClass("ClassDUsesClassA")->GetRelationshipClassCP();
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classCUsesClassA, *instanceC, *instanceD);

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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    // Expecting "Not specified" label
    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // set the "show labels" flag
    ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
    modifiedDescriptor->AddContentFlag(ContentFlags::ShowLabels);
    EXPECT_EQ(1, modifiedDescriptor->GetVisibleFields().size());

    // request for content
    ContentCPtr content = GetVerifiedContent(*modifiedDescriptor);
    ASSERT_TRUE(content.IsValid());

    // validate content set
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(1, contentSet.GetSize());

    EXPECT_STREQ(CommonStrings::RULESENGINE_NOTSPECIFIED, contentSet.Get(0)->GetDisplayLabelDefinition().GetDisplayValue().c_str());
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
    EXPECT_STREQ(CommonStrings::RULESENGINE_MULTIPLEINSTANCES, label->GetDisplayValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AppliesContentItemExtendedDataFromPresentationRules, R"*(
    <ECEntityClass typeName="MyClass1">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="CodeValue" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="MyClass2">
        <BaseClass>MyClass1</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, AppliesContentItemExtendedDataFromPresentationRules)
    {
    // set up data set
    ECClassCP ecClass1 = GetClass("MyClass1");
    ECClassCP ecClass2 = GetClass("MyClass2");
    IECInstancePtr element1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass1, [](IECInstanceR instance) {instance.SetValue("CodeValue", ECValue("test value")); });
    IECInstancePtr element2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass2);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass1->GetFullName(), true, false));

    ExtendedDataRule* ex1 = new ExtendedDataRule();
    ex1->AddItem("class_name", "ThisNode.ClassName");
    ex1->AddItem("property_value", "this.CodeValue");
    ex1->AddItem("is_MyClass", Utf8PrintfString("ThisNode.IsOfClass(\"%s\", \"%s\")", ecClass1->GetName().c_str(), ecClass1->GetSchema().GetName().c_str()));
    rules->AddPresentationRule(*ex1);

    ExtendedDataRule* ex2 = new ExtendedDataRule(Utf8PrintfString("ThisNode.IsOfClass(\"%s\", \"%s\")", ecClass2->GetName().c_str(), ecClass2->GetSchema().GetName().c_str()));
    ex2->AddItem("constant_bool", "true");
    ex2->AddItem("constant_int", "1");
    ex2->AddItem("constant_double", "1.23");
    ex2->AddItem("constant_string", "\"Red\"");
    rules->AddPresentationRule(*ex2);

    // get content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ContentCPtr content = GetVerifiedContent(*descriptor);

    // assert
    ASSERT_TRUE(content.IsValid());
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    ASSERT_EQ(2, contentSet.GetSize());

    RapidJsonAccessor extendedData1 = contentSet[0]->GetUsersExtendedData();
    ASSERT_TRUE(extendedData1.GetJson().IsObject());
    ASSERT_EQ(3, extendedData1.GetJson().MemberCount());

    ASSERT_TRUE(extendedData1.GetJson().HasMember("class_name"));
    EXPECT_STREQ(ecClass1->GetName().c_str(), extendedData1.GetJson()["class_name"].GetString());

    ASSERT_TRUE(extendedData1.GetJson().HasMember("property_value"));
    EXPECT_STREQ("test value", extendedData1.GetJson()["property_value"].GetString());

    ASSERT_TRUE(extendedData1.GetJson().HasMember("is_MyClass"));
    EXPECT_TRUE(extendedData1.GetJson()["is_MyClass"].GetBool());

    RapidJsonAccessor extendedData2 = contentSet[1]->GetUsersExtendedData();
    ASSERT_TRUE(extendedData2.GetJson().IsObject());
    ASSERT_EQ(7, extendedData2.GetJson().MemberCount());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("class_name"));
    EXPECT_STREQ(ecClass2->GetName().c_str(), extendedData2.GetJson()["class_name"].GetString());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("property_value"));
    EXPECT_TRUE(extendedData2.GetJson()["property_value"].IsNull());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("is_MyClass"));
    EXPECT_TRUE(extendedData2.GetJson()["is_MyClass"].GetBool());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("constant_bool"));
    EXPECT_TRUE(extendedData2.GetJson()["constant_bool"].GetBool());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("constant_int"));
    EXPECT_EQ(1, extendedData2.GetJson()["constant_int"].GetInt());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("constant_double"));
    EXPECT_DOUBLE_EQ(1.23, extendedData2.GetJson()["constant_double"].GetDouble());

    ASSERT_TRUE(extendedData2.GetJson().HasMember("constant_string"));
    EXPECT_STREQ("Red", extendedData2.GetJson()["constant_string"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotIncludeHiddenProperties,
    R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="RegularProperty" typeName="string" />
        <ECProperty propertyName="HiddenProperty" typeName="string">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00" />
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesNotIncludeHiddenProperties)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true, false));

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields[0]->IsPropertiesField());
    ASSERT_EQ(1, fields[0]->AsPropertiesField()->GetProperties().size());
    EXPECT_EQ(elementClass->GetPropertyP("RegularProperty"), &fields[0]->AsPropertiesField()->GetProperties()[0].GetProperty());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#224831
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentForBaseClassWhenDerivedClassIsCustomizedAndThereAreLotsOfDerivedClasses)
    {
    RulesEngineTestHelpers::ImportSchema(s_project->GetECDb(), [&](ECSchemaR schema)
        {
        ECEntityClassP baseClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(baseClass, "BaseClass"));
        ASSERT_TRUE(nullptr != baseClass);
        IECInstancePtr classMapCustomAttribute = GetClass("ECDbMap", "ClassMap")->GetDefaultStandaloneEnabler()->CreateInstance();
        classMapCustomAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy"));
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*classMapCustomAttribute));

        PrimitiveECPropertyP prop = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(prop, "Label", PRIMITIVETYPE_String));

        RulesEngineTestHelpers::CreateNDerivedClasses(schema, *baseClass, 1000);
        });

    ECEntityClassCP baseClass = GetClass("BaseClass")->GetEntityClassCP();
    bvector<ECEntityClassCP> derivedClasses;
    for (int i = 0; i < 1000; ++i)
        derivedClasses.push_back(GetClass(Utf8PrintfString("Class%d", i + 1).c_str())->GetEntityClassCP());

    // set up data set
    for (ECEntityClassCP ecClass : derivedClasses)
        RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, derivedClasses[0]->GetFullName(), "Label"));

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    ASSERT_EQ(2, descriptor->GetSelectClasses().size());
    EXPECT_EQ(baseClass->GetId(), descriptor->GetSelectClasses()[0].GetSelectClass().GetClass().GetId());
    EXPECT_EQ(derivedClasses[0]->GetId(), descriptor->GetSelectClasses()[1].GetSelectClass().GetClass().GetId());

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(1000, content->GetContentSet().GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#515142
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentInstancesOfSpecificClassesWithPolymorphicInstancesAndPropertiesForBaseClassWhenThereAreLotsOfDerivedClasses)
    {
    // set up the schema
    RulesEngineTestHelpers::ImportSchema(s_project->GetECDb(), [&](ECSchemaR schema)
        {
        ECEntityClassP baseClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(baseClass, "BaseClass"));
        ASSERT_TRUE(nullptr != baseClass);
        IECInstancePtr classMapCustomAttribute = GetClass("ECDbMap", "ClassMap")->GetDefaultStandaloneEnabler()->CreateInstance();
        classMapCustomAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy"));
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*classMapCustomAttribute));

        PrimitiveECPropertyP prop = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(prop, "Label", PRIMITIVETYPE_String));

        RulesEngineTestHelpers::CreateNDerivedClasses(schema, *baseClass, 1000);
        });

    ECEntityClassCP baseClass = GetClass("BaseClass")->GetEntityClassCP();
    bvector<ECEntityClassCP> derivedClasses;
    for (int i = 0; i < 1000; ++i)
        derivedClasses.push_back(GetClass(Utf8PrintfString("Class%d", i + 1).c_str())->GetEntityClassCP());

    // set up data set
    for (ECEntityClassCP ecClass : derivedClasses)
        RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", baseClass->GetFullName(), true, true));
    rules->AddPresentationRule(*rule);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    ASSERT_EQ(1000, descriptor->GetSelectClasses().size());
    for (size_t i = 0; i < 1000; ++i)
        {
        EXPECT_TRUE(ContainerHelpers::Contains(derivedClasses, [&](ECEntityClassCP derivedClass)
            {
            return derivedClass->GetId() == descriptor->GetSelectClasses()[i].GetSelectClass().GetClass().GetId();
            }));
        }

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(1000, content->GetContentSet().GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#257477
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentForRelatedBaseClassWhenThereAreLotsOfDerivedClasses)
    {
    // set up the schema
    RulesEngineTestHelpers::ImportSchema(s_project->GetECDb(), [&](ECSchemaR schema)
        {
        ECEntityClassP inputClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(inputClass, "InputClass"));
        ASSERT_TRUE(nullptr != inputClass);

        ECEntityClassP baseClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(baseClass, "BaseClass"));
        ASSERT_TRUE(nullptr != baseClass);
        IECInstancePtr classMapCustomAttribute = GetClass("ECDbMap", "ClassMap")->GetDefaultStandaloneEnabler()->CreateInstance();
        classMapCustomAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy"));
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*classMapCustomAttribute));

        PrimitiveECPropertyP prop = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(prop, "Label", PRIMITIVETYPE_String));

        ECRelationshipClassP relationshipClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateRelationshipClass(relationshipClass, "Relationship", *inputClass, "Source", *baseClass, "Target"));

        RulesEngineTestHelpers::CreateNDerivedClasses(schema, *baseClass, 1000);
        });

    ECEntityClassCP inputClass = GetClass("InputClass")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClass = GetClass("Relationship")->GetRelationshipClassCP();
    bvector<ECEntityClassCP> derivedClasses;
    for (int i = 0; i < 1000; ++i)
        derivedClasses.push_back(GetClass(Utf8PrintfString("Class%d", i + 1).c_str())->GetEntityClassCP());

    // insert just one of the derived class instances
    IECInstancePtr inputInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *inputClass);
    IECInstancePtr relatedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClasses[0]);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *inputInstance, *relatedInstance);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, relationshipClass->GetFullName(), ""));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new InstanceLabelOverride(1, false, derivedClasses[0]->GetFullName(), "Label"));

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *KeySet::Create(*inputInstance))));

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(derivedClasses[0]->GetId(), descriptor->GetSelectClasses()[0].GetSelectClass().GetClass().GetId());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*inputInstance, *relatedInstance) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#257477
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentForRelatedBaseClassWhenThereAreLotsOfDerivedClassesAndFiltering)
    {
    // set up the schema
    RulesEngineTestHelpers::ImportSchema(s_project->GetECDb(), [&](ECSchemaR schema)
        {
        ECEntityClassP inputClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(inputClass, "InputClass"));
        ASSERT_TRUE(nullptr != inputClass);

        ECEntityClassP baseClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(baseClass, "BaseClass"));
        ASSERT_TRUE(nullptr != baseClass);
        IECInstancePtr classMapCustomAttribute = GetClass("ECDbMap", "ClassMap")->GetDefaultStandaloneEnabler()->CreateInstance();
        classMapCustomAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy"));
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*classMapCustomAttribute));

        PrimitiveECPropertyP prop = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(prop, "Label", PRIMITIVETYPE_String));

        ECRelationshipClassP relationshipClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateRelationshipClass(relationshipClass, "Relationship", *inputClass, "Source", *baseClass, "Target"));

        RulesEngineTestHelpers::CreateNDerivedClasses(schema, *baseClass, 1000);
        });

    ECEntityClassCP inputClass = GetClass("InputClass")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClass = GetClass("Relationship")->GetRelationshipClassCP();
    bvector<ECEntityClassCP> derivedClasses;
    for (int i = 0; i < 1000; ++i)
        derivedClasses.push_back(GetClass(Utf8PrintfString("Class%d", i + 1).c_str())->GetEntityClassCP());

    // insert just one of the derived class instances
    IECInstancePtr inputInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *inputClass);
    IECInstancePtr relatedInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClasses[0]);
    IECInstancePtr relatedInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClasses[1], [](IECInstanceR instance){instance.SetValue("Label", ECValue("123"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *inputInstance, *relatedInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClass, *inputInstance, *relatedInstance2);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, 0, false, "this.Label = \"123\"", RequiredRelationDirection_Forward, relationshipClass->GetFullName(), ""));
    rules->AddPresentationRule(*rule);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *KeySet::Create(*inputInstance))));

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(derivedClasses[1]->GetId(), descriptor->GetSelectClasses()[0].GetSelectClass().GetClass().GetId());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*inputInstance,  *relatedInstance2) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#257477
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerContentTests, CreatesContentForRelatedBaseClassWhenThereAreLotsOfDerivedClassesAndRelatedInstances)
    {
    // set up the schema
    RulesEngineTestHelpers::ImportSchema(s_project->GetECDb(), [&](ECSchemaR schema)
        {
        ECEntityClassP inputClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(inputClass, "InputClass"));
        ASSERT_TRUE(nullptr != inputClass);

        ECEntityClassP baseClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(baseClass, "BaseClass"));
        ASSERT_TRUE(nullptr != baseClass);
        IECInstancePtr classMapCustomAttribute = GetClass("ECDbMap", "ClassMap")->GetDefaultStandaloneEnabler()->CreateInstance();
        classMapCustomAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy"));
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*classMapCustomAttribute));

        PrimitiveECPropertyP prop = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(prop, "Label", PRIMITIVETYPE_String));

        RulesEngineTestHelpers::CreateNDerivedClasses(schema, *baseClass, 1000);

        ECEntityClassP relatedClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(relatedClass, "RelatedClass"));

        ECRelationshipClassP relationshipClassInputToBase = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateRelationshipClass(relationshipClassInputToBase, "Input_To_Base", *inputClass, "Source", *baseClass, "Target"));

        ECRelationshipClassP relationshipClassBaseToRelated = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateRelationshipClass(relationshipClassBaseToRelated, "Base_To_Related", *baseClass, "Source", *relatedClass, "Target"));
        });

    ECEntityClassCP inputClass = GetClass("InputClass")->GetEntityClassCP();
    ECEntityClassCP relatedClass = GetClass("RelatedClass")->GetEntityClassCP();
    ECRelationshipClassCP relationshipClassInputToBase = GetClass("Input_To_Base")->GetRelationshipClassCP();
    ECRelationshipClassCP relationshipClassBaseToRelated = GetClass("Base_To_Related")->GetRelationshipClassCP();
    bvector<ECEntityClassCP> derivedClasses;
    for (int i = 0; i < 1000; ++i)
        derivedClasses.push_back(GetClass(Utf8PrintfString("Class%d", i + 1).c_str())->GetEntityClassCP());

    // insert just one of the derived class instances
    IECInstancePtr inputInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *inputClass);
    IECInstancePtr relatedDerivedInstance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClasses[0]);
    IECInstancePtr relatedDerivedInstance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *derivedClasses[1]);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassInputToBase, *inputInstance, *relatedDerivedInstance1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassInputToBase, *inputInstance, *relatedDerivedInstance2);
    IECInstancePtr relatedInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *relatedClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relationshipClassBaseToRelated, *relatedDerivedInstance2, *relatedInstance);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    auto spec = new ContentRelatedInstancesSpecification(1, 0, false, "", RequiredRelationDirection_Forward, relationshipClassInputToBase->GetFullName(), "");
    spec->AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification({new RelationshipStepSpecification(relationshipClassBaseToRelated->GetFullName(), RequiredRelationDirection_Forward)}), "test", true));
    rule->AddSpecification(*spec);
    rules->AddPresentationRule(*rule);

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::IncludeInputKeys, *KeySet::Create(*inputInstance))));

    ASSERT_EQ(1, descriptor->GetSelectClasses().size());
    EXPECT_EQ(derivedClasses[1]->GetId(), descriptor->GetSelectClasses()[0].GetSelectClass().GetClass().GetId());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*inputInstance, *relatedDerivedInstance2) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsCorrectContentSetSizeForRecursivelyRelatedContentWhenCustomizingDerivedClasses, R"*(
    <ECEntityClass typeName="X" />
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
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="X_To_A" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="X"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsCorrectContentSetSizeForRecursivelyRelatedContentWhenCustomizingDerivedClasses)
    {
    // set up the dataset
    ECClassCP classX = GetClass("X");
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP rel = GetClass("X_To_A")->GetRelationshipClassCP();

    IECInstancePtr x = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classX);
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *x, *a);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *x, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *x, *c);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rule->AddSpecification(*new ContentRelatedInstancesSpecification(1, "", {new RepeatableRelationshipPathSpecification(
        {
        new RepeatableRelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, "", 0)
        })}));
    rules->AddPresentationRule(*rule);
    rules->AddPresentationRule(*new ContentModifier(classB->GetSchema().GetName(), classB->GetName()));
    rules->AddPresentationRule(*new ContentModifier(classC->GetSchema().GetName(), classC->GetName()));

    // request content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create(*x))));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(3, GetValidatedResponse(m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor))));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsCorrectContentWhenUsingRulesetVariables, R"*(
    <ECEntityClass typeName="X" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ReturnsCorrectContentWhenUsingRulesetVariables)
    {
    // set up the dataset
    ECClassCP classX = GetClass("X");
    IECInstancePtr x = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classX);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("HasVariable(\"test\")", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classX->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);

    // request content
    RulesetVariables variables({ RulesetVariableEntry("test", true) });
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), variables, nullptr, 0, *KeySet::Create(*x))));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    RulesEngineTestHelpers::ValidateContentSetItem(*x, *content->GetContentSet()[0], *descriptor);
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests : RulesDrivenECPresentationManagerContentTests
    {
    StubPropertyFormatter m_propertyFormatter;
    RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests()
        : RulesDrivenECPresentationManagerContentTests(), m_propertyFormatter(true)
        {}
    virtual void _ConfigureManagerParams(ECPresentationManager::Params& params) override
        {
        RulesDrivenECPresentationManagerContentTests::_ConfigureManagerParams(params);
        params.SetECPropertyFormatter(&m_propertyFormatter);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesSuppliedECPropertyFormatterToFormatPrimitiveECPropertyValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="StringProperty" typeName="string" />
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="BoolProperty" typeName="bool" />
        <ECProperty propertyName="DoubleProperty" typeName="double" />
        <ECProperty propertyName="LongProperty" typeName="long" />
        <ECProperty propertyName="DateProperty" typeName="dateTime" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, UsesSuppliedECPropertyFormatterToFormatPrimitiveECPropertyValue)
    {
    ECClassCP classA = GetClass("A");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance)
        {
        instance.SetValue("StringProperty", ECValue("Test"));
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("BoolProperty", ECValue(true));
        instance.SetValue("DoubleProperty", ECValue(4.0));
        instance.SetValue("LongProperty", ECValue((int64_t)123));
        instance.SetValue("DateProperty", ECValue(DateTime(2017, 5, 30)));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification *specInstance = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*specInstance);

    // params with unit system
    auto descriptorParams = AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create());
    descriptorParams.SetUnitSystem(ECPresentation::UnitSystem::BritishImperial);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(descriptorParams));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(6, descriptor->GetVisibleFields().size());

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
    RapidJsonValueCR displayValues = recordJson["DisplayValues"];
    ASSERT_TRUE(displayValues.IsObject());

    Utf8String fieldName = FIELD_NAME(classA, "StringProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_Test_[British Imperial]", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(classA, "IntProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_3_[British Imperial]", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(classA, "BoolProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_True_[British Imperial]", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(classA, "DoubleProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_4_[British Imperial]", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(classA, "LongProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_123_[British Imperial]", displayValues[fieldName.c_str()].GetString());

    fieldName = FIELD_NAME(classA, "DateProperty");
    ASSERT_TRUE(displayValues.HasMember(fieldName.c_str()));
    ASSERT_TRUE(displayValues[fieldName.c_str()].IsString());
    EXPECT_STREQ("_2017-05-30T00:00:00.000Z_[British Imperial]", displayValues[fieldName.c_str()].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesSuppliedECPropertyFormatterToFormatNestedContentValue, R"*(
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect">
        <ECProperty propertyName="StringProperty" typeName="string" />
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
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, UsesSuppliedECPropertyFormatterToFormatNestedContentValue)
    {
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("ElementUniqueAspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        instance.SetValue("StringProperty", ECValue("Test"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        rel->GetFullName(), aspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance));

    // params with unit system
    auto descriptorParams = AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create());
    descriptorParams.SetUnitSystem(ECPresentation::UnitSystem::Metric);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(descriptorParams));
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(1, descriptor->GetVisibleFields().size());

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
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": "Test"
                },
            "DisplayValues": {
                "%s": "_Test_[Metric]"
                },
            "MergedFieldNames": []
            }]
        })", NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "StringProperty"), FIELD_NAME(aspectClass, "StringProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(UsesSuppliedECPropertyFormatterToFormatPropertyLabels, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="StringProperty" typeName="string" />
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="BoolProperty" typeName="bool" />
        <ECProperty propertyName="DoubleProperty" typeName="double" />
        <ECProperty propertyName="LongProperty" typeName="long" />
        <ECProperty propertyName="DateProperty" typeName="dateTime" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, UsesSuppliedECPropertyFormatterToFormatPropertyLabels)
    {
    ECClassCP classA = GetClass("A");

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification *specInstance = new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), false, false);
    rule->AddSpecification(*specInstance);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(6, descriptor->GetVisibleFields().size());

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    EXPECT_STREQ("_StringProperty_", fields[0]->GetLabel().c_str());
    EXPECT_STREQ("_IntProperty_", fields[1]->GetLabel().c_str());
    EXPECT_STREQ("_BoolProperty_", fields[2]->GetLabel().c_str());
    EXPECT_STREQ("_DoubleProperty_", fields[3]->GetLabel().c_str());
    EXPECT_STREQ("_LongProperty_", fields[4]->GetLabel().c_str());
    EXPECT_STREQ("_DateProperty_", fields[5]->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FormatsPrimitiveArrayPropertyValues, R"*(
    <ECEntityClass typeName="MyClass">
        <ECArrayProperty propertyName="ArrayProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, FormatsPrimitiveArrayPropertyValues)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        ECValue nullECValue; nullECValue.SetIsNull(true);
        instance.AddArrayElements("ArrayProperty", 3);
        instance.SetValue("ArrayProperty", ECValue(2), 0);
        instance.SetValue("ArrayProperty", ECValue(1), 1);
        instance.SetValue("ArrayProperty", nullECValue, 2);
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // params with unit system
    auto descriptorParams = AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create());
    descriptorParams.SetUnitSystem(ECPresentation::UnitSystem::UsCustomary);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(descriptorParams));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

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
    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"(
        {
        "%s": ["_2_[US Customary]", "_1_[US Customary]", null]
        })", FIELD_NAME(ecClass, "ArrayProperty")).c_str());
    EXPECT_EQ(expectedDisplayValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FormatsStructPropertyValues, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="DoubleProperty" typeName="double" />
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="MyClass">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, FormatsStructPropertyValues)
    {
    // set up data set
    ECClassCP ecClass = GetClass("MyClass");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass, [](IECInstanceR instance)
        {
        ECValue nullECValue; nullECValue.SetIsNull(true);
        instance.SetValue("StructProperty.DoubleProperty", nullECValue);
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    // params with unit system
    auto descriptorParams = AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create());
    descriptorParams.SetUnitSystem(ECPresentation::UnitSystem::UsSurvey);

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(descriptorParams));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

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
    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"({
        "%s": {
           "DoubleProperty": null,
           "IntProperty": "_123_[US Survey]",
           "StringProperty": "_abc_[US Survey]"
           }
        })", FIELD_NAME(ecClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedDisplayValues, recordJson["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FormatsNestedStructPropertyValues, R"*(
    <ECStructClass typeName="MyStruct">
        <ECProperty propertyName="DoubleProperty" typeName="double" />
        <ECProperty propertyName="IntProperty" typeName="int" />
        <ECProperty propertyName="StringProperty" typeName="string" />
    </ECStructClass>
    <ECEntityClass typeName="Element">
    </ECEntityClass>
    <ECEntityClass typeName="ElementUniqueAspect">
        <ECStructProperty propertyName="StructProperty" typeName="MyStruct" />
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
TEST_F(RulesDrivenECPresentationManagerContentWithCustomPropertyFormatterTests, FormatsNestedStructPropertyValues)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    ECClassCP aspectClass = GetClass("ElementUniqueAspect");
    ECRelationshipClassCP rel = GetClass("ElementOwnsUniqueAspect")->GetRelationshipClassCP();

    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass, [](IECInstanceR instance)
        {
        ECValue nullECValue; nullECValue.SetIsNull(true);
        instance.SetValue("StructProperty.DoubleProperty", nullECValue);
        instance.SetValue("StructProperty.IntProperty", ECValue(123));
        instance.SetValue("StructProperty.StringProperty", ECValue("abc"));
        });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *element, *aspect);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rules->AddPresentationRule(*rule);

    ContentInstancesOfSpecificClassesSpecification* spec = new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), false, false);
    rule->AddSpecification(*spec);

    spec->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        rel->GetFullName(), aspectClass->GetFullName(), "*", RelationshipMeaning::SameInstance));

    // validate descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());
    ASSERT_EQ(1, descriptor->GetVisibleFields().size());

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
        "%s": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "%s": {
                    "DoubleProperty": null,
                    "IntProperty": 123,
                    "StringProperty": "abc"
                    }
                },
            "DisplayValues": {
                "%s": {
                    "DoubleProperty": null,
                    "IntProperty": "_123_[Default]",
                    "StringProperty": "_abc_[Default]"
                    }
                },
            "MergedFieldNames": []
            }]
        })", NESTED_CONTENT_FIELD_NAME(elementClass, aspectClass),
        aspectClass->GetId().ToString().c_str(), aspect->GetInstanceId().c_str(),
        FIELD_NAME(aspectClass, "StructProperty"), FIELD_NAME(aspectClass, "StructProperty")).c_str());
    EXPECT_EQ(expectedValues, recordJson["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(recordJson["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(IncludesAllPropertiesWhenHiddenPropertyDisplayIsOverridenInContentRule,
    R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="RegularProperty" typeName="string" />
        <ECProperty propertyName="HiddenProperty" typeName="string">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00" />
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, IncludesAllPropertiesWhenHiddenPropertyDisplayIsOverridenInContentRule)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule();
    rules->AddPresentationRule(*rule);

    ContentSpecification* specification = new ContentInstancesOfSpecificClassesSpecification(1001, "", elementClass->GetFullName(), true, false);
    rule->AddSpecification(*specification);
    specification->AddPropertyOverride(*(new PropertySpecification("HiddenProperty", 2000, "", nullptr, true, nullptr, nullptr, true)));

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(2, fields.size());
    ASSERT_TRUE(fields[0]->IsPropertiesField());
    ASSERT_TRUE(fields[1]->IsPropertiesField());
    ASSERT_EQ(1, fields[0]->AsPropertiesField()->GetProperties().size());
    ASSERT_EQ(1, fields[1]->AsPropertiesField()->GetProperties().size());
    EXPECT_EQ(elementClass->GetPropertyP("RegularProperty"), &fields[0]->AsPropertiesField()->GetProperties()[0].GetProperty());
    EXPECT_EQ(elementClass->GetPropertyP("HiddenProperty"), &fields[1]->AsPropertiesField()->GetProperties()[0].GetProperty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(IncludesAllPropertiesWhenHiddenPropertyDisplayIsOverridenInContentModifier,
    R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="RegularProperty" typeName="string" />
        <ECProperty propertyName="HiddenProperty" typeName="string">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00" />
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, IncludesAllPropertiesWhenHiddenPropertyDisplayIsOverridenInContentModifier)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* contentRule = new ContentRule();
    rules->AddPresentationRule(*contentRule);
    ContentSpecification* specification = new ContentInstancesOfSpecificClassesSpecification(1001, "", elementClass->GetFullName(), true, false);
    contentRule->AddSpecification(*specification);

    ContentModifier* modifier = new ContentModifier(elementClass->GetSchema().GetName(), "Element");
    rules->AddPresentationRule(*modifier);
    modifier->AddPropertyOverride(*(new PropertySpecification("HiddenProperty", 2000, "", nullptr, true, nullptr, nullptr, true)));

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(2, fields.size());
    ASSERT_TRUE(fields[0]->IsPropertiesField());
    ASSERT_TRUE(fields[1]->IsPropertiesField());
    ASSERT_EQ(1, fields[0]->AsPropertiesField()->GetProperties().size());
    ASSERT_EQ(1, fields[1]->AsPropertiesField()->GetProperties().size());
    EXPECT_EQ(elementClass->GetPropertyP("RegularProperty"), &fields[0]->AsPropertiesField()->GetProperties()[0].GetProperty());
    EXPECT_EQ(elementClass->GetPropertyP("HiddenProperty"), &fields[1]->AsPropertiesField()->GetProperties()[0].GetProperty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AppliesContentRulesInCorrectPriorityOrder,
    R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="RegularProperty" typeName="string" />
        <ECProperty propertyName="HiddenProperty" typeName="string">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00" />
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, AppliesContentRulesInCorrectPriorityOrder)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // set up input
    IECInstancePtr elementInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{elementInstance});

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* contentRule1 = new ContentRule("", 1000, false);
    rules->AddPresentationRule(*contentRule1);
    contentRule1->AddSpecification(*(new SelectedNodeInstancesSpecification(1000, false, "", "", true)));

    ContentRule* contentRule2 = new ContentRule("", 1001, false);
    rules->AddPresentationRule(*contentRule2);
    ContentSpecification* hiddenPropertySpec = new SelectedNodeInstancesSpecification(1001, false, "", "", true);
    contentRule2->AddSpecification(*hiddenPropertySpec);
    hiddenPropertySpec->AddPropertyOverride(*(new PropertySpecification("HiddenProperty", 1001, "", nullptr, true)));

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());
    ASSERT_TRUE(fields[0]->IsPropertiesField());
    ASSERT_EQ(1, fields[0]->AsPropertiesField()->GetProperties().size());
    EXPECT_EQ(elementClass->GetPropertyP("HiddenProperty"), &fields[0]->AsPropertiesField()->GetProperties()[0].GetProperty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_CollectsResultFromEachRecursiveStep, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_A" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_CollectsResultFromEachRecursiveStep)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAA = GetClass("A_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{a1});

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1000, false);
    auto nodesSpec = new ContentRelatedInstancesSpecification(1000, "", { new RepeatableRelationshipPathSpecification({
            new RepeatableRelationshipStepSpecification(relAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
            new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, "", 0),
            new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward, "", 0)
            })
        });

    rule->AddSpecification(*nodesSpec);
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", (int)ContentFlags::KeysOnly | (int)ContentFlags::IncludeInputKeys, *input)));

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    ASSERT_EQ(3, content->GetContentSet().GetSize());

    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult(*a1, *a2),
        InstanceInputAndResult(*a1, *b),
        InstanceInputAndResult(*a1, *c),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_DoesNotCollectResultsOfRecursiveStepsLeadingToNonRecursiveStep, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_A" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_DoesNotCollectResultsOfRecursiveStepsLeadingToNonRecursiveStep)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAA = GetClass("A_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{a1});

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1000, false);
    auto nodesSpec = new ContentRelatedInstancesSpecification(1000, "", { new RepeatableRelationshipPathSpecification({
            new RepeatableRelationshipStepSpecification(relAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
            new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, ""),
            new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward, "", 0)
            })
        });

    rule->AddSpecification(*nodesSpec);
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", (int)ContentFlags::KeysOnly | (int)ContentFlags::IncludeInputKeys, *input)));

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    ASSERT_EQ(2, content->GetContentSet().GetSize());

    RulesEngineTestHelpers::ValidateContentSet(
        {
        InstanceInputAndResult(*a1, *b),
        InstanceInputAndResult(*a1, *c),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_CollectsResultsOnlyFromLastNonRecursiveStep, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_A" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_CollectsResultsOnlyFromLastNonRecursiveStep)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAA = GetClass("A_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{a1});

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1000, false);
    auto nodesSpec = new ContentRelatedInstancesSpecification(1000, "", { new RepeatableRelationshipPathSpecification({
            new RepeatableRelationshipStepSpecification(relAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
            new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, "", 0),
            new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward, "")
            })
        });

    rule->AddSpecification(*nodesSpec);
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", (int)ContentFlags::KeysOnly | (int)ContentFlags::IncludeInputKeys, *input)));

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    RulesEngineTestHelpers::ValidateContentSet({ InstanceInputAndResult(*a1, *c) }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_CollectsResultsFromAllRecursiveStepsWithCircuralRalations, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_A" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_CollectsResultsFromAllRecursiveStepsWithCircuralRalations)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAA = GetClass("A_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{a1});

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1000, false);
    auto nodesSpec = new ContentRelatedInstancesSpecification(1000, "", { new RepeatableRelationshipPathSpecification({
            new RepeatableRelationshipStepSpecification(relAA->GetFullName(), RequiredRelationDirection_Forward, "", 0),
            new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward, "", 0),
            new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward, "", 0),
            new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Backward, "", 0),
            new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Backward, "", 0),
            new RepeatableRelationshipStepSpecification(relAA->GetFullName(), RequiredRelationDirection_Backward, "", 0),
            })
        });

    rule->AddSpecification(*nodesSpec);
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", (int)ContentFlags::KeysOnly | (int)ContentFlags::IncludeInputKeys, *input)));

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    ASSERT_EQ(4, content->GetContentSet().GetSize());

    RulesEngineTestHelpers::ValidateContentSet({
        InstanceInputAndResult(*a1, *a1),
        InstanceInputAndResult(*a1, *a2),
        InstanceInputAndResult(*a1, *b),
        InstanceInputAndResult(*a1, *c),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_CollectsResultsFromAllRecursiveStepsGoingBackwards, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_A" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentRelatedInstances_CollectsResultsFromAllRecursiveStepsGoingBackwards)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECRelationshipClassCP relAA = GetClass("A_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetClass("B_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAA, *a1, *a2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b, *c);

    // set up input
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>{c});

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* rule = new ContentRule("", 1000, false);
    auto nodesSpec = new ContentRelatedInstancesSpecification(1000, "", { new RepeatableRelationshipPathSpecification({
            new RepeatableRelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Backward, "", 0),
            new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Backward, "", 0),
            new RepeatableRelationshipStepSpecification(relAA->GetFullName(), RequiredRelationDirection_Backward, "", 0)
            })
        });

    rule->AddSpecification(*nodesSpec);
    rules->AddPresentationRule(*rule);

    // request
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", (int)ContentFlags::KeysOnly | (int)ContentFlags::IncludeInputKeys, *input)));

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    ASSERT_EQ(3, content->GetContentSet().GetSize());

    RulesEngineTestHelpers::ValidateContentSet({
        InstanceInputAndResult(*c, *b),
        InstanceInputAndResult(*c, *a2),
        InstanceInputAndResult(*c, *a1),
        }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_InstanceFilterWithSelectedInstanceKeys, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="long" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="long" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_InstanceFilterWithSelectedInstanceKeys)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance){instance.SetValue("PropB", ECValue(RulesEngineTestHelpers::GetInstanceKey(*a1).GetId().GetValue()));});
    IECInstancePtr b21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance){instance.SetValue("PropB", ECValue(RulesEngineTestHelpers::GetInstanceKey(*a2).GetId().GetValue()));});
    IECInstancePtr b22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [&](IECInstanceR instance){instance.SetValue("PropB", ECValue(RulesEngineTestHelpers::GetInstanceKey(*a2).GetId().GetValue()));});

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* rule = new ContentRule("", 1000, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, "SelectedInstanceKeys.AnyMatches(x => x.ECInstanceId = this.PropB)", classB->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    KeySetPtr input;
    ContentCPtr content;

    // request content for a1, confirm we get b1
    input = KeySet::Create(bvector<IECInstancePtr>{a1});
    content = GetVerifiedContent(*GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::KeysOnly, *input))));
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ b1.get() }, *content);

    // request content for a2, confirm we get b21 and b22
    input = KeySet::Create(bvector<IECInstancePtr>{a2});
    content = GetVerifiedContent(*GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::KeysOnly, *input))));
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ b21.get(), b22.get() }, *content);

    // request content for a3, confirm we get no results
    input = KeySet::Create(bvector<IECInstancePtr>{a3});
    content = GetVerifiedContent(*GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::KeysOnly, *input))));
    EXPECT_EQ(0, content->GetContentSet().GetSize());

    // request content for a node that groups all A instances, confirm we get all B instances
    input = KeySet::Create(*ECInstancesNodeKey::Create(
        {
        RulesEngineTestHelpers::GetInstanceKey(*a1),
        RulesEngineTestHelpers::GetInstanceKey(*a2),
        RulesEngineTestHelpers::GetInstanceKey(*a3),
        }, "", bvector<Utf8String>()));
    content = GetVerifiedContent(*GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::KeysOnly, *input))));
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ b1.get(), b21.get(), b22.get() }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_InstanceFilterWithSelectedInstanceKeysAndIsOfClass, R"*(
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_InstanceFilterWithSelectedInstanceKeysAndIsOfClass)
    {
    // set up data set
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    ECClassCP classD = GetClass("D");

    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* rule = new ContentRule("", 1000, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, "SelectedInstanceKeys.AnyMatches(x => this.IsOfClass(x.ECClassId))", classB->GetFullName(), true, false));
    rules->AddPresentationRule(*rule);

    // request content for c and expect to get c and d
    KeySetPtr input = KeySet::Create(*c);
    ContentCPtr content = GetVerifiedContent(*GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::KeysOnly, *input))));
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ c.get(), d.get() }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_ReturnsContentWhenGroupingNodeDependsOnRulesetVariables, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SelectedNodeInstances_ReturnsContentWhenGroupingNodeDependsOnRulesetVariables)
    {
    // set up data set
    ECClassCP classA = GetClass("A");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    RootNodeRule* nodeRule = new RootNodeRule("", 1000, false);
    nodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, ChildrenHint::Unknown, false, false, true, false, "GetVariableBoolValue(\"show_nodes\")", classA->GetFullName(), false));

    ContentRule* contentRule = new ContentRule("", 1000, false);
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification());

    rules->AddPresentationRule(*nodeRule);
    rules->AddPresentationRule(*contentRule);

    // request nodes
    RulesetVariables variables({ RulesetVariableEntry("show_nodes", true) });
    auto rootNodes = RulesEngineTestHelpers::GetValidatedNodes([&]() {return GetValidatedResponse(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), variables))); });
    ASSERT_EQ(1, rootNodes.GetSize());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, rootNodes[0]->GetType().c_str());

    // request
    KeySetPtr input = KeySet::Create(bvector<NavNodeKeyCPtr>{rootNodes[0]->GetKey()});
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), variables, nullptr, (int)ContentFlags::KeysOnly, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    // request for content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());
    ASSERT_EQ(1, content->GetContentSet().GetSize());

    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ a.get() }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsTypeNameAsExtendedTypeNameIfProvided, R"*(
    <ECEntityClass typeName="MyClass">
        <ECProperty propertyName="MyProperty" typeName="string" extendedTypeName="URL" />
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SetsTypeNameAsExtendedTypeNameIfProvided)
    {
    // insert some instances
    ECClassCP ecClass = GetClass("MyClass");
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *ecClass);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", ecClass->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    EXPECT_EQ(1, fields.size());

    ASSERT_EQ("URL", fields[0]->GetTypeDescription().GetTypeName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_InstanceFilterWithVariablesAndLambda, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, ContentInstancesOfSpecificClasses_InstanceFilterWithVariablesAndLambda)
    {
    // set up data set
    ECClassCP classA = GetClass("A");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    ContentRule* rule = new ContentRule("", 1000, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, "GetVariableIntValues(\"ids\").AnyMatches(x => x = this.ECInstanceId)", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // request content without variables
    ContentCPtr content = GetVerifiedContent(*GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, (int)ContentFlags::KeysOnly, *KeySet::Create()))));
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(0, content->GetContentSet().GetSize());

    // request content with variables (a2 id)
    RulesetVariables variables;
    variables.SetIntValues("ids", { (int64_t)BeInt64Id::FromString(a2->GetInstanceId().c_str()).GetValueUnchecked() });

    content = GetVerifiedContent(*GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), variables, nullptr, (int)ContentFlags::KeysOnly, *KeySet::Create()))));
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(1, content->GetContentSet().GetSize());
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ a2.get() }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DescriptorOverride_FilterWithLambda, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntProp" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (RulesDrivenECPresentationManagerContentTests, DescriptorOverride_FilterWithLambda)
    {
    // set up data set
    ECClassCP classA = GetClass("A");

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntProp", ECValue(1)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntProp", ECValue(10)); });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1000, "", classA->GetFullName(), false, false));
    rules->AddPresentationRule(*rule);

    // setup variables
    RulesetVariables variables;
    variables.SetIntValues("values", { (int64_t)10 });

    // get the descriptor
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), variables, nullptr, 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    // get the default content
    ContentCPtr content = GetVerifiedContent(*descriptor);
    ASSERT_TRUE(content.IsValid());

    // validate the default content set
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ a1.get(), a2.get() }, *content);

    // create the override
    ContentDescriptorPtr ovr = ContentDescriptor::Create(*descriptor);
    ovr->SetFieldsFilterExpression(Utf8PrintfString("GetVariableIntValues(\"values\").AnyMatches(x => x = %s)", FIELD_NAME(classA, "IntProp")));

    // get the content with descriptor override
    content = GetVerifiedContent(*ovr);
    ASSERT_TRUE(content.IsValid());

    // make sure the records in the content set are filtered
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ a2.get() }, *content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesNotUseContentModifiersThatDontMeetSchemaRequirements, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, DoesNotUseContentModifiersThatDontMeetSchemaRequirements)
    {
    // set up data set
    ECClassCP elementClass = GetClass("Element");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);

    // set up ruleset
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", elementClass->GetFullName(), true, false));
    rules->AddPresentationRule(*contentRule);

    ContentModifier* modifier = new ContentModifier(elementClass->GetSchema().GetName(), elementClass->GetName());
    modifier->AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("this schema doesn't exist"));
    modifier->AddCalculatedProperty(*new CalculatedPropertiesSpecification("custom property", 1, "\"calculated value\""));
    rules->AddPresentationRule(*modifier);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(0, fields.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AccountsForSupplementalRulePriority, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, AccountsForSupplementalRulePriority)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // set up primary ruleset
    PresentationRuleSetPtr primaryRules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*primaryRules);

    ContentRule* primaryContentRule = new ContentRule("", 0, true);
    primaryContentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    primaryRules->AddPresentationRule(*primaryContentRule);

    // set up supplemental ruleset
    PresentationRuleSetPtr supplementalRules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    supplementalRules->SetIsSupplemental(true);
    m_locater->AddRuleSet(*supplementalRules);

    ContentRule* supplementalContentRule = new ContentRule("", 1, true);
    supplementalContentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classB->GetFullName(), true, false));
    supplementalRules->AddPresentationRule(*supplementalContentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), primaryRules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *KeySet::Create())));
    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ b.get() }, *content);
    }
/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(HandlesSelectedNodeLabelInContentRuleCondition, R"*(
    <ECEntityClass typeName="B"/>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, HandlesSelectedNodeLabelInContentRuleCondition)
    {
    ECClassCP classB = GetClass("B");
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    InstanceLabelOverrideP labelOverrideRule = new InstanceLabelOverride(2, false, classB->GetFullName(), "");
    labelOverrideRule->AddValueSpecification(*new InstanceLabelOverrideClassNameValueSpecification());
    rules->AddPresentationRule(*labelOverrideRule);

    ContentRuleP rule = new ContentRule("SelectedNode.Label = \"B\" ", 1, false);
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*rule);
    // validate descriptor
    KeySetPtr input = KeySet::Create(bvector<IECInstancePtr>({ b }));
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), nullptr, 0, *input)));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ b.get() }, *content);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortAscending, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntProp" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SortingRule_SortAscending)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntProp", ECValue(3)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntProp", ECValue(1)); });
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntProp", ECValue(2)); });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SortingRuleP sortingRule = new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "IntProp", true, false, false);
    rules->AddPresentationRule(*sortingRule);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ a2.get(), a3.get(), a1.get() }, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortAscendingPolymorphically, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntProp" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SortingRule_SortAscendingPolymorphically)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntProp", ECValue(3)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntProp", ECValue(1)); });
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntProp", ECValue(2)); });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SortingRuleP sortingRule = new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "IntProp", true, false, true);
    rules->AddPresentationRule(*sortingRule);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{ b2.get(), b3.get(), b1.get() }, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortByMultiplePropertiesPolymorphically, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="IntPropB" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SortingRule_SortByMultiplePropertiesPolymorphically)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(3)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(3)); instance.SetValue("IntPropA", ECValue(3)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(1)); instance.SetValue("IntPropA", ECValue(3)); });
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropB", ECValue(3)); instance.SetValue("IntPropA", ECValue(2)); });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SortingRuleP sortingRule = new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "IntPropA", true, false, true);
    rules->AddPresentationRule(*sortingRule);
    SortingRuleP sortingRule2 = new SortingRule("", 2, classB->GetSchema().GetName(), classB->GetName(), "IntPropB", false, false, true);
    rules->AddPresentationRule(*sortingRule2);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{a2.get(), a3.get(), a1.get(), b3.get(), b1.get(), b2.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_DoNotSort, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SortingRule_DoNotSort)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(3)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SortingRuleP sortingRule1 = new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "IntPropA", true, false, false);
    rules->AddPresentationRule(*sortingRule1);
    SortingRuleP sortingRule2 = new SortingRule("", 2, classA->GetSchema().GetName(), classA->GetName(), "", false, true, false);
    rules->AddPresentationRule(*sortingRule2);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{a1.get(), a2.get(), a3.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_DoNotSortByLowerPriortyRules, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntProp1" typeName="int"/>
        <ECProperty propertyName="IntProp2" typeName="int"/>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SortingRule_DoNotSortByLowerPriortyRules)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(3)); instance.SetValue("IntProp2", ECValue(7)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(1)); instance.SetValue("IntProp2", ECValue(1)); });
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(2)); instance.SetValue("IntProp2", ECValue(3)); });
    IECInstancePtr a4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(2)); instance.SetValue("IntProp2", ECValue(2)); });
    IECInstancePtr a5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(2)); instance.SetValue("IntProp2", ECValue(1)); });
    IECInstancePtr a6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) { instance.SetValue("IntProp1", ECValue(3)); instance.SetValue("IntProp2", ECValue(3)); });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SortingRuleP sortingRule1 = new SortingRule("", 3, classA->GetSchema().GetName(), classA->GetName(), "IntProp1", true, false, false);
    rules->AddPresentationRule(*sortingRule1);
    SortingRuleP sortingRule2 = new SortingRule("", 2, classA->GetSchema().GetName(), classA->GetName(), "", false, true, false);
    rules->AddPresentationRule(*sortingRule2);
    SortingRuleP sortingRule3 = new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "IntProp2", true, false, false);
    rules->AddPresentationRule(*sortingRule3);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{a2.get(), a3.get(), a4.get(), a5.get(), a1.get(), a6.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_DoNotSortDerivedClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SortingRule_DoNotSortDerivedClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(3)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(3)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SortingRuleP sortingRule = new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "IntPropA", true, false, true);
    rules->AddPresentationRule(*sortingRule);
    SortingRuleP sortingRule2 = new SortingRule("", 2, classB->GetSchema().GetName(), classB->GetName(), "", false, true, true);
    rules->AddPresentationRule(*sortingRule2);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{a2.get(), a3.get(), a1.get(), b1.get(), b2.get(), b3.get()}, *content, true);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_OnlySortBaseClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="IntPropA" typeName="int"/>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(RulesDrivenECPresentationManagerContentTests, SortingRule_OnlySortBaseClasses)
    {
    // set up data set
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(3)); });
    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr a3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(3)); });
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(1)); });
    IECInstancePtr b3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR instance) {instance.SetValue("IntPropA", ECValue(2)); });
    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    SortingRuleP sortingRule = new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "IntPropA", true, false, false);
    rules->AddPresentationRule(*sortingRule);
    ContentRule* contentRule = new ContentRule();
    contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", classA->GetFullName(), true, false));
    rules->AddPresentationRule(*contentRule);

    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ASSERT_TRUE(descriptor.IsValid());

    ContentCPtr content = GetVerifiedContent(*descriptor);
    RulesEngineTestHelpers::ValidateContentSet(bvector<IECInstanceCP>{b1.get(), b2.get(), b3.get(), a2.get(), a3.get(), a1.get()}, *content, true);
    }
