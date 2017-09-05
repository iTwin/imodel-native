/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentQueryBuilderTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                04/2016
//---------------------------------------------------------------------------------------
void ContentQueryBuilderTests::SetUp()
    {
    Localization::Init();
    m_ruleset = PresentationRuleSet::CreateInstance("", 1, 0, false, "", "", "", false);
    m_schemaHelper = new ECSchemaHelper(ExpectedQueries::GetInstance(BeTest::GetHost()).GetDb(), m_relatedPathsCache, nullptr);
    m_builder = new ContentQueryBuilder(ContentQueryBuilderParameters(*m_schemaHelper, 
        m_nodesLocater, *m_ruleset, ContentDisplayType::Undefined, m_settings, m_expressionsCache, 
        m_categorySupplier, nullptr, nullptr, &m_localizationProvider));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                04/2016
//---------------------------------------------------------------------------------------
void ContentQueryBuilderTests::TearDown()
    {
    delete m_builder;
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
    TestParsedSelectionInfo info({
        bpair<ECClassCP, ECInstanceId>(gadgetClass, {ECInstanceId((uint64_t)1)}), 
        bpair<ECClassCP, ECInstanceId>(widgetClass, {ECInstanceId((uint64_t)2)}), 
        });

    // create the spec
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, 
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "MyID", RelationshipMeaning::RelatedInstance));
    spec.GetPropertiesDisplaySpecificationsR().push_back(new PropertiesDisplaySpecification("MyID", 1000, true));

    // get the query
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor, info);
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
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Both, 
        "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "RulesEngineTest:Gadget", "MyID", RelationshipMeaning::RelatedInstance));

    // get the query
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor);
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
    m_ruleset->GetContentModifierRules().back()->GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "Description", RelationshipMeaning::RelatedInstance));
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Sprocket", false);
    
    // get the query
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor);
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
    m_ruleset->GetContentModifierRules().back()->GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "Description", RelationshipMeaning::RelatedInstance));
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Sprocket", false);
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:GadgetHasSprockets",
        "RulesEngineTest:Gadget", "Description", RelationshipMeaning::RelatedInstance));
        
    // get the query
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor);
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
    m_ruleset->GetContentModifierRules().back()->GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Backward,
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget", "Description", RelationshipMeaning::RelatedInstance));
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "RulesEngineTest:Sprocket", false);
    
    // get the query
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor);
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
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, 
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket", "Description", RelationshipMeaning::RelatedInstance));
    
    // get the query
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor);
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
    spec.GetRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, 
        "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget", "Description", RelationshipMeaning::RelatedInstance));
    spec.GetRelatedPropertiesR().back()->GetNestedRelatedPropertiesR().push_back(new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, 
        "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket", "Description", RelationshipMeaning::RelatedInstance));
    
    // get the query
    ContentDescriptorCPtr descriptor = GetBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor);
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
* @bsitest                                      Aidas.Vaiksnoras                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, NestsFilterExpressionQuery)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1,"", "RulesEngineTest:Widget", false);

    ContentDescriptorPtr descriptor = GetBuilder().CreateDescriptor(spec);
    descriptor->SetFilterExpression("Widget_MyID = \"WidgetId\"");
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("FilterExpressionQueryTest");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected->GetContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(query->GetContract()->GetDescriptor().AsJson());
    }
