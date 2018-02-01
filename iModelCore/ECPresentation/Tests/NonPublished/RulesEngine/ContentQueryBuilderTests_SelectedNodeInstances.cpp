/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentQueryBuilderTests_SelectedNodeInstances.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsNullDescriptorWhenNoSelectedNodes)
    {
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, TestParsedSelectionInfo());
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedOneInstanceNode)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info(*ecClass, {ECInstanceId((uint64_t)123), ECInstanceId((uint64_t)125)});
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
    
    TestParsedSelectionInfo info({
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
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info({
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
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("PropertyC", 1000, false));
    
    TestParsedSelectionInfo info(*class3, ECInstanceId((uint64_t)123));
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
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesMultipleHiddenProperties)
    {
    ECClassCP class3 = GetECClass("SchemaComplex", "Class3");
    SelectedNodeInstancesSpecification spec(1, false, "SchemaComplex", "Class3", false);
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("PropertyC,PropertyD", 1000, false));
    
    TestParsedSelectionInfo info(*class3, ECInstanceId((uint64_t)123));
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
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("PropertyB", 1000, false));
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("PropertyC", 1000, false));
    
    TestParsedSelectionInfo info({
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
    
    TestParsedSelectionInfo info(*gadgetClass, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info(*gadgetClass, ECInstanceId((uint64_t)123));    
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
    
    TestParsedSelectionInfo info(*sprocketClass, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info(*class2, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info(*gadget, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info(*gadgetClass, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info(*sprocketClass, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info(*sprocketClass, ECInstanceId((uint64_t)123));
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
    
    TestParsedSelectionInfo info(*sprocketClass, ECInstanceId((uint64_t)123));
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

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_SelectsRawValueAndGroupsByDisplayValue)
    {
    ECClassCP classH = GetECClass("RulesEngineTest", "ClassH");
    TestParsedSelectionInfo info(*classH, ECInstanceId((uint64_t)123));

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("PointProperty", 1000, true));

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

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_InstanceLabelOverride_AppliedByPriority)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Grid);

    ECClassCP ecClass = GetECClass("RulesEngineTest", "Widget");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID,Description", 1000, true));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, true, "RulesEngineTest:Widget", "Description"));
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
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
    TestParsedSelectionInfo info({
        bpair<ECClassCP, ECInstanceId>(gadgetClass, {ECInstanceId((uint64_t)1)}), 
        bpair<ECClassCP, ECInstanceId>(widgetClass, {ECInstanceId((uint64_t)2)}), 
        });

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID", 1000, true));
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
    TestParsedSelectionInfo info({
        bpair<ECClassCP, ECInstanceId>(gadgetClass, {ECInstanceId((uint64_t)1)}), 
        bpair<ECClassCP, ECInstanceId>(widgetClass, {ECInstanceId((uint64_t)2)}), 
        });

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID,Widget", 1000, true));
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

