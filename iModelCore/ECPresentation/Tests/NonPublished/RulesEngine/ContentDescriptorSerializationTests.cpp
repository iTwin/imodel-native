/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentDescriptorSerializationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "../../../Source/RulesDriven/RulesEngine/ContentProviders.h"
#include "ECDbTestProject.h"
#include "TestHelpers.h"

#define TEST_SCHEMA "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                            \
            "<ECSchema schemaName=\"TestSchema\" alias=\"test\" version=\"9.3\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"                \
            "    <ECEnumeration typeName=\"TestEnum\" backingTypeName=\"int\" isStrict=\"True\" description=\"\" displayLabel=\"IntegerEnum\">"             \
            "        <ECEnumerator value=\"1\" displayLabel=\"A\" />"                                                                                       \
            "        <ECEnumerator value=\"2\" displayLabel=\"B\" />"                                                                                       \
            "        <ECEnumerator value=\"3\" displayLabel=\"C\" />"                                                                                       \
            "    </ECEnumeration>"                                                                                                                          \
            "    <KindOfQuantity typeName=\"TestKOQ\" displayLabel=\"Test\" persistenceUnit=\"MM\" relativeError=\"0\" presentationUnits=\"MM;CM\"/>"       \
            "    <ECEntityClass typeName=\"ClassA\">"                                                                                                       \
            "        <ECProperty propertyName=\"String\" typeName=\"string\" />"                                                                            \
            "        <ECProperty propertyName=\"EnumProperty\" typeName=\"TestEnum\" />"                                                                    \
            "        <ECProperty propertyName=\"KOQProperty\" typeName=\"int\" kindOfQuantity=\"TestKOQ\" />"                                               \
            "    </ECEntityClass>"                                                                                                                          \
            "</ECSchema>"

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                09/2017
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorSerializationTests : ::testing::Test
{
    static ECDbTestProject* s_project;
    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("ContentDescriptorAsJsonTests");

        ECSchemaPtr schema;
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
        EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, TEST_SCHEMA, *schemaContext));

        BentleyStatus importStatus = s_project->GetECDb().Schemas().ImportSchemas({ schema.get() });
        BeAssert(SUCCESS == importStatus);

        BeSQLite::DbResult result = s_project->GetECDb().SaveChanges();
        BeAssert(BeSQLite::DbResult::BE_SQLITE_OK == result);
        }
    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }
};
ECDbTestProject* ContentDescriptorSerializationTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentDescriptorSerializationTests, ECPropertiesFieldWithSimplePrimitiveProperty)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass("TestSchema", "ClassA");

    ContentDescriptorPtr descriptor = ContentDescriptor::Create();
    descriptor->GetAllFields().push_back(new ContentDescriptor::ECPropertiesField(*classA, ContentDescriptor::Property("this", *classA, *classA->GetPropertyP("String"))));

    rapidjson::Document doc = descriptor->AsJson();
    ASSERT_TRUE(doc.HasMember("Fields"));
    ASSERT_TRUE(doc["Fields"].IsArray());
    ASSERT_EQ(1, doc["Fields"].GetArray().Size());
    RapidJsonValueCR field = doc["Fields"][0];

    EXPECT_STREQ("ClassA_String",field["Name"].GetString());
    EXPECT_STREQ("String", field["DisplayLabel"].GetString());
    EXPECT_STREQ("", field["Editor"].GetString());
    EXPECT_EQ(0, field["Priority"].GetInt());
    EXPECT_STREQ("string", field["Type"].GetString());
    EXPECT_FALSE(field["IsReadOnly"].GetBool());

    ASSERT_TRUE(field.HasMember("Properties"));
    ASSERT_TRUE(field["Properties"].IsArray());
    ASSERT_EQ(1, field["Properties"].GetArray().Size());
    ASSERT_TRUE(field["Properties"][0].HasMember("Property"));
    RapidJsonValueCR propertyJson = field["Properties"][0]["Property"];

    EXPECT_STREQ("String", propertyJson["Name"].GetString());
    EXPECT_STREQ("string", propertyJson["Type"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentDescriptorSerializationTests, ECPropertiesFieldWithEnumProperty)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass("TestSchema", "ClassA");

    ContentDescriptorPtr descriptor = ContentDescriptor::Create();
    descriptor->GetAllFields().push_back(new ContentDescriptor::ECPropertiesField(*classA, ContentDescriptor::Property("this", *classA, *classA->GetPropertyP("EnumProperty"))));

    rapidjson::Document doc = descriptor->AsJson();
    ASSERT_TRUE(doc.HasMember("Fields"));
    ASSERT_TRUE(doc["Fields"].IsArray());
    ASSERT_EQ(1, doc["Fields"].GetArray().Size());
    RapidJsonValueCR field = doc["Fields"][0];

    EXPECT_STREQ("ClassA_EnumProperty", field["Name"].GetString());
    EXPECT_STREQ("EnumProperty", field["DisplayLabel"].GetString());
    EXPECT_STREQ("enum", field["Type"].GetString());

    ASSERT_TRUE(field.HasMember("Properties"));
    ASSERT_TRUE(field["Properties"].IsArray());
    ASSERT_EQ(1, field["Properties"].GetArray().Size());
    ASSERT_TRUE(field["Properties"][0].HasMember("Property"));
    RapidJsonValueCR propertyJson = field["Properties"][0]["Property"];

    EXPECT_STREQ("EnumProperty", propertyJson["Name"].GetString());
    EXPECT_STREQ("enum", propertyJson["Type"].GetString());
    EXPECT_TRUE(propertyJson["IsStrict"].GetBool());

    ASSERT_TRUE(propertyJson["Choices"].IsArray());
    ASSERT_EQ(3, propertyJson["Choices"].GetArray().Size());
    RapidJsonValueCR choices = propertyJson["Choices"];
    EXPECT_STREQ("A", choices[0]["Label"].GetString());
    EXPECT_STREQ("B", choices[1]["Label"].GetString());
    EXPECT_STREQ("C", choices[2]["Label"].GetString());
    EXPECT_EQ(1, choices[0]["Value"].GetInt());
    EXPECT_EQ(2, choices[1]["Value"].GetInt());
    EXPECT_EQ(3, choices[2]["Value"].GetInt());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentDescriptorSerializationTests, ECPropertiesFieldWithKOQProperty)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass("TestSchema", "ClassA");

    ContentDescriptorPtr descriptor = ContentDescriptor::Create();
    descriptor->GetAllFields().push_back(new ContentDescriptor::ECPropertiesField(*classA, ContentDescriptor::Property("this", *classA, *classA->GetPropertyP("KOQProperty"))));

    rapidjson::Document doc = descriptor->AsJson();
    ASSERT_TRUE(doc.HasMember("Fields"));
    ASSERT_TRUE(doc["Fields"].IsArray());
    ASSERT_EQ(1, doc["Fields"].GetArray().Size());
    RapidJsonValueCR field = doc["Fields"][0];

    EXPECT_STREQ("ClassA_KOQProperty",field["Name"].GetString());
    EXPECT_STREQ("KOQProperty", field["DisplayLabel"].GetString());
    EXPECT_STREQ("int", field["Type"].GetString());

    ASSERT_TRUE(field.HasMember("Properties"));
    ASSERT_TRUE(field["Properties"].IsArray());
    ASSERT_EQ(1, field["Properties"].GetArray().Size());
    ASSERT_TRUE(field["Properties"][0].HasMember("Property"));
    RapidJsonValueCR propertyJson = field["Properties"][0]["Property"];

    EXPECT_STREQ("KOQProperty", propertyJson["Name"].GetString());
    EXPECT_STREQ("int", propertyJson["Type"].GetString());

    ASSERT_TRUE(propertyJson.HasMember("KindOfQuantity"));
    RapidJsonValueCR koq = propertyJson["KindOfQuantity"];
    EXPECT_STREQ("TestSchema:TestKOQ", koq["Name"].GetString());
    EXPECT_STREQ("Test", koq["DisplayLabel"].GetString());
    EXPECT_STREQ("MM(real)", koq["PersistenceUnit"].GetString());
    EXPECT_STREQ("MM(real)", koq["CurrentUnit"].GetString());

    ASSERT_TRUE(koq["PresentationUnits"].IsArray());
    ASSERT_EQ(2, koq["PresentationUnits"].GetArray().Size());
    RapidJsonValueCR units = koq["PresentationUnits"];
    EXPECT_STREQ("MM(real)", units[0].GetString());
    EXPECT_STREQ("CM(real)", units[1].GetString());
    }