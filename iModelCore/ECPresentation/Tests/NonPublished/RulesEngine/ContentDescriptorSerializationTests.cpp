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
    ContentDescriptor::ECPropertiesField field(*classA, ContentDescriptor::Property("this", *classA, *classA->GetPropertyP("String")));
    rapidjson::Document actual = field.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Category": {
            "Name": "",
            "DisplayLabel": "",
            "Description": "",
            "Expand": false,
            "Priority": 0
        },
        "Name": "ClassA_String",
        "DisplayLabel": "String",
        "Editor": "",
        "Priority": 0,
        "Type": {
            "ValueFormat": "Primitive",
            "TypeName": "string"
            },
        "IsReadOnly": false,
        "Properties": [{
            "Property": {
                "BaseClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:ClassA",
                    "Label": "ClassA"
                },
                "ActualClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:ClassA",
                    "Label": "ClassA"
                },
                "Name": "String",
                "Type": "string"
                },
            "RelatedClassPath": []
            }]
        })");
    expected["Properties"][0]["Property"]["BaseClassInfo"]["Id"].SetString(classA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Properties"][0]["Property"]["ActualClassInfo"]["Id"].SetString(classA->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentDescriptorSerializationTests, ECPropertiesFieldWithEnumProperty)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass("TestSchema", "ClassA");
    ContentDescriptor::ECPropertiesField field(*classA, ContentDescriptor::Property("this", *classA, *classA->GetPropertyP("EnumProperty")));
    rapidjson::Document actual = field.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Category": {
            "Name": "",
            "DisplayLabel": "",
            "Description": "",
            "Expand": false,
            "Priority": 0
        },
        "Name": "ClassA_EnumProperty",
        "DisplayLabel": "EnumProperty",
        "Editor": "",
        "Priority": 0,
        "Type": {
            "ValueFormat": "Primitive",
            "TypeName": "enum"
            },
        "IsReadOnly": false,
        "Properties": [{
            "Property": {
                "BaseClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:ClassA",
                    "Label": "ClassA"
                },
                "ActualClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:ClassA",
                    "Label": "ClassA"
                },
                "Name": "EnumProperty",
                "Type": "enum",
                "IsStrict": true,
                "Choices": [{
                    "Label": "A",
                    "Value": 1
                    },{
                    "Label": "B",
                    "Value": 2
                    },{
                    "Label": "C",
                    "Value": 3
                    }]
                },
            "RelatedClassPath": []
            }]
        })");
    expected["Properties"][0]["Property"]["BaseClassInfo"]["Id"].SetString(classA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Properties"][0]["Property"]["ActualClassInfo"]["Id"].SetString(classA->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentDescriptorSerializationTests, ECPropertiesFieldWithKOQProperty)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass("TestSchema", "ClassA");
    ContentDescriptor::ECPropertiesField field(*classA, ContentDescriptor::Property("this", *classA, *classA->GetPropertyP("KOQProperty")));
    rapidjson::Document actual = field.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"*({
        "Category": {
            "Name": "",
            "DisplayLabel": "",
            "Description": "",
            "Expand": false,
            "Priority": 0
            },
        "Name": "ClassA_KOQProperty",
        "DisplayLabel": "KOQProperty",
        "Editor": "",
        "Priority": 0,
        "Type": {
            "ValueFormat": "Primitive",
            "TypeName": "int"
            },
        "IsReadOnly": false,
        "Properties": [{
            "Property": {
                "BaseClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:ClassA",
                    "Label": "ClassA"
                },
                "ActualClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:ClassA",
                    "Label": "ClassA"
                },
                "Name": "KOQProperty",
                "Type": "int",
                "KindOfQuantity": {
                    "Name": "TestSchema:TestKOQ",
                    "DisplayLabel": "Test",
                    "PersistenceUnit": "MM(real)",
                    "CurrentUnit": "MM(real)",
                    "PresentationUnits": ["MM(real)", "CM(real)"]
                    }
                },
            "RelatedClassPath": []
            }]
        })*");
    expected["Properties"][0]["Property"]["BaseClassInfo"]["Id"].SetString(classA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Properties"][0]["Property"]["ActualClassInfo"]["Id"].SetString(classA->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }