/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../../Source/Content/ContentQueryContracts.h"
#include "../../../../Source/Content/ContentQueryResultsReader.h"
#include "../Queries/QueryExecutorTests.h"
#include "../../Helpers/TestHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
static void VerifyItemInstance(ContentSetItemCR item, IECInstanceCR instance)
    {
    ASSERT_EQ(1, item.GetKeys().size());
    EXPECT_STREQ(instance.GetInstanceId().c_str(), item.GetKeys()[0].GetId().ToString().c_str());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentQueryResultsReaderTests : QueryExecutorTests
    {
    ContentDescriptorPtr CreateContentDescriptor() const
        {
        return ContentDescriptor::Create(*m_connection, *m_ruleset, RulesetVariables(), *NavNodeKeyListContainer::Create());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesUnionSelectionFromClassWithPointProperty)
    {
    ECEntityClassCP classH = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    ECEntityClassCP classE = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();

    IECInstancePtr h = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH);
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    AddField(*descriptor, *classH, ContentDescriptor::Property("h", *classH, *classH->GetPropertyP("PointProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *classE, ContentDescriptor::Property("e", *classE, *classE->GetPropertyP("IntProperty")->GetAsPrimitiveProperty()));

    SelectClass<ECClass> selectClass1(*classH, "h", false);
    ComplexQueryBuilderPtr q1 = ComplexQueryBuilder::Create();
    q1->SelectContract(*ContentQueryContract::Create(1, *descriptor, classH, *q1, nullptr, {}, false, false), "h");
    q1->From(selectClass1);

    SelectClass<ECClass> selectClass2(*classE, "e", false);
    ComplexQueryBuilderPtr q2 = ComplexQueryBuilder::Create();
    q2->SelectContract(*ContentQueryContract::Create(2, *descriptor, classE, *q2, nullptr, {}, false, false), "e");
    q2->From(selectClass2);

    UnionQueryBuilderPtr query = UnionQueryBuilder::Create({q1, q2});

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr item;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(item, reader));
    VerifyItemInstance(*item, *h);

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(item, reader));
    VerifyItemInstance(*item, *e);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(item, reader));
    EXPECT_TRUE(item.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingFromOneClass)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("GadgetId"));
        instance.SetValue("Description", ECValue("Gadget 1"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("GadgetId"));
        instance.SetValue("Description", ECValue("Gadget 2"));
        });

    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    descriptor->AddContentFlag(ContentFlags::MergeResults);

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_gadgetClass, *query, nullptr, {}, false, false), "gadget");
    query->From(*m_gadgetClass, false, "gadget");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document json = record->AsJson();

    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "GadgetId",
        "%s": null
    })",
        FIELD_NAME(m_gadgetClass, "MyID"), FIELD_NAME(m_gadgetClass, "Description")
    ).c_str());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"({
        "%s": "GadgetId",
        "%s": "%s"
    })",
        FIELD_NAME(m_gadgetClass, "MyID"), FIELD_NAME(m_gadgetClass, "Description"), formattedVariesStr.c_str()
    ).c_str());
    EXPECT_EQ(expectedDisplayValues, json["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["DisplayValues"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingFromMultipleClasses)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        instance.SetValue("Description", ECValue("Description"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Widget"));
        instance.SetValue("Description", ECValue("Description"));
        });

    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ContentDescriptorPtr innerDescriptor = CreateContentDescriptor();
    ContentDescriptor::Field* f1 = new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreateDefaultCategory(), "MyID", "MyID");
    f1->AsPropertiesField()->AddProperty(ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    f1->AsPropertiesField()->AddProperty(ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    innerDescriptor->AddRootField(*f1);
    ContentDescriptor::Field* f2 = new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreateDefaultCategory(), "Description", "Description");
    f2->AsPropertiesField()->AddProperty(ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    f2->AsPropertiesField()->AddProperty(ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    innerDescriptor->AddRootField(*f2);

    ComplexQueryBuilderPtr q1 = ComplexQueryBuilder::Create();
    q1->SelectContract(*ContentQueryContract::Create(1, *innerDescriptor, m_gadgetClass, *q1, nullptr, {}, false, false), "gadget");
    q1->From(*m_gadgetClass, false, "gadget");

    ComplexQueryBuilderPtr q2 = ComplexQueryBuilder::Create();
    q2->SelectContract(*ContentQueryContract::Create(2, *innerDescriptor, m_widgetClass, *q2, nullptr, {}, false, false), "widget");
    q2->From(*m_widgetClass, false, "widget");

    ContentDescriptorPtr outerDescriptor = ContentDescriptor::Create(*innerDescriptor);
    for (ContentDescriptor::Field* field : outerDescriptor->GetAllFields())
        {
        for (ContentDescriptor::Property const& fieldProperty : field->AsPropertiesField()->GetProperties())
            const_cast<ContentDescriptor::Property&>(fieldProperty).SetPrefix("");
        }
    outerDescriptor->AddContentFlag(ContentFlags::MergeResults);

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(0, *outerDescriptor, nullptr, *query, nullptr, {}, false, false));
    query->From(*UnionQueryBuilder::Create({q1, q2}));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document json = record->AsJson();

    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "MyID": null,
        "Description": "Description"
    })").c_str());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"({
        "MyID": "%s",
        "Description": "Description"
    })",
        formattedVariesStr.c_str()
    ).c_str());
    EXPECT_EQ(expectedDisplayValues, json["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["DisplayValues"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesStructProperties)
    {
    ECEntityClassCP classI = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassI")->GetEntityClassCP();
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classI, [](IECInstanceR instance)
        {
        instance.SetValue("StringProperty", ECValue("1"));
        instance.SetValue("StructProperty.IntProperty", ECValue(2));
        instance.SetValue("StructProperty.StructProperty.IntProperty", ECValue(3));
        instance.SetValue("StructProperty.StructProperty.StringProperty", ECValue("4"));
        });

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    AddField(*descriptor, *classI, ContentDescriptor::Property("this", *classI, *classI->GetPropertyP("StringProperty")));
    AddField(*descriptor, *classI, ContentDescriptor::Property("this", *classI, *classI->GetPropertyP("StructProperty")));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, classI, *query, nullptr, {}, false), "this");
    query->From(*classI, false, "this");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "1",
        "%s": {
           "IntProperty": 2,
           "StructProperty": {
               "IntProperty": 3,
               "StringProperty": "4"
            }
        }
    })",
        FIELD_NAME(classI, "StringProperty"), FIELD_NAME(classI, "StructProperty")
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesArrayProperties)
    {
    ECClassCP classStruct1 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Struct1");
    ECEntityClassCP classR = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassR")->GetEntityClassCP();
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classR, [&classStruct1](IECInstanceR instance)
        {
        instance.AddArrayElements("IntsArray", 2);
        instance.SetValue("IntsArray", ECValue(2), 0);
        instance.SetValue("IntsArray", ECValue(1), 1);

        instance.AddArrayElements("StructsArray", 2);

        IECInstancePtr struct1 = classStruct1->GetDefaultStandaloneEnabler()->CreateInstance();
        struct1->SetValue("StringProperty", ECValue("a"));
        ECValue structValue1;
        structValue1.SetStruct(struct1.get());
        instance.SetValue("StructsArray", structValue1, 0);

        IECInstancePtr struct2 = classStruct1->GetDefaultStandaloneEnabler()->CreateInstance();
        struct2->SetValue("StringProperty", ECValue("b"));
        ECValue structValue2;
        structValue2.SetStruct(struct2.get());
        instance.SetValue("StructsArray", structValue2, 1);
        });

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    AddField(*descriptor, *classR, ContentDescriptor::Property("this", *classR, *classR->GetPropertyP("IntsArray")));
    AddField(*descriptor, *classR, ContentDescriptor::Property("this", *classR, *classR->GetPropertyP("StructsArray")));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, classR, *query, nullptr, {}, false), "this");
    query->From(*classR, false, "this");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": [2, 1],
        "%s": [{
           "IntProperty": null,
           "StringProperty": "a"
        },{
           "IntProperty": null,
           "StringProperty": "b"
       }]
    })",
        FIELD_NAME(classR, "IntsArray"), FIELD_NAME(classR, "StructsArray")
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, SelectsRelatedProperties)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Gadget"));});
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Widget"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget);

    RelatedClass relatedPropertyPath(*m_gadgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "rel"), false, SelectClass<ECClass>(*m_widgetClass, "rel_RET_Widget_0"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "widget_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
            ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_gadgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_gadgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "gadget_MyID_unique_property_name": "Test Gadget",
        "widget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "widget_MyID_unique_property_name": "Test Widget"
                },
            "DisplayValues": {
                "widget_MyID_unique_property_name": "Test Widget"
                },
            "MergedFieldNames": []
        }]
    })",
        m_widgetClass->GetId().ToString().c_str(), widget->GetInstanceId().c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingOfRelatedProperties)
    {
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 1")); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 2")); });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget2);

    RelatedClass relatedPropertyPath(*m_gadgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r"), false, SelectClass<ECClass>(*m_widgetClass, "w"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddContentFlag(ContentFlags::MergeResults);
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "widget_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
            ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_gadgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_gadgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "gadget_MyID_unique_property_name": "Test Gadget",
        "widget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}, {"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "widget_MyID_unique_property_name": null
                },
            "DisplayValues": {
                "widget_MyID_unique_property_name": "%s"
                },
            "MergedFieldNames": ["widget_MyID_unique_property_name"]
        }]
    })",
        m_widgetClass->GetId().ToString().c_str(), widget1->GetInstanceId().c_str(),
        m_widgetClass->GetId().ToString().c_str(), widget2->GetInstanceId().c_str(),
        formattedVariesStr.c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingOfRelatedPropertiesOfTheSameRelatedInstance)
    {
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget")); });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget2);

    RelatedClass relatedPropertyPath(*m_gadgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r"), false, SelectClass<ECClass>(*m_widgetClass, "w"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddContentFlag(ContentFlags::MergeResults);
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "widget_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
        ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_gadgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_gadgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "gadget_MyID_unique_property_name": "Test Gadget",
        "widget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "widget_MyID_unique_property_name": "Test Widget"
                },
            "DisplayValues": {
                "widget_MyID_unique_property_name": "Test Widget"
                },
            "MergedFieldNames": []
        }]
    })",
        m_widgetClass->GetId().ToString().c_str(), widget->GetInstanceId().c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, SelectsOneToManyRelatedProperties)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget")); });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 1")); });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 2")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget2);

    RelatedClass relatedPropertyPath(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
            ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": "Test Widget",
        "gadget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": "Test Gadget 1"
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "Test Gadget 1"
                },
            "MergedFieldNames": []
        }, {
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": "Test Gadget 2"
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "Test Gadget 2"
                },
            "MergedFieldNames": []
        }]
    })",
        m_gadgetClass->GetId().ToString().c_str(), gadget1->GetInstanceId().c_str(),
        m_gadgetClass->GetId().ToString().c_str(), gadget2->GetInstanceId().c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingOfOneToManyRelatedPropertiesWhenThereIsOneRelatedItemAndValuesEqual)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 1")); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 2")); });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget2);

    RelatedClass relatedPropertyPath(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddContentFlag(ContentFlags::MergeResults);
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
            ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": null,
        "gadget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}, {"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": "Test Gadget"
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "Test Gadget"
                },
            "MergedFieldNames": []
        }]
    })",
        m_gadgetClass->GetId().ToString().c_str(), gadget1->GetInstanceId().c_str(),
        m_gadgetClass->GetId().ToString().c_str(), gadget2->GetInstanceId().c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingOfOneToManyRelatedPropertiesWhenThereIsOneRelatedItemAndValuesDifferent)
    {
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 1")); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 2")); });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 1")); });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 2")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget2);

    RelatedClass relatedPropertyPath(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddContentFlag(ContentFlags::MergeResults);
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
        ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": null,
        "gadget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}, {"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": null
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "%s"
                },
            "MergedFieldNames": ["gadget_MyID_unique_property_name"]
        }]
    })",
        m_gadgetClass->GetId().ToString().c_str(), gadget1->GetInstanceId().c_str(),
        m_gadgetClass->GetId().ToString().c_str(), gadget2->GetInstanceId().c_str(),
        formattedVariesStr.c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingOfOneToManyRelatedPropertiesWhenValuesEqualAndNumberOfRelatedItemsIsEqualButLargerThanOne)
    {
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 1")); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 2")); });
    IECInstancePtr gadget11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget11);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget12);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget21);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget22);

    RelatedClass relatedPropertyPath(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddContentFlag(ContentFlags::MergeResults);
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
        ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document json = record->AsJson();

    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": null,
        "gadget_related_content_unique_field_name": null
    })").c_str());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": "%s",
        "gadget_related_content_unique_field_name": "%s"
    })",
        formattedVariesStr.c_str(), formattedVariesStr.c_str()
    ).c_str());
    EXPECT_EQ(expectedDisplayValues, json["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["DisplayValues"]);

    EXPECT_TRUE(record->IsMerged("gadget_related_content_unique_field_name"));

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingOfOneToManyRelatedPropertiesWhenSizesDifferentAndIncrease)
    {
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 1")); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 2")); });
    IECInstancePtr gadget11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget11);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget21);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget22);

    RelatedClass relatedPropertyPath(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddContentFlag(ContentFlags::MergeResults);
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
            ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document json = record->AsJson();

    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": null,
        "gadget_related_content_unique_field_name": null
    })").c_str());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": "%s",
        "gadget_related_content_unique_field_name": "%s"
    })",
        formattedVariesStr.c_str(), formattedVariesStr.c_str()
    ).c_str());
    EXPECT_EQ(expectedDisplayValues, json["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["DisplayValues"]);

    EXPECT_TRUE(record->IsMerged("gadget_related_content_unique_field_name"));

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingOfOneToManyRelatedPropertiesWhenSizesDifferentAndDecrease)
    {
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 1")); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 2")); });
    IECInstancePtr gadget11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    IECInstancePtr gadget22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget11);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget12);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget22);

    RelatedClass relatedPropertyPath(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddContentFlag(ContentFlags::MergeResults);
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
            ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document json = record->AsJson();

    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": null,
        "gadget_related_content_unique_field_name": null
    })").c_str());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": "%s",
        "gadget_related_content_unique_field_name": "%s"
    })",
        formattedVariesStr.c_str(), formattedVariesStr.c_str()
    ).c_str());
    EXPECT_EQ(expectedDisplayValues, json["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["DisplayValues"]);

    EXPECT_TRUE(record->IsMerged("gadget_related_content_unique_field_name"));

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, SelectsOneToManyRelatedProperties_DeepTreeCase)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP gadgetHasSprocketsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget")); });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 1")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget1);
    IECInstancePtr sprocket11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Sprocket 1.1")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprocketsRelationship, *gadget1, *sprocket11);
    IECInstancePtr sprocket12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Sprocket 1.2")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprocketsRelationship, *gadget1, *sprocket12);

    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 2")); });
    IECInstancePtr sprocket21 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Sprocket 2.1")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprocketsRelationship, *gadget2, *sprocket21);
    IECInstancePtr sprocket22 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Sprocket 2.2")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprocketsRelationship, *gadget2, *sprocket22);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget2);

    RelatedClass relWG(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r1"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));
    RelatedClass relGS(*m_gadgetClass, SelectClass<ECRelationshipClass>(*gadgetHasSprocketsRelationship, "r2"), true, SelectClass<ECClass>(*m_sprocketClass, "s"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content 1", {relWG},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
            ContentDescriptor::Property(relWG.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "sprocket_related_content_unique_field_name", "related content 2", {relGS},
            {
            new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_sprocketClass->GetPropertyP("MyID")), "sprocket_MyID_unique_property_name",
                ContentDescriptor::Property(relGS.GetTargetClass().GetAlias(), *m_sprocketClass, *m_sprocketClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
            })
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join({relWG, relGS});

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": "Test Widget",
        "gadget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": "Test Gadget 1",
                "sprocket_related_content_unique_field_name": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 1.1"
                        },
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 1.1"
                        },
                    "MergedFieldNames": []
                    }, {
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 1.2"
                        },
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 1.2"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "Test Gadget 1",
                "sprocket_related_content_unique_field_name": [{
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 1.1"
                        }
                    }, {
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 1.2"
                        }
                    }]
                },
            "MergedFieldNames": []
        }, {
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": "Test Gadget 2",
                "sprocket_related_content_unique_field_name": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 2.1"
                        },
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 2.1"
                        },
                    "MergedFieldNames": []
                    }, {
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 2.2"
                        },
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 2.2"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "Test Gadget 2",
                "sprocket_related_content_unique_field_name": [{
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 2.1"
                        }
                    }, {
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket 2.2"
                        }
                    }]
                },
            "MergedFieldNames": []
        }]
    })",
        m_gadgetClass->GetId().ToString().c_str(), gadget1->GetInstanceId().c_str(),
        m_sprocketClass->GetId().ToString().c_str(), sprocket11->GetInstanceId().c_str(), m_sprocketClass->GetId().ToString().c_str(), sprocket12->GetInstanceId().c_str(),
        m_gadgetClass->GetId().ToString().c_str(), gadget2->GetInstanceId().c_str(),
        m_sprocketClass->GetId().ToString().c_str(), sprocket21->GetInstanceId().c_str(), m_sprocketClass->GetId().ToString().c_str(), sprocket22->GetInstanceId().c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingOfOneToManyRelatedProperties_DeepTreeCase)
    {
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP gadgetHasSprocketsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget")); });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 1")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget1);
    IECInstancePtr sprocket1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Sprocket")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprocketsRelationship, *gadget1, *sprocket1);

    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget")); });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 2")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget2);
    IECInstancePtr sprocket2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Sprocket")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprocketsRelationship, *gadget2, *sprocket2);

    RelatedClass relWG(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r1"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));
    RelatedClass relGS(*m_gadgetClass, SelectClass<ECRelationshipClass>(*gadgetHasSprocketsRelationship, "r2"), true, SelectClass<ECClass>(*m_sprocketClass, "s"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddContentFlag(ContentFlags::MergeResults);
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content 1", {relWG},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
            ContentDescriptor::Property(relWG.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "sprocket_related_content_unique_field_name", "related content 2", {relGS},
            {
            new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_sprocketClass->GetPropertyP("MyID")), "sprocket_MyID_unique_property_name",
                ContentDescriptor::Property(relGS.GetTargetClass().GetAlias(), *m_sprocketClass, *m_sprocketClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
            })
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join({relWG, relGS});

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name": "Test Widget",
        "gadget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}, {"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": null,
                "sprocket_related_content_unique_field_name": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}, {"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket"
                        },
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "%s",
                "sprocket_related_content_unique_field_name": [{
                    "DisplayValues": {
                        "sprocket_MyID_unique_property_name": "Test Sprocket"
                        }
                    }]
                },
            "MergedFieldNames": ["gadget_MyID_unique_property_name"]
        }]
    })",
        m_gadgetClass->GetId().ToString().c_str(), gadget1->GetInstanceId().c_str(), m_gadgetClass->GetId().ToString().c_str(), gadget2->GetInstanceId().c_str(),
        m_sprocketClass->GetId().ToString().c_str(), sprocket1->GetInstanceId().c_str(),
        m_sprocketClass->GetId().ToString().c_str(), sprocket2->GetInstanceId().c_str(),
        formattedVariesStr.c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, SelectsOneToManyRelatedProperties_DiamondCase)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP widgetsHaveGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetsHaveGadgets")->GetRelationshipClassCP();

    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 1")); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 2")); });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 1")); });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 2")); });

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetsHaveGadgetsRelationship, *widget2, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetsHaveGadgetsRelationship, *widget2, *gadget2);

    RelatedClass relWG(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r1"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));
    RelatedClass relGW(*m_gadgetClass, SelectClass<ECRelationshipClass>(*widgetsHaveGadgetsRelationship, "r2"), false, SelectClass<ECClass>(*m_widgetClass, "w"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name_1",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content 1", {relWG},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
            ContentDescriptor::Property(relWG.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "widget_related_content_unique_field_name", "related content 2", {relGW},
            {
            new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name_2",
                ContentDescriptor::Property(relGW.GetTargetClass().GetAlias(), *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
            })
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join({relWG, relGW});
    query->Where("this.ECInstanceId = ?", {std::make_shared<BoundQueryId>(widget1->GetInstanceId())});

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name_1": "Test Widget 1",
        "gadget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": "Test Gadget 1",
                "widget_related_content_unique_field_name": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        },
                    "DisplayValues": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "Test Gadget 1",
                "widget_related_content_unique_field_name": [{
                    "DisplayValues": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        }
                    }]
                },
            "MergedFieldNames": []
        }, {
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": "Test Gadget 2",
                "widget_related_content_unique_field_name": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        },
                    "DisplayValues": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "Test Gadget 2",
                "widget_related_content_unique_field_name": [{
                    "DisplayValues": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        }
                    }]
                },
            "MergedFieldNames": []
        }]
    })",
        m_gadgetClass->GetId().ToString().c_str(), gadget1->GetInstanceId().c_str(),
        m_widgetClass->GetId().ToString().c_str(), widget2->GetInstanceId().c_str(),
        m_gadgetClass->GetId().ToString().c_str(), gadget2->GetInstanceId().c_str(),
        m_widgetClass->GetId().ToString().c_str(), widget2->GetInstanceId().c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingOfOneToManyRelatedProperties_DiamondCase)
    {
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP widgetsHaveGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetsHaveGadgets")->GetRelationshipClassCP();

    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 1")); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Widget 2")); });
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 1")); });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance) {instance.SetValue("MyID", ECValue("Test Gadget 2")); });

    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetsHaveGadgetsRelationship, *widget2, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetsHaveGadgetsRelationship, *widget2, *gadget2);

    RelatedClass relWG(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r1"), true, SelectClass<ECClass>(*m_gadgetClass, "g"));
    RelatedClass relGW(*m_gadgetClass, SelectClass<ECRelationshipClass>(*widgetsHaveGadgetsRelationship, "r2"), false, SelectClass<ECClass>(*m_widgetClass, "w"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddContentFlag(ContentFlags::MergeResults);
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name_1",
        ContentDescriptor::Property("this", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "gadget_related_content_unique_field_name", "related content 1", {relWG},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_gadgetClass->GetPropertyP("MyID")), "gadget_MyID_unique_property_name",
            ContentDescriptor::Property(relWG.GetTargetClass().GetAlias(), *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
        new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "widget_related_content_unique_field_name", "related content 2", {relGW},
            {
            new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*m_widgetClass->GetPropertyP("MyID")), "widget_MyID_unique_property_name_2",
                ContentDescriptor::Property(relGW.GetTargetClass().GetAlias(), *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty())),
            })
        }));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "this");
    query->From(*m_widgetClass, false, "this");
    query->Join({relWG, relGW});
    query->Where("this.ECInstanceId = ?", {std::make_shared<BoundQueryId>(widget1->GetInstanceId())});

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "widget_MyID_unique_property_name_1": "Test Widget 1",
        "gadget_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": "Test Gadget 1",
                "widget_related_content_unique_field_name": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        },
                    "DisplayValues": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "Test Gadget 1",
                "widget_related_content_unique_field_name": [{
                    "DisplayValues": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        }
                    }]
                },
            "MergedFieldNames": []
        }, {
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "gadget_MyID_unique_property_name": "Test Gadget 2",
                "widget_related_content_unique_field_name": [{
                    "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
                    "Values": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        },
                    "DisplayValues": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        },
                    "MergedFieldNames": []
                    }]
                },
            "DisplayValues": {
                "gadget_MyID_unique_property_name": "Test Gadget 2",
                "widget_related_content_unique_field_name": [{
                    "DisplayValues": {
                        "widget_MyID_unique_property_name_2": "Test Widget 2"
                        }
                    }]
                },
            "MergedFieldNames": []
        }]
    })",
        m_gadgetClass->GetId().ToString().c_str(), gadget1->GetInstanceId().c_str(),
        m_widgetClass->GetId().ToString().c_str(), widget2->GetInstanceId().c_str(),
        m_gadgetClass->GetId().ToString().c_str(), gadget2->GetInstanceId().c_str(),
        m_widgetClass->GetId().ToString().c_str(), widget2->GetInstanceId().c_str(),
        formattedVariesStr.c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, SelectsRelatedPropertiesFromOnlySingleClassWhenSelectingFromMultipleClasses)
    {
    ECEntityClassCR classD = *s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassD")->GetEntityClassCP();
    ECEntityClassCR classE = *s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCR classF = *s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    ECRelationshipClassCR classDHasClassERelationship = *s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();

    IECInstancePtr dInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), classD, [](IECInstanceR instance){instance.SetValue("StringProperty", ECValue("D Property"));});
    IECInstancePtr eInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), classE, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(11));});
    IECInstancePtr fInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), classF, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(22));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), classDHasClassERelationship, *dInstance, *eInstance);

    RelatedClass relatedPropertyPath(classE, SelectClass<ECRelationshipClass>(classDHasClassERelationship, "rel"), false, SelectClass<ECClass>(classD, "rel_RET_ClassD_0"));

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->AddRootField(*new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*classE.GetPropertyP("IntProperty")), "ClassE_IntProperty_unique_property_name",
        ContentDescriptor::Property("this", classE, *classE.GetPropertyP("IntProperty")->GetAsPrimitiveProperty())));
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(m_categorySupplier.CreateDefaultCategory(), "classD_related_content_unique_field_name", "related content", {relatedPropertyPath},
        {
        new ContentDescriptor::ECPropertiesField(m_categorySupplier.CreatePropertyCategory(*classD.GetPropertyP("StringProperty")), "classD_StringProperty_unique_property_name",
            ContentDescriptor::Property(relatedPropertyPath.GetTargetClass().GetAlias(), classD, *classD.GetPropertyP("StringProperty")->GetAsPrimitiveProperty())),
        }));

    ComplexQueryBuilderPtr query1 = ComplexQueryBuilder::Create();
    query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &classE, *query1, nullptr), "this");
    query1->From(classE, false, "this");
    query1->Join(relatedPropertyPath);

    ComplexQueryBuilderPtr query2 = ComplexQueryBuilder::Create();
    query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &classF, *query2, nullptr), "this");
    query2->From(classF, false, "this");

    UnionQueryBuilderPtr query = UnionQueryBuilder::Create({query1, query2});

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "ClassE_IntProperty_unique_property_name": 11,
        "classD_related_content_unique_field_name": [{
            "PrimaryKeys": [{"ECClassId": "%s", "ECInstanceId": "%s"}],
            "Values": {
                "classD_StringProperty_unique_property_name": "D Property"
                },
            "DisplayValues": {
                "classD_StringProperty_unique_property_name": "D Property"
                },
            "MergedFieldNames": []
        }]
    })",
        classD.GetId().ToString().c_str(), dInstance->GetInstanceId().c_str()
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    expectedValues.Parse(Utf8PrintfString(R"({
        "ClassE_IntProperty_unique_property_name": 22,
        "classD_related_content_unique_field_name": []
    })").c_str());
    json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, UsesSuppliedECPropertyFormatterToFormatPrimitiveECPropertyValues)
    {
    StubPropertyFormatter formatter;
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Test 1"));
        instance.SetValue("Description", ECValue("Test 2"));
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("BoolProperty", ECValue(true));
        instance.SetValue("DoubleProperty", ECValue(4.0));
        });

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("IntProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("BoolProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("DoubleProperty")->GetAsPrimitiveProperty()));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "widget");
    query->From(*m_widgetClass, false, "widget");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);
    reader.SetPropertyFormatter(formatter);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(Utf8PrintfString(R"({
        "%s": "_Test 1_",
        "%s": "_Test 2_",
        "%s": "_3_",
        "%s": "_True_",
        "%s": "_4_"
    })",
        FIELD_NAME(m_widgetClass, "MyID"), FIELD_NAME(m_widgetClass, "Description"), FIELD_NAME(m_widgetClass, "IntProperty"),
        FIELD_NAME(m_widgetClass, "BoolProperty"), FIELD_NAME(m_widgetClass, "DoubleProperty")
    ).c_str());
    rapidjson::Document json = record->AsJson();
    EXPECT_EQ(expectedDisplayValues, json["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["DisplayValues"]);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, DoesntIncludeFieldPropertyValueInstanceKeysWhenDescriptorContainsExcludeEditingDataFlag)
    {
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    ContentDescriptorPtr descriptor = CreateContentDescriptor();
    descriptor->SetContentFlags(descriptor->GetContentFlags() | (int)ContentFlags::ExcludeEditingData);
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query, nullptr, {}, false, false), "widget");
    query->From(*m_widgetClass, false, "widget");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    ContentQueryContractsFilter contracts(*query);
    QueryExecutor executor(*m_connection, *query->GetQuery());
    ContentReader reader(s_project->GetECDb().Schemas(), contracts);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    ASSERT_TRUE(record.IsValid());
    EXPECT_TRUE(record->GetFieldInstanceKeys().empty());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }
