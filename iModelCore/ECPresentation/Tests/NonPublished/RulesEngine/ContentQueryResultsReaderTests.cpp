/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "../../../Source/RulesDriven/RulesEngine/LocalizationHelper.h"
#include "../../../Source/RulesDriven/RulesEngine/ContentQueryResultsReader.h"
#include "../../../Localization/Xliffs/ECPresentation.xliff.h"
#include "QueryExecutorTests.h"
#include "ExpectedQueries.h"
#include "TestHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void VerifyItemInstance(ContentSetItemCR item, IECInstanceCR instance)
    {
    ASSERT_EQ(1, item.GetKeys().size());
    EXPECT_STREQ(instance.GetInstanceId().c_str(), item.GetKeys()[0].GetId().ToString().c_str());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct ContentQueryResultsReaderTests : QueryExecutorTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesUnionSelectionFromClassWithPointProperty)
    {
    ECEntityClassCP classH = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    ECEntityClassCP classE = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();

    IECInstancePtr h = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH);
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *classH, ContentDescriptor::Property("h", *classH, *classH->GetPropertyP("PointProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *classE, ContentDescriptor::Property("e", *classE, *classE->GetPropertyP("IntProperty")->GetAsPrimitiveProperty()));

    ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
    q1->SelectContract(*ContentQueryContract::Create(1, *descriptor, classH, *q1), "h");
    q1->From(*classH, false, "h");

    ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
    q2->SelectContract(*ContentQueryContract::Create(2, *descriptor, classE, *q2), "e");
    q2->From(*classE, false, "e");

    UnionContentQueryPtr query = UnionContentQuery::Create({q1, q2});

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);
    ContentReader reader(*query);

    ContentSetItemPtr item;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(item, reader));
    VerifyItemInstance(*item, *h);

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(item, reader));
    VerifyItemInstance(*item, *e);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(item, reader));
    EXPECT_TRUE(item.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2016
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

    Utf8String localizationId = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_Varies().m_str);
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, localizationId.c_str());

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    descriptor->AddContentFlag(ContentFlags::MergeResults);

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_gadgetClass, *query), "gadget");
    query->From(*m_gadgetClass, false, "gadget");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);
    ContentReader reader(*query);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "GadgetId",
        "%s": "%s"
    })",
        FIELD_NAME(m_gadgetClass, "MyID"), FIELD_NAME(m_gadgetClass, "Description"), formattedVariesStr.c_str()
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
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, HandlesResultsMergingFromMultipleClasses)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        instance.SetValue("Description", ECValue("Gadget description"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Widget"));
        instance.SetValue("Description", ECValue("Widget description"));
        });

    Utf8String localizationId = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_Varies().m_str);
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, localizationId.c_str());

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr innerDescriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*innerDescriptor, *m_gadgetClass, ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*innerDescriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    ContentDescriptor::Field* field = new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category("Misc.", "Misc.", "", 0), "Description", "Description");
    field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    field->SetUniqueName("Description");
    innerDescriptor->AddField(field);

    ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
    q1->SelectContract(*ContentQueryContract::Create(1, *innerDescriptor, m_gadgetClass, *q1), "gadget");
    q1->From(*m_gadgetClass, false, "gadget");

    ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
    q2->SelectContract(*ContentQueryContract::Create(2, *innerDescriptor, m_widgetClass, *q2), "widget");
    q2->From(*m_widgetClass, false, "widget");

    ContentDescriptorPtr outerDescriptor = ContentDescriptor::Create(*innerDescriptor);
    for (ContentDescriptor::Field* field : outerDescriptor->GetAllFields())
        {
        for (ContentDescriptor::Property const& fieldProperty : field->AsPropertiesField()->GetProperties())
            const_cast<ContentDescriptor::Property&>(fieldProperty).SetPrefix("");
        }
    outerDescriptor->AddContentFlag(ContentFlags::MergeResults);

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(0, *outerDescriptor, nullptr, *query));
    query->From(*UnionContentQuery::Create({q1, q2}));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);
    ContentReader reader(*query);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    ASSERT_TRUE(json["Values"].HasMember(FIELD_NAME(m_gadgetClass, "MyID")) && json["Values"][FIELD_NAME(m_gadgetClass, "MyID")].IsString());
    ASSERT_STREQ(formattedVariesStr.c_str(), json["Values"][FIELD_NAME(m_gadgetClass, "MyID")].GetString());
    ASSERT_TRUE(json["Values"].HasMember(FIELD_NAME(m_widgetClass, "MyID")) && json["Values"][FIELD_NAME(m_widgetClass, "MyID")].IsString());
    ASSERT_STREQ(formattedVariesStr.c_str(), json["Values"][FIELD_NAME(m_widgetClass, "MyID")].GetString());
    ASSERT_TRUE(json["Values"].HasMember("Description") && json["Values"]["Description"].IsString());
    ASSERT_STREQ(formattedVariesStr.c_str(), json["Values"]["Description"].GetString());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2016
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

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *classI, ContentDescriptor::Property("this", *classI, *classI->GetPropertyP("StringProperty")));
    AddField(*descriptor, *classI, ContentDescriptor::Property("this", *classI, *classI->GetPropertyP("StructProperty")));

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, classI, *query, bvector<RelatedClassPath>(), false), "this");
    query->From(*classI, false, "this");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);
    ContentReader reader(*query);

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
* @bsitest                                      Grigas.Petraitis                06/2017
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

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *classR, ContentDescriptor::Property("this", *classR, *classR->GetPropertyP("IntsArray")));
    AddField(*descriptor, *classR, ContentDescriptor::Property("this", *classR, *classR->GetPropertyP("StructsArray")));

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, classR, *query, bvector<RelatedClassPath>(), false), "this");
    query->From(*classR, false, "this");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);
    ContentReader reader(*query);

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
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, SelectsRelatedProperties)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Gadget"));});
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Widget"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget);

    RelatedClass relatedPropertyPath(*m_gadgetClass, *m_widgetClass, *widgetHasGadgetsRelationship, false, "rel_RET_Widget_0", "rel");

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("this", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("rel_RET_Widget_0", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    const_cast<ContentDescriptor::Property&>(descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back()).SetIsRelated(relatedPropertyPath, RelationshipMeaning::RelatedInstance);

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_gadgetClass, *query), "this");
    query->From(*m_gadgetClass, false, "this");
    query->Join(relatedPropertyPath);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);
    ContentReader reader(*query);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": "Test Gadget",
        "%s": "Test Widget"
    })",
        FIELD_NAME(m_gadgetClass, "MyID"), FIELD_NAME(m_widgetClass, "MyID")
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
* @bsitest                                      Grigas.Petraitis                11/2016
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

    RelatedClass relatedPropertyPath(classE, classD, classDHasClassERelationship, false, "rel_RET_ClassD_0", "rel");

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, classE, ContentDescriptor::Property("this", classE, *classE.GetPropertyP("IntProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, classE, ContentDescriptor::Property("rel_RET_ClassD_0", classD, *classD.GetPropertyP("StringProperty")->GetAsPrimitiveProperty()));
    const_cast<ContentDescriptor::Property&>(descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back()).SetIsRelated(relatedPropertyPath, RelationshipMeaning::RelatedInstance);

    ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
    query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &classE, *query1), "this");
    query1->From(classE, false, "this");
    query1->Join(relatedPropertyPath);

    ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
    query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &classF, *query2), "this");
    query2->From(classF, false, "this");

    UnionContentQueryPtr query = UnionContentQuery::Create({query1, query2});

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);
    ContentReader reader(*query);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    rapidjson::Document expectedValues;
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": 11,
        "%s": "D Property"
    })",
        FIELD_NAME(&classE, "IntProperty"), FIELD_NAME(&classD, "StringProperty")
    ).c_str());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    expectedValues.Parse(Utf8PrintfString(R"({
        "%s": 22,
        "%s": null
    })",
        FIELD_NAME(&classE, "IntProperty"), FIELD_NAME(&classD, "StringProperty")
    ).c_str());
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
* @bsitest                                      Grigas.Petraitis                09/2016
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

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("IntProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("BoolProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("DoubleProperty")->GetAsPrimitiveProperty()));

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query), "widget");
    query->From(*m_widgetClass, false, "widget");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    QueryExecutor executor(*m_connection, *query);
    ContentReader reader(*query);
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
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryResultsReaderTests, DoesntIncludeFieldPropertyValueInstanceKeysWhenDescriptorContainsExcludeEditingDataFlag)
    {
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor->SetContentFlags(descriptor->GetContentFlags() | (int)ContentFlags::ExcludeEditingData);
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query), "widget");
    query->From(*m_widgetClass, false, "widget");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    QueryExecutor executor(*m_connection, *query);
    ContentReader reader(*query);

    ContentSetItemPtr record;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(record, reader));
    ASSERT_TRUE(record.IsValid());
    EXPECT_TRUE(record->GetFieldInstanceKeys().empty());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(record, reader));
    EXPECT_TRUE(record.IsNull());
    }
