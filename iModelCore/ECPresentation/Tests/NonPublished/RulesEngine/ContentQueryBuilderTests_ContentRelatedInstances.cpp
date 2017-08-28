/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentQueryBuilderTests_ContentRelatedInstances.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsNullDescriptorWhenNoSelectedNodes)
    {
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Both, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, TestParsedSelectionInfo());
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsForwardRelatedInstanceQueryWhenSelectedOneInstanceNode)
    {
    ECClassCP ecClass = GetECClass("RulesEngineTest", "Gadget");
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, "", "RulesEngineTest:Widget,Sprocket");
        
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_ReturnsForwardRelatedInstanceQueryWhenSelectedOneInstanceNode");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsBackwardRelatedInstanceQueryWhenSelectedOneInstanceNode)
    {
    ECClassCP ecClass = GetECClass("RulesEngineTest", "Gadget");
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Backward, "", "RulesEngineTest:Widget,Sprocket");
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_ReturnsBackwardRelatedInstanceQueryWhenSelectedOneInstanceNode");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsBothDirectionsRelatedInstanceQueryWhenSelectedOneInstanceNode)
    {
    ECClassCP ecClass = GetECClass("RulesEngineTest", "Gadget");
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Both, "", "RulesEngineTest:Widget,Sprocket");
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_ReturnsBothDirectionsRelatedInstanceQueryWhenSelectedOneInstanceNode");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass)
    {
    ECClassCP ecClass = GetECClass("RulesEngineTest", "Gadget");
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, "", "RulesEngineTest:Sprocket");
    
    TestParsedSelectionInfo info(*ecClass, {ECInstanceId((uint64_t)123), ECInstanceId((uint64_t)125)});
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses)
    {
    ECClassCP ecClass1 = GetECClass("RulesEngineTest", "Widget");
    ECClassCP ecClass2 = GetECClass("RulesEngineTest", "Sprocket");
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Both, 
        "RulesEngineTest:WidgetHasGadgets,GadgetHasSprockets", "RulesEngineTest:Gadget");
    
    TestParsedSelectionInfo info({
        bpair<ECClassCP, ECInstanceId>(ecClass1, {ECInstanceId((uint64_t)123)}), 
        bpair<ECClassCP, ECInstanceId>(ecClass2, {ECInstanceId((uint64_t)125)}), 
        });
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_AppliesInstanceFilter)
    {
    ECClassCP ecClass = GetECClass("RulesEngineTest", "Gadget");
    ContentRelatedInstancesSpecification spec(1, 0, false, "this.MyID = \"Sprocket MyID\"", RequiredRelationDirection_Forward, "", "RulesEngineTest:Sprocket");
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_AppliesInstanceFilter");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_SkipsRelatedLevel)
    {
    ECClassCP ecClass = GetECClass("RulesEngineTest", "Widget");
    ContentRelatedInstancesSpecification spec(1, 1, false, "", RequiredRelationDirection_Forward, "", "RulesEngineTest:Sprocket");
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_SkipsRelatedLevel");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_SkipsRelatedLevelWithSpecifiedRelationship)
    {
    ECClassCP ecClass = GetECClass("RulesEngineTest", "Sprocket");
    ContentRelatedInstancesSpecification spec(1, 1, false, "", RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget");
    
    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_SkipsRelatedLevelWithSpecifiedRelationship");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_CreatesRecursiveQuery, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward" />
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_CreatesRecursiveQuery)
    {
    ECClassCP rel = GetECClass("ElementOwnsChildElements");
    ECClassCP ecClass = GetECClass("Element");
    ContentRelatedInstancesSpecification spec(1, 0, true, "", RequiredRelationDirection_Forward, rel->GetFullName(), ecClass->GetFullName());
    
    TestParsedSelectionInfo info(*ecClass, {ECInstanceId((uint64_t)123), ECInstanceId((uint64_t)456)});
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = GetExpectedQuery();
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_CreatesRecursiveQueryWhenRelationshipIsOnBaseClass, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementProperty" typeName="int" />
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Sheet">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="SheetProperty" typeName="int" />
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_CreatesRecursiveQueryWhenRelationshipIsOnBaseClass)
    {
    ECClassCP baseClass = GetECClass("Element");
    ECClassCP derivedClass = GetECClass("Sheet");
    ECClassCP rel = GetECClass("ElementOwnsChildElements");
    ContentRelatedInstancesSpecification spec(1, 0, true, "", RequiredRelationDirection_Forward, rel->GetFullName(), baseClass->GetFullName());
    
    TestParsedSelectionInfo info(*derivedClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = GetExpectedQuery();
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }
