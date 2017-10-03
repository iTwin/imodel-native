/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RulesEngine/ContentSerializationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/Content.h>
#include "../../NonPublished/RulesEngine/ECDbTestProject.h"
#include "../../NonPublished/RulesEngine/TestHelpers.h"

#define TEST_SCHEMA R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="test" version="9.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="True" description="" displayLabel="IntegerEnum">
            <ECEnumerator value="1" displayLabel="A" />
            <ECEnumerator value="2" displayLabel="B" />
            <ECEnumerator value="3" displayLabel="C" />
        </ECEnumeration>
        <KindOfQuantity typeName="TestKOQ" displayLabel="Test" persistenceUnit="MM" relativeError="0" presentationUnits="MM;CM"/>
        <ECEntityClass typeName="ClassA">
            <ECProperty propertyName="String" typeName="string" />
            <ECProperty propertyName="EnumProperty" typeName="TestEnum" />
            <ECProperty propertyName="KOQProperty" typeName="int" kindOfQuantity="TestKOQ" />
        </ECEntityClass>
    </ECSchema>)xml"

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                09/2017
+===============+===============+===============+===============+===============+======*/
struct ContentSerializationTests : ::testing::Test
{
    static ECDbTestProject* s_project;
    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("ContentSerializationTests");

        ECSchemaPtr schema;
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
        EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, TEST_SCHEMA, *schemaContext));

        BentleyStatus importStatus = s_project->GetECDb().Schemas().ImportSchemas({schema.get()});
        BeAssert(SUCCESS == importStatus);

        BeSQLite::DbResult result = s_project->GetECDb().SaveChanges();
        BeAssert(BeSQLite::DbResult::BE_SQLITE_OK == result);
        }
    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }
};
ECDbTestProject* ContentSerializationTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSerializationTests, ECPropertiesFieldWithSimplePrimitiveProperty)
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
TEST_F(ContentSerializationTests, ECPropertiesFieldWithEnumProperty)
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
TEST_F(ContentSerializationTests, ECPropertiesFieldWithKOQProperty)
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

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                10/2017
//=======================================================================================
struct TestEditorParams : ContentFieldEditor::Params
{
private:
    Utf8String m_name;
protected:
    Utf8CP _GetName() const override {return m_name.c_str();}
    Params* _Clone() const override {return new TestEditorParams(m_name);}
    bool _Equals(Params const& other) const override {return Params::_Equals(other) && m_name == static_cast<TestEditorParams const&>(other).m_name;}
    rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetString("params_content", json.GetAllocator());
        return json;
        }
public:
    TestEditorParams(Utf8String name) : m_name(name) {}
};

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSerializationTests, ContentFieldEditors)
    {
    ContentFieldEditor editor("editor_name");
    editor.GetParams().push_back(new TestEditorParams("test_params"));
    rapidjson::Document actual = editor.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"*({
        "Name": "editor_name",
        "Params": {
            "test_params": "params_content"
            }
        })*");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }