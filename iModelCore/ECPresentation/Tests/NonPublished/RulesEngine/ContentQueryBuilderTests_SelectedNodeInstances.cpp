/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentQueryBuilderTests_SelectedNodeInstances.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsNullDescriptorWhenNoSelectedNodes)
    {
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, TestParsedSelectionInfo());
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
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
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
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
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
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetPropertiesDisplaySpecificationsR().push_back(new PropertiesDisplaySpecification("PropertyC", 1000, false));
    
    TestParsedSelectionInfo info(*class3, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetPropertiesDisplaySpecificationsR().push_back(new PropertiesDisplaySpecification("PropertyC,PropertyD", 1000, false));
    
    TestParsedSelectionInfo info(*class3, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetPropertiesDisplaySpecificationsR().push_back(new PropertiesDisplaySpecification("PropertyB", 1000, false));
    spec.GetPropertiesDisplaySpecificationsR().push_back(new PropertiesDisplaySpecification("PropertyC", 1000, false));
    
    TestParsedSelectionInfo info({
        bpair<ECClassCP, ECInstanceId>(class2, {ECInstanceId((uint64_t)123)}), 
        bpair<ECClassCP, ECInstanceId>(class3, {ECInstanceId((uint64_t)123)})
        });
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "IntProperty", RelationshipMeaning::RelatedInstance));
    
    TestParsedSelectionInfo info(*gadgetClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "IntProperty,LongProperty", RelationshipMeaning::RelatedInstance));
    
    TestParsedSelectionInfo info(*gadgetClass, ECInstanceId((uint64_t)123));    
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "", RelationshipMeaning::RelatedInstance));
    
    TestParsedSelectionInfo info(*sprocketClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "SchemaComplex:Class1HasClass2And3", "SchemaComplex:Class1", "", RelationshipMeaning::RelatedInstance));
    
    TestParsedSelectionInfo info(*class2, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Both,
        "RulesEngineTest:WidgetHasGadget,GadgetHasSprocket", "", "IntProperty,Gadget", RelationshipMeaning::RelatedInstance));
    
    TestParsedSelectionInfo info(*gadget, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadget,WidgetHasGadgets", "", "Description", RelationshipMeaning::RelatedInstance));
    
    TestParsedSelectionInfo info(*gadgetClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    relatedPropertiesSpec->GetNestedRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadget,WidgetHasGadgets", "", "IntProperty", RelationshipMeaning::RelatedInstance));
    spec.GetRelatedPropertiesR().push_back(relatedPropertiesSpec);
    
    TestParsedSelectionInfo info(*sprocketClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    relatedPropertiesSpec->GetNestedRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadget", "", "Description", RelationshipMeaning::RelatedInstance));
    spec.GetRelatedPropertiesR().push_back(relatedPropertiesSpec);
    
    TestParsedSelectionInfo info(*sprocketClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    relatedPropertiesSpec->GetNestedRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadget", "", "Description", RelationshipMeaning::RelatedInstance));
    spec.GetRelatedPropertiesR().push_back(relatedPropertiesSpec);
    
    TestParsedSelectionInfo info(*sprocketClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_AddsNestedRelatedProperties2");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_SetsShowImagesFlag)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.SetShowImages(true);
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_SetsShowImagesFlag");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_SetsShowLabelsFlagForGridContentType)
    {
    m_builder->GetParameters().SetPreferredDisplayType(ContentDisplayType::Grid);

    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
        
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_SetsShowLabelsFlagForGridContentType");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_SetsKeysOnlyFlagForGraphicsContentType)
    {
    m_builder->GetParameters().SetPreferredDisplayType(ContentDisplayType::Graphics);

    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SelectedNodeInstances_SetsKeysOnlyFlagForGraphicsContentType");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }
