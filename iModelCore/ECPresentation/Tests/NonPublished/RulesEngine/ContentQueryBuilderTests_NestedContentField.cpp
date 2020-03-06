/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, RelatedContentField_WithSingleStepRelationshipPath)
    {
    ECEntityClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();
    ECEntityClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    ECRelationshipClassCP rel = GetECClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    ContentDescriptor::Category category("name", "label", "", 1);
    ContentDescriptor::RelatedContentField field(category, "field_name", "field_label",
        {RelatedClass(*gadgetClass, *sprocketClass, *rel, true, "sprocket", "rel")},
        {
        new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("sprocket", *sprocketClass, *sprocketClass->GetPropertyP("MyID"))),
        new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("sprocket", *sprocketClass, *sprocketClass->GetPropertyP("Description")))
        });

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(field);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery(BeTest::GetNameOfCurrentTest());
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, RelatedContentField_WithMultiStepRelationshipPath)
    {
    ECEntityClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();
    ECEntityClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    ECEntityClassCP widgetClass = GetECClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    ECRelationshipClassCP relGS = GetECClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    ECRelationshipClassCP relWG = GetECClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    ContentDescriptor::Category category("name", "label", "", 1);
    ContentDescriptor::RelatedContentField field(category, "field_name", "field_label",
        {
        RelatedClass(*widgetClass, *gadgetClass, *relWG, true, "source", "rel_wg"),
        RelatedClass(*gadgetClass, *sprocketClass, *relGS, true, "sprocket", "rel_gs"),
        },
        {
        new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("rel_RET_Sprocket_0", *sprocketClass, *sprocketClass->GetPropertyP("MyID"))),
        new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("rel_RET_Sprocket_0", *sprocketClass, *sprocketClass->GetPropertyP("Description")))
        });

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(field);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery(BeTest::GetNameOfCurrentTest());
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, RelatedContentField_WithNestedContentFields)
    {
    ECEntityClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();
    ECEntityClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    ECEntityClassCP widgetClass = GetECClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    ECRelationshipClassCP relGS = GetECClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    ECRelationshipClassCP relWG = GetECClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    ContentDescriptor::Category category("name", "label", "", 1);

    ContentDescriptor::RelatedContentField field(category, "gadget_field_name", "gadget_field_label",
        {RelatedClass(*widgetClass, SelectClass(*gadgetClass, false), *relWG, true, "gadget", "rel_wg")},
        {
        new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("rel_RET_Gadget_0", *gadgetClass, *gadgetClass->GetPropertyP("MyID"))),
        new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category::GetDefaultCategory(), ContentDescriptor::Property("rel_RET_Gadget_0", *gadgetClass, *gadgetClass->GetPropertyP("Description"))),
        new ContentDescriptor::RelatedContentField(category, "sprocket_field_name", "sprocket_field_label",
            {RelatedClass(*gadgetClass, SelectClass(*sprocketClass, false), *relGS, true, "sprocket_instance", "rel_gs")},
            {
            new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category(), ContentDescriptor::Property("sprocket_instance", *sprocketClass, *sprocketClass->GetPropertyP("MyID"))),
            })
        });

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(field);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery(BeTest::GetNameOfCurrentTest());
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }
