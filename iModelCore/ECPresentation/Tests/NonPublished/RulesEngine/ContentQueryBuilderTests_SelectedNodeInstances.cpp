/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsNullDescriptorWhenNoSelectedNodes)
    {
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, TestParsedInput());
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedOneInstanceNode)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);

    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedOneInstanceNode");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);

    TestParsedInput info(*ecClass, {ECInstanceId((uint64_t)123), ECInstanceId((uint64_t)125)});
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses)
    {
    ECClassCP ecClass1 = GetECClass("Basic1", "Class1A");
    ECClassCP ecClass2 = GetECClass("Basic2", "Class2");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);

    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(ecClass1, {ECInstanceId((uint64_t)123)}),
        bpair<ECClassCP, ECInstanceId>(ecClass2, {ECInstanceId((uint64_t)123)})
        });
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_FiltersSelectedNodesBySchemaName)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "Basic2", "", false);

    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_FiltersSelectedNodesByClassName)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "Class1B", false);

    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_FiltersSelectedNodesByClassNamePolymorphically)
    {
    ECClassCP classA = GetECClass("Basic4", "ClassA");
    ECClassCP classB = GetECClass("Basic4", "ClassB");
    ECClassCP classC = GetECClass("Basic4", "ClassC");
    SelectedNodeInstancesSpecification spec(1, false, "", "ClassB", true);

    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(classA, {ECInstanceId((uint64_t)123)}),
        bpair<ECClassCP, ECInstanceId>(classB, {ECInstanceId((uint64_t)123)}),
        bpair<ECClassCP, ECInstanceId>(classC, {ECInstanceId((uint64_t)123)})
        });
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_FiltersSelectedNodesByClassNamePolymorphically");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesHiddenProperty)
    {
    ECClassCP class3 = GetECClass("SchemaComplex", "Class3");
    SelectedNodeInstancesSpecification spec(1, false, "SchemaComplex", "Class3", false);
    spec.AddPropertyOverride(*new PropertySpecification("PropertyC", 1000, "", "", false));

    TestParsedInput info(*class3, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_RemovesHiddenProperty");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_RemovesAllHiddenPropertiesWhenHidingAtSpecificationLevel, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="PhysicalProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesAllHiddenPropertiesWhenHidingAtSpecificationLevel)
    {
    ECClassCP ecClass = GetECClass("PhysicalElement");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertyOverride(*new PropertySpecification("*", 1000, "", "", false));

    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_RemovesAllHiddenPropertiesWhenHidingAtSpecificationLevel");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_RemovesAllHiddenBaseClassPropertiesWhenHidingAtContentModifierLevel, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="PhysicalProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesAllHiddenBaseClassPropertiesWhenHidingAtContentModifierLevel)
    {
    ECClassCP baseClass = GetECClass("Element");
    ECClassCP derivedClass = GetECClass("PhysicalElement");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    ContentModifier* modifier = new ContentModifier(baseClass->GetSchema().GetName(), baseClass->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("*", 1000, "", "", false));
    m_ruleset->AddPresentationRule(*modifier);

    TestParsedInput info(*derivedClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_RemovesAllHiddenBaseClassPropertiesWhenHidingAtContentModifierLevel");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_RemovesAllHiddenDerivedClassPropertiesWhenHidingAtContentModifierLevel, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="PhysicalProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesAllHiddenDerivedClassPropertiesWhenHidingAtContentModifierLevel)
    {
    ECClassCP derivedClass = GetECClass("PhysicalElement");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    ContentModifier* modifier = new ContentModifier(derivedClass->GetSchema().GetName(), derivedClass->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("*", 1000, "", "", false));
    m_ruleset->AddPresentationRule(*modifier);

    TestParsedInput info(*derivedClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_RemovesAllHiddenDerivedClassPropertiesWhenHidingAtContentModifierLevel");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesMultipleHiddenProperties)
    {
    ECClassCP class3 = GetECClass("SchemaComplex", "Class3");
    SelectedNodeInstancesSpecification spec(1, false, "SchemaComplex", "Class3", false);
    spec.AddPropertyOverride(*new PropertySpecification("PropertyC", 1000, "", "", false));
    spec.AddPropertyOverride(*new PropertySpecification("PropertyD", 1000, "", "", false));

    TestParsedInput info(*class3, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_RemovesMultipleHiddenProperties");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesMultipleHiddenPropertiesOfDifferentClasses)
    {
    ECClassCP class2 = GetECClass("SchemaComplex", "Class2");
    ECClassCP class3 = GetECClass("SchemaComplex", "Class3");
    SelectedNodeInstancesSpecification spec(1, false, "SchemaComplex", "", false);
    spec.AddPropertyOverride(*new PropertySpecification("PropertyB", 1000, "", "", false));
    spec.AddPropertyOverride(*new PropertySpecification("PropertyC", 1000, "", "", false));

    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(class2, {ECInstanceId((uint64_t)123)}),
        bpair<ECClassCP, ECInstanceId>(class3, {ECInstanceId((uint64_t)123)})
        });
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_RemovesMultipleHiddenPropertiesOfDifferentClasses");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsSingleRelatedProperty)
    {
    ECClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget");
    SelectedNodeInstancesSpecification spec(1, false, "RulesEngineTest", "Gadget", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "IntProperty", RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*gadgetClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsSingleRelatedProperty");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsMultipleRelatedProperties)
    {
    ECClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget");
    SelectedNodeInstancesSpecification spec(1, false, "RulesEngineTest", "Gadget", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "IntProperty,LongProperty", RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*gadgetClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsMultipleRelatedProperties");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsAllRelatedProperties)
    {
    ECClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket");
    SelectedNodeInstancesSpecification spec(1, false, "RulesEngineTest", "Sprocket", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "", RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*sprocketClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsAllRelatedProperties");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsBackwardRelatedProperties)
    {
    ECClassCP class2 = GetECClass("SchemaComplex", "Class2");
    SelectedNodeInstancesSpecification spec(1, false, "SchemaComplex", "Class2", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "SchemaComplex:Class1HasClass2And3", "SchemaComplex:Class1", "", RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*class2, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsBackwardRelatedProperties");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* note: this test not only tests relationships on both directions, but also:
* - multiple specified relationships between different classes;
* - empty related classes property (defaults to "any class");
* - a property that exists in different related classes.
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsBothDirectionsRelatedProperties)
    {
    ECClassCP gadget = GetECClass("RulesEngineTest", "Gadget");

    SelectedNodeInstancesSpecification spec(1, false, "RulesEngineTest", "Gadget", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Both,
        "RulesEngineTest:WidgetHasGadget,GadgetHasSprocket", "", "IntProperty,Gadget", RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*gadget, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsBothDirectionsRelatedProperties");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsPropertiesOfTheSameClassFoundByFollowingDifferentRelationships)
    {
    ECClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget");
    SelectedNodeInstancesSpecification spec(1, false, "RulesEngineTest", "Gadget", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadget,WidgetHasGadgets", "", "Description", RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*gadgetClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsPropertiesOfTheSameClassFoundByFollowingDifferentRelationships");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsNestedPropertiesOfTheSameClassFoundByFollowingDifferentRelationships)
    {
    ECClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket");
    SelectedNodeInstancesSpecification spec(1, false, "RulesEngineTest", "Sprocket", false);
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "", "_none_", RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadget,WidgetHasGadgets", "", "IntProperty", RelationshipMeaning::RelatedInstance));
    spec.AddRelatedProperty(*relatedPropertiesSpec);

    TestParsedInput info(*sprocketClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsNestedPropertiesOfTheSameClassFoundByFollowingDifferentRelationships");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsNestedRelatedProperties)
    {
    ECClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket");
    SelectedNodeInstancesSpecification spec(1, false, "RulesEngineTest", "Sprocket", false);
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "", "_none_", RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadget", "", "Description", RelationshipMeaning::RelatedInstance));
    spec.AddRelatedProperty(*relatedPropertiesSpec);

    TestParsedInput info(*sprocketClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsNestedRelatedProperties");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsNestedRelatedProperties2)
    {
    ECClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket");
    SelectedNodeInstancesSpecification spec(1, false, "RulesEngineTest", "Sprocket", false);
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "", "Description", RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadget", "", "Description", RelationshipMeaning::RelatedInstance));
    spec.AddRelatedProperty(*relatedPropertiesSpec);

    TestParsedInput info(*sprocketClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsNestedRelatedProperties2");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_SelectsRawValueAndGroupsByDisplayValue)
    {
    ECClassCP classH = GetECClass("RulesEngineTest", "ClassH");
    TestParsedInput info(*classH, ECInstanceId((uint64_t)123));

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertyOverride(*new PropertySpecification("PointProperty", 1000, "", "", true));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::DistinctValues);

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_SelectsRawValueAndGroupsByDisplayValue");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected->GetContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(query->GetContract()->GetDescriptor().AsJson());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_InstanceLabelOverride_AppliedByPriority)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Grid);

    ECClassCP ecClass = GetECClass("RulesEngineTest", "Widget");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));
    spec.AddPropertyOverride(*new PropertySpecification("Description", 1000, "", "", true));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, true, "RulesEngineTest:Widget", "Description"));
    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_InstanceLabelOverride_AppliedByPriority");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_InstanceLabelOverride_OverrideSpecifiedClassInstancesLabelsWhenMultipleClassesSelected)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Grid);

    ECClassCP widgetClass = GetECClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget");

    // set up selection
    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(gadgetClass, {ECInstanceId((uint64_t)1)}),
        bpair<ECClassCP, ECInstanceId>(widgetClass, {ECInstanceId((uint64_t)2)}),
        });

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_InstanceLabelOverride_OverrideSpecifiedClassInstancesLabelsWhenMultipleClassesSelected");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_InstanceLabelOverride_OverrideNavigationProperty)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Grid);

    ECClassCP widgetClass = GetECClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget");

    // set up selection
    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(gadgetClass, {ECInstanceId((uint64_t)1)}),
        bpair<ECClassCP, ECInstanceId>(widgetClass, {ECInstanceId((uint64_t)2)}),
        });

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));
    spec.AddPropertyOverride(*new PropertySpecification("Widget", 1000, "", "", true));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);


    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_InstanceLabelOverride_OverrideNavigationProperty");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_JoinsRelatedInstanceWithInnerJoin)
    {
    ECClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget");
    TestParsedInput info(*gadgetClass, ECInstanceId((uint64_t) 1));

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket", "sprocketAlias", true));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_JoinsRelatedInstanceWithInnerJoin");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }
