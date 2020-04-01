/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                04/2016
//---------------------------------------------------------------------------------------
void ContentQueryBuilderTests::SetUp()
    {
    ECPresentationTest::SetUp();
    Localization::Init();
    IConnectionManagerCR connections = ExpectedQueries::GetInstance(BeTest::GetHost()).GetConnections();
    IConnectionCR connection = ExpectedQueries::GetInstance(BeTest::GetHost()).GetConnection();
    m_ruleset = PresentationRuleSet::CreateInstance("", 1, 0, false, "", "", "", false);
    m_schemaHelper = new ECSchemaHelper(connection, nullptr, nullptr, nullptr);
    m_context = new ContentDescriptorBuilder::Context(*m_schemaHelper, connections,
        connection, *m_ruleset, ContentDisplayType::Undefined, m_categorySupplier, nullptr, ECPresentation::UnitSystem::Undefined, 
        &m_localizationProvider, "test locale", *NavNodeKeyListContainer::Create(), nullptr);
    m_context->SetContentFlagsCalculator([](int defaultFlags){return defaultFlags | (int)ContentFlags::SkipInstancesCheck;});
    m_descriptorBuilder = new ContentDescriptorBuilder(*m_context);
    m_queryBuilder = new ContentQueryBuilder(ContentQueryBuilderParameters(*m_schemaHelper, connections,
        m_nodesLocater, connection, *m_ruleset, m_context->GetLocale(), m_settings, m_schemaHelper->GetECExpressionsCache(), 
        m_categorySupplier, nullptr, nullptr, &m_localizationProvider));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                04/2016
//---------------------------------------------------------------------------------------
void ContentQueryBuilderTests::TearDown()
    {
    DELETE_AND_CLEAR(m_schemaHelper);
    DELETE_AND_CLEAR(m_context);
    DELETE_AND_CLEAR(m_descriptorBuilder);
    DELETE_AND_CLEAR(m_queryBuilder);
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryCPtr ContentQueryBuilderTests::GetExpectedQuery()
    {
    return ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery(BeTest::GetNameOfCurrentTest());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ContentQueryBuilderTests::GetECSchema()
    {
    return ExpectedQueries::GetInstance(BeTest::GetHost()).GetDb().Schemas().GetSchema(BeTest::GetNameOfCurrentTest());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ContentQueryBuilderTests::GetECClass(Utf8CP schemaName, Utf8CP className)
    {
    return ExpectedQueries::GetInstance(BeTest::GetHost()).GetECClass(schemaName, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ContentQueryBuilderTests::GetECClass(Utf8CP className)
    {
    return GetECClass(BeTest::GetNameOfCurrentTest(), className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, FieldNamesDontCollideWhenSelectingInstanceAndRelatedPropertyOfTheSameClass)
    {
    ECClassCP widgetClass = GetECClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget");

    // set up selection
    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(gadgetClass, {ECInstanceId((uint64_t)1)}), 
        bpair<ECClassCP, ECInstanceId>(widgetClass, {ECInstanceId((uint64_t)2)}), 
        });

    // create the spec
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, 
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID", RelationshipMeaning::RelatedInstance));
    spec.AddPropertyOverride(*new PropertySpecification("MyID", 1000, "", "", true));

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    // compare
    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("FieldNamesDontCollideWhenSelectingInstanceAndRelatedPropertyOfTheSameClass");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, FieldNamesContainNamesOfAllRelatedClassesWhenSelectingMultipleClassesWithSameRelatedProperty)
    {
    // create the spec
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Widget,Sprocket", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Both, 
        "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "RulesEngineTest:Gadget", "MyID", RelationshipMeaning::RelatedInstance));

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    // compare
    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("FieldNamesContainNamesOfAllRelatedClassesWhenSelectingMultipleClassesWithSameRelatedProperty");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, AppliesRelatedPropertiesSpecificationFromContentModifier)
    {    
    // create the specs
    m_ruleset->AddPresentationRule(*new ContentModifier("RulesEngineTest", "Sprocket"));
    m_ruleset->GetContentModifierRules().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "Description", RelationshipMeaning::RelatedInstance));
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Sprocket", false);
    
    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    // compare
    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("AppliesRelatedPropertiesSpecificationFromContentModifier");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, DoesntApplyRelatedPropertiesSpecificationFromContentModifierToNestedRelatedClasses)
    {    
    // create the specs
    m_ruleset->AddPresentationRule(*new ContentModifier("RulesEngineTest", "Gadget"));
    m_ruleset->GetContentModifierRules().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "Description", RelationshipMeaning::RelatedInstance));
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Sprocket", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:GadgetHasSprockets",
        "RulesEngineTest:Gadget", "Description", RelationshipMeaning::RelatedInstance));
        
    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    // compare
    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("DoesntApplyRelatedPropertiesSpecificationFromContentModifierToNestedRelatedClasses");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, RelatedPropertiesAreAppendedCorrectlyWhenUsingCustomDescriptor)
    {    
    // create the specs
    m_ruleset->AddPresentationRule(*new ContentModifier("RulesEngineTest", "Sprocket"));
    m_ruleset->GetContentModifierRules().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "Description", RelationshipMeaning::RelatedInstance));
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Sprocket", false);
    
    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    // compare
    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("RelatedPropertiesAreAppendedCorrectlyWhenUsingCustomDescriptor");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, CreatesContentFieldsForXToManyRelatedInstanceProperties)
    {    
    // create the specs
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Gadget", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, 
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket", "Description", RelationshipMeaning::RelatedInstance));
    
    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    // compare
    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("CreatesContentFieldsForXToManyRelatedInstanceProperties");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected->GetContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(query->GetContract()->GetDescriptor().AsJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, CreatesNestedContentFieldsForXToManyRelatedInstanceProperties)
    {    
    // create the specs
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Widget", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, 
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget", "Description", RelationshipMeaning::RelatedInstance));
    spec.GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, 
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket", "Description", RelationshipMeaning::RelatedInstance));
    
    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    // compare
    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("CreatesNestedContentFieldsForXToManyRelatedInstanceProperties");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected->GetContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(query->GetContract()->GetDescriptor().AsJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryBuilderTests, CreatesNestedContentFieldsForXToManySameInstanceProperties)
    {
    // create the specs
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Widget", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget", "Description", RelationshipMeaning::RelatedInstance));
    spec.GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket", "Description", RelationshipMeaning::SameInstance));

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    // compare
    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery(BeTest::GetNameOfCurrentTest());
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected->GetContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(query->GetContract()->GetDescriptor().AsJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, NestsFilterExpressionQuery)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1,"", "RulesEngineTest:Widget", false);

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    descriptor->SetFilterExpression(Utf8PrintfString("%s = \"WidgetId\"", FIELD_NAME(GetECClass("RulesEngineTest", "Widget"), "MyID")).c_str());
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("FilterExpressionQueryTest");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected->GetContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(query->GetContract()->GetDescriptor().AsJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SetsShowImagesFlag)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.SetShowImages(true);
    
    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SetsShowImagesFlag");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SetsShowLabelsFlagForGridContentType)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Grid);

    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
        
    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SetsShowLabelsFlagForGridContentType");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SetsNoFieldsAndKeysOnlyFlagForGraphicsContentType)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Graphics);

    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    
    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SetsNoFieldsAndKeysOnlyFlagForGraphicsContentType");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SetsNoFieldsAndShowLabelsFlagsForListContentType)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::List);

    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    
    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("SetsNoFieldsAndShowLabelsFlagsForListContentType");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, AppliesRelatedInstanceSpecificationForTheSameXToManyRelationshipAndClassTwoTimes)
    {
    ECClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget");
    SelectedNodeInstancesSpecification spec(1, false, "RulesEngineTest", "Gadget", false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget", "widgetAlias"));
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetsHaveGadgets", "RulesEngineTest:Widget", "widgetAlias2"));

    TestParsedInput info(*gadgetClass, ECInstanceId((uint64_t) 123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor, info);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery(BeTest::GetNameOfCurrentTest());
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }