/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentQueryBuilderTests_NestedContentField.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, NestedContentField_WithSingleStepRelationshipPath)
    {
    ECEntityClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();
    ECEntityClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    ECRelationshipClassCP rel = GetECClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    ContentDescriptor::Category category("name", "label", "", 1);
    ContentDescriptor::NestedContentField field(category, "field_name", "field_label", *sprocketClass, "sprocket", 
        {RelatedClass(*sprocketClass, *gadgetClass, *rel, false, "primary_instance", "rel")}, 
        {
        new ContentDescriptor::ECPropertiesField(*gadgetClass, ContentDescriptor::Property("rel_RET_Sprocket_0", *sprocketClass, *sprocketClass->GetPropertyP("MyID"))),
        new ContentDescriptor::ECPropertiesField(*gadgetClass, ContentDescriptor::Property("rel_RET_Sprocket_0", *sprocketClass, *sprocketClass->GetPropertyP("Description")))
        });

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(field);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("NestedContentField_WithSingleStepRelationshipPath");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, NestedContentField_WithMultiStepRelationshipPath)
    {
    ECEntityClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();
    ECEntityClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    ECEntityClassCP widgetClass = GetECClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    ECRelationshipClassCP relGS = GetECClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    ECRelationshipClassCP relWG = GetECClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    ContentDescriptor::Category category("name", "label", "", 1);
    RelatedClassPath path = {
        RelatedClass(*sprocketClass, *gadgetClass, *relGS, false, "intermediate", "rel_gs"), 
        RelatedClass(*gadgetClass, *widgetClass, *relWG, false, "primary_instance", "rel_wg")
        };
    ContentDescriptor::NestedContentField field(category, "field_name", "field_label", 
        *sprocketClass, "sprocket", path,
        {
        new ContentDescriptor::ECPropertiesField(*widgetClass, ContentDescriptor::Property("rel_RET_Sprocket_0", *sprocketClass, *sprocketClass->GetPropertyP("MyID"))),
        new ContentDescriptor::ECPropertiesField(*widgetClass, ContentDescriptor::Property("rel_RET_Sprocket_0", *sprocketClass, *sprocketClass->GetPropertyP("Description")))
        });

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(field);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("NestedContentField_WithMultiStepRelationshipPath");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, NestedContentField_WithNestedContentFields)
    {
    ECEntityClassCP sprocketClass = GetECClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();
    ECEntityClassCP gadgetClass = GetECClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    ECEntityClassCP widgetClass = GetECClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    ECRelationshipClassCP relGS = GetECClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();
    ECRelationshipClassCP relWG = GetECClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    ContentDescriptor::Category category("name", "label", "", 1);

    ContentDescriptor::NestedContentField field(category, "gadget_field_name", "gadget_field_label", 
        *gadgetClass, "gadget",
        {RelatedClass(*gadgetClass, *widgetClass, *relWG, false, "widget_instance", "rel_wg")},
        {
        new ContentDescriptor::ECPropertiesField(*gadgetClass, ContentDescriptor::Property("rel_RET_Gadget_0", *gadgetClass, *gadgetClass->GetPropertyP("MyID"))),
        new ContentDescriptor::ECPropertiesField(*gadgetClass, ContentDescriptor::Property("rel_RET_Gadget_0", *gadgetClass, *gadgetClass->GetPropertyP("Description"))),
        new ContentDescriptor::NestedContentField(category, "sprocket_field_name", "sprocket_field_label", 
            *sprocketClass, "sprocket",
            {RelatedClass(*sprocketClass, *gadgetClass, *relGS, false, "gadget_instance", "rel_gs")},
            {
            new ContentDescriptor::ECPropertiesField(*sprocketClass, ContentDescriptor::Property("sprocket_instance", *sprocketClass, *sprocketClass->GetPropertyP("MyID"))),
            })
        });

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(field);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("NestedContentField_WithNestedContentFields");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }
