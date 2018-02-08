/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentQueryBuilderTests_ContentRelatedInstances.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsNullDescriptorWhenNoSelectedNodes)
    {
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Both, "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, TestParsedSelectionInfo());
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = GetExpectedQuery();
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected->GetContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(query->GetContract()->GetDescriptor().AsJson());
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
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = GetExpectedQuery();
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected->GetContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(query->GetContract()->GetDescriptor().AsJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_DoesntSplitRecursiveQueryClassesIntoDerivedClasses, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_DoesntSplitRecursiveQueryClassesIntoDerivedClasses)
    {
    ECClassCP baseClass = GetECClass("Element");
    ECClassCP derivedClass = GetECClass("Sheet");
    ECClassCP rel = GetECClass("ElementOwnsChildElements");

    ContentRelatedInstancesSpecification spec(1, 0, true, "", RequiredRelationDirection_Forward, rel->GetFullName(), baseClass->GetFullName());
    m_ruleset->AddPresentationRule(*new ContentModifier(GetECSchema()->GetName(), derivedClass->GetName()));
    
    TestParsedSelectionInfo info(*derivedClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = GetExpectedQuery();
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected->GetContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(query->GetContract()->GetDescriptor().AsJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_SelectPointPropertyRawDataGroupedByDisplayValue)
    {
    ECClassCP ecClass = GetECClass("RulesEngineTest", "ClassD");
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassH");
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("PointProperty", 1000, true));

    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::DistinctValues);

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_SelectPointPropertyRawDataGroupedByDisplayValue");
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_InstanceLabelOverride_AppliedByPriorityForSpecifiedClass)
    {
    ECClassCP ecClass = GetECClass("RulesEngineTest", "Gadget");
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, "", "RulesEngineTest:Sprocket");
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID,Description", 1000, true));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Sprocket", "MyID"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, true, "RulesEngineTest:Sprocket", "Description"));      

    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_InstanceLabelOverride_AppliedByPriorityForSpecifiedClass");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_InstanceLabelOverride_OverrideNavigationProperty)
    {

    ECClassCP ecClass = GetECClass("RulesEngineTest", "Gadget");
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, "", "RulesEngineTest:Widget,Sprocket");
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("MyID,Gadget", 1000, true));   
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));    

    TestParsedSelectionInfo info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentRelatedInstances_InstanceLabelOverride_OverrideNavigationProperty");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }